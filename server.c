#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

// structs:
typedef struct request_queue {
   int fd;
   char *request;
} request_t;

// set up the request queue
request_t q[MAX_queue_len];
int insert_idx = 0;
int remove_idx = 0;

struct worker_params {
   int file;
   int id;
};

// set up the lock 
pthread_mutex_t ring_access = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// set up the condition variables
pthread_cond_t some_content = PTHREAD_COND_INITIALIZER;
pthread_cond_t free_slot = PTHREAD_COND_INITIALIZER;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

//Graceful termination signals
static volatile sig_atomic_t doneflag = 0; // accessible by main and signal handler
static void setdoneflag(int signo) {
	doneflag = 1;
}

/* ************************************ Utilities ********************************/

// Function to get the content type from the request
char* getContentType(char * mybuf) {
   // Returns the content type based on the file type in the request
   char *endingbuf = (char *)malloc(sizeof(char) * BUFF_SIZE);
   memset(endingbuf, '\0', BUFF_SIZE);
   int i = 0;
   while(mybuf[i] != '.') {
		i++;   
   }
   int j = i;
	while(mybuf[i] != '\0') {
		endingbuf[i-j] = mybuf[i];
		i++;
	}
	if(strcmp(endingbuf, ".html")) {
		return "text/html";	
	}
	else if(strcmp(endingbuf, ".jpg")) {
		return "image/jpeg";	
	}
	else if(strcmp(endingbuf, ".gif")) {
		return "image/gif";	
	}
	else {
		return "text/plain";	
	}
}

// Function to open and read the file from the disk into the memory
int readFromDisk(char* filename, char **buffer) {
  int size;
  FILE* f;

  f = fopen(filename, "rb"); 
  if(f != NULL){
    if( fseek(f, 0, SEEK_END) ){
      fclose(f);
      return -1;
    }

    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *buffer = (char *)malloc(sizeof(char) * size);
    fread(*buffer, size, size, f);
    fclose(f);
    return size;
  }

  return -1; //error
}

/* ************************************ Important Functions ********************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  while (1) {
    // Accept client connection
    int fd = accept_connection();
    if (fd < 0) { //Illegal request
	      printf("Error Connection Not Accepted");
        exit(1);
    }
    char *filename = (char *)malloc(sizeof(char) * BUFF_SIZE);
    memset(filename, '\0', BUFF_SIZE);
    // Get request from the client
    if (get_request(fd, filename) != 0) {
	    printf("Unable to Get Request");
      exit(1);
    }
    if(pthread_mutex_lock(&ring_access) != 0)
      printf("lock unsuccessful");
	  
	  while(insert_idx == MAX_queue_len - 1){
		  pthread_cond_wait (&some_content, &ring_access);
	  }
    
    //insert request
    q[insert_idx].fd = fd;
    q[insert_idx].request = filename;
    insert_idx ++;
    
    pthread_cond_signal(&free_slot);
    if(pthread_mutex_unlock(&ring_access) != 0)
      printf("unlock unsuccessful");
    
  }

  return NULL;
}

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  int reqCompleted = 0;
  while (1) {

    if(pthread_mutex_lock(&ring_access) != 0)
      printf("lock unsuccessful");
	  while(insert_idx == remove_idx){
		  pthread_cond_wait (&free_slot, &ring_access);
	  }
    
    // Get the request from the queue
    struct worker_params* wp = (struct worker_params*) arg;
    int fd2 = q[remove_idx].fd;
    char* filename = q[remove_idx].request;
    remove_idx ++;
    
    pthread_cond_signal(&some_content);
    if(pthread_mutex_unlock(&ring_access) != 0)
      printf("unlock unsuccessful");
    
    // Get the data from the disk or the cache (extra credit B)
    
    char *buffer = NULL;
    printf("%s \n", filename);
    int numbytes = readFromDisk(filename +1, &buffer);
    if(numbytes == -1) {
      printf("error read");
    }
    
    // return the result
    char *buf = (char *)malloc(sizeof(char) * BUFF_SIZE);
    int error;
    int result;
    if(fd2 < 0) {
      buf = "bad request";
      error = return_error(fd2, buf); //return error for illegal request
      if (error != 0 ){
        printf("failed to return error");
      }
    }
    else {
      char *content_type = getContentType(filename);
      result = return_result(fd2, content_type, buffer, numbytes);
      if(result != 0) {
        printf("error return the result");
      }
    }

    // Log the request into the file and terminal
    if(pthread_mutex_lock(&mtx) != 0)
       printf("lock unsuccessful");
    reqCompleted++;
    char logInfo[BUFF_SIZE];
    char endinfor[BUFF_SIZE];
    memset(logInfo, '\0', BUFF_SIZE);
    memset(endinfor, '\0', BUFF_SIZE);
    sprintf(logInfo, "[%d][%d][%d][%s]", wp->id, reqCompleted, wp->file, filename);
    if (fd2 < 0)
      sprintf(endinfor, "[%s]\n", buf);
    else
      sprintf(endinfor, "[%d]\n", result);
    strcat(logInfo, endinfor);  

    
    int ret = write(wp->file, logInfo, strlen(logInfo));
		if(ret < 0){
			printf("ERROR: Cannot write to file %s\n", filename);
			exit(1);
    }
    printf("%s", logInfo);
    
    if(pthread_mutex_unlock(&mtx) != 0)
      printf("unlock unsuccessful");

 }
  return NULL;
}

/* ************************************ Main ********************************/

