# Project README file
Author: Weiqing Li wli467@gatech.edu


## Project Description
### Warm-up
#### Echo Client - Server
The client initializes and write a content to the socket. After the writing, the client listens for response from server.

The server initializes and listens for a connection from client. When there is a data sent from the client, the server reads content and writes it back to the socket.

#### Transfer File
Basic socket setup was used, and connect code for `echoclient.c` and `echoserver.c`, then add parts for file read and write.

`transferserver.c`: when a connection accepted from client, it open a file and read a file and send it over socket.

We can use a simple way: create a buffer for reading file content, then read file onto buffer using `read()`, and send buffer using `write()` to client socket. Performed in loops to read and write file in chunks.

However, we have a better way, which I used `sendfile()` on server to read file and send to client. `sendfile()` excapsulates series of operations of the above `read()` and `write()`.

`transferclient.c`: when a connection is requested, a file is opened and data received from server is written to the file until no more data read from socket.

Challenge: make sure appropriate access permission was given. Grant only write access to file descriptor in `transferclient.c` is not enough, read access is also needed, the the code works.


### Implement Getfile Protocol
#### `gfserver.c`
Divide big function into small functions. e.g. break function `gfserver_server` into pieces `start_server`, `parse_request`, `send_unsuccessful_response`.
Challenge: error case handling, parse request or response string and format request or reponse string.
#### `gfclient.c`
Problem: issue with read from socket to receive server response stall at very last chunk. Change buffer size not working. Need a better method.
### Implement Multithreated Getfile Server
#### `gfclient_download.c`
At beginning, `gfclient_download.c` made request in single thread insode `main()`. Now we need to move codes for making request to server into a seperate function so that requests can be made inside a thread.

New design: now `main()` only deal with parsing arguments, initializing worker thread pool, queuing up requests into `request_queue`.

Sequence of tasks `main()`: 
    parse arguments
--> initialize variables (e.g. request queue, mutex, condition variables etc.)
--> initialize worker threads with function `assign_worker_to_task`
--> queuing requests into request queue `request_queue`
--> set variable `queuing_request_finished` used to indicate `main()` thread has finished queuing requests
--> wait for worker threads to finish all request tasks and join
--> free memory

`assign_worker_to_task`: each worker thread will execute this function. It run indefinitely inside for-loop until `request_queue` is empty and `main()` thread has finished queuing up the requests. Worker threads wait until there is an item in `request_queue`. When there is an item, which is broadcasted by `main()`, it performs making request to server. Once all items in queue are fulfilled and `main()` updated `queuing_request_finished` variableto indicate it has finished adding request to queue, worker threads exit and join the main thread.

#### Challenge
Problem of memory leaking and how to notify worker thread when should exit.

Problem 1: Account for cases where queuing up in main thread not as quick as fulfilling the requests in queue by worker threads. Having empty request queue not necessarily indicate worker threads to exit  since main thread may add more requests in queue. 

Solution: `queuing_request_finished` added, worker thread can only exit when this variable is set and queue is empty.

Problem 2: More test case should be tested, like change number of threads and requests in each thread. e.g. 
#threads is large, #request is large;
#threads is small, #request is small;
#thread > #request per thread.

#### `gfserver_main.c`
At beginning, `gfserver_main.c` handles request in a single thread whenever registered handler function was called. Now we need to add wrapper request handler which will queue request and notify workers to fulfill the queued requests.

Sequence of tasks `main()`: 
    parse arguments
--> initialize variables (e.g. request queue, mutex, condition variables etc.)
--> register wrapper handler, `client_request_handler`, instead of `handler_get`
--> initialize worker threads
--> run `gfserver_server` to listen to client requests

Function `client_request_handler` registered to `gfs` will be called every time client makes a request. All `client_request_handler` queueed in the request path and broadcast worker threads.

`assign_worker_to_task`: each worker thread will execute this function. It run indefinitely inside for-loop. Worker threads wait until there is an item in `client_request_queue`. When there is an item, which is broadcasted by `client_request_handler`, it call `handler_get` with request path dequeued.

#### Challenge
Test code by changing number of threads and number of requests client made.


## Suggestion
Understand the response from bonnie and debug with that is too hard.

## References
http://borkware.com/hacks/CRefresher.pdf
https://stackoverflow.com/
https://beej.us/guide/bgnet/
