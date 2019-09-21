# Project README file
Author: Weiqing Li wli467@uncc.edu


## Project Description
### Warm-up
#### Echo Client - Server
The client initializes and write a content to the socket. After the writing, the client listens for response from server.

The server initializes and listens for a connection from client. When there is a data sent from the client, the server reads content and writes it back to the socket.

#### Transfer File
Basic socket setup was used, and connect code for echoclient.c and echoserver.c, then add parts for file read and write.

transferserver.c: when a connection accepted from client, it open a file and read a file and send it over socket.

We can use a simple way: create a buffer for reading file content, then read file onto buffer using read(), and send buffer using write() to client socket. Performed in loops to read and write file in chunks.

However, we have a better way, which I used sendfile() on server to read file and send to client. sendfile() excapsulates series of operations of the above read() and write().

### Implement Getfile Protocol

### Implement Multithreated Getfile Server


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
