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

typedef struct requestBuffer {

	request_t* q;
	pthread_cond_t* cond;
	pthread_mutex_t* mutex;
  int insert_idx;
  int remove_idx;
  int buffer_length;
  int max_size;
} req_buffer_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

//create a thread-queue
request_t queue[MAX_queue_len];
int insert_idx = 0;
int remove_idx = 0;

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

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
    // Open and read the contents of file given the request
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {

	struct req_buffer_t* rq = (struct req_buffer_t*) arg;

  while (1) {

    // Accept client connection
    if (int fd = accept_connection() < 0) { //Returns fd
	      printf("Error Connection Not Accepted");
        exit();
    }
    char *filename = (char *)malloc(sizeof(char) * BUFF_SIZE);
    memset(filename, '\0', BUFF_SIZE);
    // Get request from the client
    if (get_request(fd, filename) != 0) {
	    printf("Unable to Get Request");
      exit();
    }

	  pthread_mutex_lock(rq->mutex);
	  while(rq->buffer_length == rq->max_size){
		  pthread_cond_wait (rq->cond,rq->mutex);
	  }
    rq->q[rq->insert_idx].fd = fd;
    rq->q[rq->insert_idx].request = filename;
    rq->insert_idx++;
    if(rq->insert_idx == rq->max_size){
      req->insert_idx = 0;
    }
    rq->bufferLength++;
    pthread_cond_signal(rq->cond);
    pthread_mutex_unlock(rq->mutex);
    // else {
    //   if(pthread_mutex_lock(&mtx) != 0)
    //     printf("lock unsuccessful");
    //   // Add the request into the queue
    //   queue[insert_idx].fd = fd;
    //   queue[insert_idx].request = filename;
    //   insert_idx ++;
    //   if(pthread_mutex_unlock(&mtx) != 0)
    //     printf("unlock unsuccessful");
    // }
  }
  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

  struct req_buffer_t* rq = (struct req_buffer_t*) arg;

  while (1) {
    if(pthread_mutex_lock(rq->mutex) != 0)
      printf("lock unsuccessful");
	  while(rq->buffer_length == 0){
		  pthread_cond_wait (rq->cond,rq->mutex);
	  }
    int fd = rq->q[rq->remove_idx].fd;
    char* filename = rq->q[rq->remove_idx].request;
    rq->buffer_length--;
    rq->remove_idx++;
    if(rq->remove_idx == rq->max_size){
      req->remove_idx = 0;
    }
    pthread_cond_signal(rq->cond);
    if(pthread_mutex_lock(rq->mutex) != 0)
      printf("lock unsuccessful");
    // Get the request from the queue
    int fd = queue[remove_idx].fd;
    char * filename = queue[remove_idx].request;
    remove_idx ++;
int ret = write(fd, filename, strlen(filename));
		if(ret < 0){
			printf("ERROR: Cannot write to file %s\n", wordFileName);
			exit(0);
		}
    printf("%s", filename);
    if(pthread_mutex_unlock(&mtx) != 0)
      printf("unlock unsuccessful");
    // Get the data from the disk or the cache (extra credit B)
    // if (fd < 0) {
    //   char * buf = "bad request";
    //   int error = return_error(fd, buf);
    //   if (error != 0 ){
    //     printf("failed to return error")
    //   }
    //   else
    //     exit();
    //  }
    //  else {
    // // Log the request into the file and terminal
    //   FILE *f;
    //   if(f = fopen("web_server_log.txt", "w") == EOF){
    //     printf("error open the file")
    //   };
    //   if(pthread_mutex_lock(&mtx) != 0)
    //     printf("lock unsuccessful");

    //   fprintf(f, "%s", filename);

    //   if(pthread_mutex_unlock(&mtx) != 0)
    //     printf("unlock unsuccessful");

    //   printf("%s", filename);
    // // return the result
    //   char *content_type = getContentType(filename);
    //   if(return_result(fd, content_type, ))
    // }

  }
  return NULL;
}

/**********************************************************************************/

int main(int argc, char **argv) {

  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args
  int *port = argv[1];
  int *pathSize = strlen(argv[2])
  char *path = malloc(pathSize + 1);
  memset(path, '\0', pathSize + 1);
  strcpy(path, argv[2]);
  int *numDispatchers = argv[3];
  int *numWorkers = argv[4];
  int *dFlag = argv[6];
  int *queLength = argv[7];
  int *cacheSize = argv[8];

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
	else if (numworkers > 100 || numWorkers <= 0) {
		perror("Invalid number of workers");
		exit(0);
	}
	else if (dflag != 0) {
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
	
  while (!doneflag) {
  		// Open log file
  	int fd = open("web_server_log.txt", O_WRONLY);
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

  		// Create dispatcher and worker threads (all threads should be detachable)
  		pthread_t w_threads[numWorkers];
  		pthread_t d_threads[numDispatchers];

  		pthread_attr_t attr;
  		pthread_attr_init(&attr);
  		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  		//request_t requestQueue[queLength];

	  	//struct req_buffer_t* rq = (struct req_buffer_t*) malloc(sizeof(struct req_buffer_t));
      		struct req_buffer_t* rq = (struct req_buffer_t*) malloc(sizeof(queLength));
		rq->q = (struct buffer*) malloc(sizeof(struct buffer));
		rq->insert_idx = 0;
      		rq->remove_idx = 0;
      		rq->max_size = queLength;
      		rq->buffer_length = 0;
		rq->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
		rq->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		pthread_cond_init(rq->cond, NULL);
		pthread_mutex_init(rq->mutex, NULL);

  		for(int i = 0; i < numDispatchers; i++){
    		  if(pthread_create(&(d_threads[i]), &attr, dispatch, (void*) &rq) != 0) {
            	    printf("Dispatcher thread failed to create\n");
    		  }
  		}

  		for(int i = 0; i < numWorkers; i++){
    		  if(pthread_create(&(w_threads[i]), &attr, worker, (void*) &requestQueue) != 0) {
                    printf("Worker thread failed to create\n");
    		  }
  		}

  		// Create dynamic pool manager thread (extra credit A)
	}
	
   // Terminate server gracefully
   // Print the number of pending requests in the request queue
   printf("Program terminating ...\n");    	
   printf("Pending requests: %d", insert.idx); 
   // close log file
   int close(fd);  
   // Remove cache (extra credit B)
   
   return 0;
}
