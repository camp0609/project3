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

/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   char *request;
} request_t;

// set up the request queue
request_t q[MAX_queue_len];
int insert_idx = 0;
int remove_idx = 0;

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

//pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

/*
// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
}
// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
}
// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
}
// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
}
*/

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
   // Should return the content type based on the file type in the request
   // (See Section 5 in Project description for more details)
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
// Add necessary arguments as needed
int readFromDisk(char* filename, char * buffer) {
  int size;
  FILE* f;

  f = fopen(filename, "rb"); 
  if(f != NULL){
    if( fseek(f, 0, SEEK_END) ){
      fclose(f);
      return -1;
    }

    size = ftell(f);
    fclose(f);
    return size;
  }

  return -1; //error
}

/**********************************************************************************/

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
    
    //insert things
    q[insert_idx].fd = fd;
    q[insert_idx].request = filename;
    insert_idx ++;
    
    pthread_cond_signal(&free_slot);
    if(pthread_mutex_unlock(&ring_access) != 0)
      printf("unlock unsuccessful");
    
  }

  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void * f) {

  while (1) {

    if(pthread_mutex_lock(&ring_access) != 0)
      printf("lock unsuccessful");
	  while(insert_idx == remove_idx){
		  pthread_cond_wait (&free_slot, &ring_access);
	  }
    
    // Get the request from the queue
    int *fd1 = (int *) f;
    int fd = *fd1;
    int fd2 = q[remove_idx].fd;
    char* filename = q[remove_idx].request;
    remove_idx ++;
    
    pthread_cond_signal(&some_content);
    if(pthread_mutex_unlock(&ring_access) != 0)
      printf("unlock unsuccessful");
    
    // Get the data from the disk or the cache (extra credit B)
    
    char * buffer = (char *)malloc(sizeof(char) * BUFF_SIZE);
    int numbytes = readFromDisk(filename, buffer);
    if(numbytes == -1) {
      printf("error read");
    }

    // Log the request into the file and terminal
    if(pthread_mutex_lock(&mtx) != 0)
       printf("lock unsuccessful");
       
    int ret = write(fd, filename, strlen(filename));
		if(ret < 0){
			printf("ERROR: Cannot write to file %s\n", filename);
			exit(1);
    }
    printf("%s", filename);
    
    if(pthread_mutex_unlock(&mtx) != 0)
      printf("unlock unsuccessful");

    // return the result
    if(fd2 < 0) {
      char *buf = "bad request";
      int error = return_error(fd, buf); //return error for illegal request
      if (error != 0 ){
        printf("failed to return error");
      }
    }
    else {
      char *content_type = getContentType(filename);
      if(return_result(fd2, content_type, buffer, numbytes) != 0) {
        printf("error return the result");
      }
    }

 }
  return NULL;
}
/**********************************************************************************/

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

  //char *path[100] = argv[2];

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
  		
  		// Initialize cache (extra credit B)

  		// Start the server
  		init(port);

      pthread_t w_threads[numWorkers];
  		pthread_t d_threads[numDispatchers];

  		pthread_attr_t attr;
  		pthread_attr_init(&attr);
  		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  		//request_t requestQueue[queLength];


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

  		
  while (!doneflag) {

  		// Create dispatcher and worker threads (all threads should be detachable)
  		sleep(1);

  		// Create dynamic pool manager thread (extra credit A)
	}
	
   // Terminate server gracefully
   // Print the number of pending requests in the request queue
   printf("Program terminating ...\n");    	
   printf("Pending requests: %d", insert_idx); 
   // close log file
   close(fd);  
   // Remove cache (extra credit B)
   
   return 0;
}
