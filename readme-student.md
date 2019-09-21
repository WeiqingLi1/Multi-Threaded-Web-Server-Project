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

`assign_worker_to_task`: each worker thread will execute this function. It run indefinitely inside for-loop until `request_queue` is empty and `main()` thread has finished queuing up the requests. Worker threads wait until there is an item in `request_queue`. When there is an item, which is broad casted by `main()`, it performs making request to server. Once all items in queue are fulfilled and `main()` updated `queuing_request_finished` variableto indicate it has finished adding request to queue, worker threads exit and join the main thread.

#### Challenge
#### `gfserver_main.c`
At beginning, `gfserver_main.c` handles request in a single thread whenever registered handler function was called. Now we need to add wrapper request handler which will queue request and notify workers to fulfill the queued requests.
move codes for making request to server into a seperate function so that requests can be made inside a thread.

Your README file is your opportunity to demonstrate to us that you understand the project.  Ideally, this
should be a guide that someone not familiar with the project could pick up and read and understand
what you did, and why you did it.

Specifically, we will evaluate your submission based upon:

- Your project design.  Pictures are particularly helpful here.
- Your explanation of the trade-offs that you considered, the choices you made, and _why_ you made those choices.
- A description of the flow of control within your submission. Pictures are helpful here.
- How you implemented your code. This should be a high level description, not a rehash of your code.
- How you _tested_ your code.  Remember, Bonnie isn't a test suite.  Tell us about the tests you developed.
  Explain any tests you _used_ but did not develop.
- References: this should be every bit of code that you used but did not write.  If you copy code from
  one part of the project to another, we expect you to document it. If you _read_ anything that helped you
  in your work, tell us what it was.  If you referred to someone else's code, you should tell us here.
  Thus, this would include articles on Stack Overflow, any repositories you referenced on github.com, any
  books you used, any manual pages you consulted.


In addition, you have an opportunity to earn extra credit.  To do so, we want to see something that
adds value to the project.  Observing that there is a problem or issue _is not enough_.  We want
something that is easily actioned.  Examples of this include:

- Suggestions for additional tests, along with an explanation of _how_ you envision it being tested
- Suggested revisions to the instructions, code, comments, etc.  It's not enough to say "I found
  this confusing" - we want you to tell us what you would have said _instead_.

While we do award extra credit, we do so sparingly.
