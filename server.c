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

request_t queue[MAX_queue_len];

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

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

  while (1) {

    // Accept client connection
    if (int GetFd = accept_connection() < 0) { //Returns fd
			//exit();	 		
	 		printf("Error Connection Not Accepted");
	 }

    // Get request from the client
    char *dispatchbuffer = (char *)malloc(sizeof(char) * 1024);
    memset(dispatchbuffer, '\0', 1024);
	 if (int get_request(GetFd, dispatchbuffer) != 0) {
	 		printf("Unable to Get Request");
	 }

    // Add the request into the queue
    //queue[] = dispatchbuffer;

   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

   while (1) {

    // Get the request from the queue
    char *workerbuffer = (char *)malloc(sizeof(char) * 1024);
    memset(workerbuffer, '\0', 1024);
    //workerbuffer = queue[];

    // Get the data from the disk or the cache (extra credit B)

    // Log the request into the file and terminal
	

    // return the result
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

  // Change SIGINT action for grace termination

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

  // Create dynamic pool manager thread (extra credit A)

  // Terminate server gracefully
    // Print the number of pending requests in the request queue
    // close log file
    // Remove cache (extra credit B)

  return 0;
}