int main(int argc, char **argv) {
  printf("starting");
  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args
  int port = strtol(argv[1], NULL, 10);
  int pathSize = strlen(argv[2]);
  char *path = malloc(pathSize + 1);
  memset(path, '\0', pathSize + 1);
  strcpy(path, argv[2]);
  int numDispatchers = strtol(argv[3], NULL, 10);
  int numWorkers = strtol(argv[4], NULL, 10);
  int dFlag = strtol(argv[5], NULL, 10);
  int queLength = strtol(argv[6], NULL, 10);
  int cacheSize = strtol(argv[7], NULL, 10);

  // Perform error checks on the input arguments
	if (port < 1025 || port > 65535) {
		perror("Invalid port number");
		exit(0);	
	}	
	else if (pathSize <= 0) {
		perror("Invalid path");
		exit(0);	
	}
	else if (numDispatchers > 100 || numDispatchers <= 0) {
		perror("Invalid number of dispatchers");
		exit(0);	
	}
	else if (numWorkers > 100 || numWorkers <= 0) {
		perror("Invalid number of workers");
		exit(0);
	}
	else if (dFlag != 0) {
		perror("Dynamic pool size not supported");
		exit(0);	
	}
	else if (queLength > 100 || queLength <= 0) {
		perror("Invalid queue length");
		exit(0);	
	}
	else if (cacheSize != 0) {
		perror("Dynamic caching not supported");
		exit(0);	
	}

  // Change SIGINT action for grace termination
  //Graceful Termination: Deallocate memory, free mutexs
  struct sigaction act;
  	act.sa_handler = setdoneflag; //set up signal handler
	act.sa_flags = 0;
	if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)) {
		perror("Failed to set SIGINT handler");
		return 1;	
	}
  
  // Open log file
  int fd = open("web_server_log.txt", O_CREAT | O_WRONLY, 0777);
  if (fd < 0){
			printf("ERROR: Cannot open the log file \n");
			exit(0);
		}
		
  // Change the current working directory to server root directory
  		if (chdir(path) != 0)  
    		perror("failed to change to web root directory");

  		// Start the server
  		init(port);

      pthread_t w_threads[numWorkers];
  		pthread_t d_threads[numDispatchers];

  		pthread_attr_t attr;
  		pthread_attr_init(&attr);
  		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

  		for(int i = 0; i < numDispatchers; i++){
    		if(pthread_create(&(d_threads[i]), &attr, dispatch, NULL) != 0) {
            printf("Dispatcher thread failed to create\n");
    		}
  		}

  		for(int i = 0; i < numWorkers; i++){
    		if(pthread_create(&(w_threads[i]), &attr, worker, (void*)&fd) != 0) {
            printf("Worker thread failed to create\n");
    		}
  		}

  //Wait for Ctrl-C
  while (!doneflag) {
  		sleep(1);
	}
	
   // Terminate server gracefully
   // Print the number of pending requests in the request queue
   printf("Program terminating ...\n");    	
   printf("Pending requests: %d", insert_idx); 
   // close log file
   close(fd);
   
   return 0;
}
