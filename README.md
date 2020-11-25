The purpose of this program is to be a web server which uses multiple POSIX threads. The webservar handles the following file types: HTML, JPEG, GIF, and TXT. A client connects to the server and requests a file, the web server then returns the file to the client.

The code can be compiled using the Makefile in the Template folder. The program is then run on two seperate terminals. Terminal 1 runs the web server code, and terminal 2 acts as a client connecting to the server.

---------------------------------------------------------------------------------------------
There is the main function, two important functions (dispatch and worker), and two utility functions(getContentType and readFromDisk). 

The dispatch function attempts to accept a client connection using the accept_connection function. If successfull the get_request function is called to recieve the clien'ts file request. The client's file request is then inserted into a request queue.
The worker function takes a request from the request queue and calls the readFromDisk function to grab the contents of the file from the disk. If successful the request is logged into a log file and the file type is found using the getContentType funciton. The requested file is then returned to the client using the return_result function.

getContentType takes in a character buffer that is the file name. It then finds the "." of the file name and what characters come after it (.html, .jpg, etc.). Using these characters the function returns the file content type in the form of a string ("text/html", "image/jpeg", etc.).
readFromDisk takes in the file name that is requested and opens the file. The file is read into a buffer which will eventually be returned to the client. readFromDisk then returns the size of the file.

Finally there is the main function. The main function takes in arguments for which port is used, the number of dispatchers and workers, and the sizes of the queue and file paths. After performing error checks the function sets up what is needed for graceful termination. A log file is then opened which worker writes to. the server is then started and the POSIX threads are created for the various dispatchers and workers. The server runs until Ctrl-C is entered in which case it shuts down gracefully and returns the amount of requests left in the request queue.
---------------------------------------------------------------------------------------------

Project Group: 25

Members:
Collin Campbell, camp0609
Clara Huang, huan2089
Hunter Bahl, bahlx038

Credits:
Dispatch: Collin
Worker: Clara
Graceful Termination: Hunter
Utilities: All contributed
Debugging: All Contributed
