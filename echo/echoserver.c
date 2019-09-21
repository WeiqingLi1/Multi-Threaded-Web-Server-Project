#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>


#define BUFSIZE 1219

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoserver [options]\n"                                                    \
"options:\n"                                                                  \
"  -p                  Port (Default: 19121)\n"                                \
"  -m                  Maximum pending connections (default: 1)\n"            \
"  -h                  Show this help message\n"                              \

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
  {"port",          required_argument,      NULL,           'p'},
  {"maxnpending",   required_argument,      NULL,           'm'},
  {"help",          no_argument,            NULL,           'h'},
  {NULL,            0,                      NULL,             0}
};


int main(int argc, char **argv) {
  int option_char;
  int portno = 19121; /* port to listen on */
  int maxnpending = 1;
  
  // Parse and set command line arguments
  while ((option_char = getopt_long(argc, argv, "p:m:hx", gLongOptions, NULL)) != -1) {
   switch (option_char) {
      case 'p': // listen-port
        portno = atoi(optarg);
        break;                                        
      default:
        fprintf(stderr, "%s ", USAGE);
        exit(1);
      case 'm': // server
        maxnpending = atoi(optarg);
        break; 
      case 'h': // help
        fprintf(stdout, "%s ", USAGE);
        exit(0);
        break;
    }
  }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }
    if (maxnpending < 1) {
        fprintf(stderr, "%s @ %d: invalid pending count (%d)\n", __FILE__, __LINE__, maxnpending);
        exit(1);
    }


  /* Socket Code Here */
  char *hostname = LOCALHOST;
  char messageBuffer[MESSAGEBUFFER];
  char portnumchar[6];
  int serverSocket, clientSocket;
  struct addinfo hints, *serverinfo, *p;
  struct sockaddr their_addr;
  socklen_t sin_size;
  int yes = 1;
  int returnvalue, recvMsgSize;

  sprintf(portnumchar, "%d", portno);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if((returnvalue = getaddinfo(hostname, portnumchar, &hints, &serverinfo)) != 0){
	fprintf(stderr, "%s @ %d: [SERVER] Failure at getaddinfo() (%s)\n", __FILE__, __LINE__, gai_strerror(returnvalue));
	exit(1);
  }

  for(p = serverinfo; p != NULL; p = p->ai_next){
	if((serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
		fprintf(stderr, "%s @ %d: [SERVER] Failure at socket()\n", __FILE__, __LINE__);
		continue;
	}

	if(setsocket(serverSocket, SOL_SOCKET, SO_RESUEADDR, &yes, sizeof(int)) == -1){
		fprintf(stderr, "%s @ %d: [SERVER] Failure at setsockopt()\n", __FILE__, __LINE__);
		exit(1);
	}

	if(bind(serverSocket, p->ai_addr, p->addrlen) == -1){
		close(serverSocket);
		fprintf(stderr, "%s @ %d: [SERVER] Failure at bind()\n", __FILE__, __LINE__);
		continue;
	}

	break;
  }

  freeaddinfo(serverinfo);

  if(p == NULL){
	fprintf(stderr, "%s @ %d: [SERVER] Failure at bind() with port\n", __FILE__, __LINE__);
	exit(1);
  }

  if(listen(serverSocket, BACKLOG) == -1){
	fprintf(stderr, "%s @ %d": [SERVER] Failure at listen() with backlog(%d)\n", __FILE__, __LINE__, BACKLOG);
	exit(1);
  }

  while(1){
	sin_size = sizeof their_addr;
	clientSocket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
	if(clientSocket == -1){
		fprintf(stderr, "%s @ %d": [SERVER] Failure at accept()\n", __FILE__, __LINE__);
		continue;
	}

	if((recvMsgSize = recv(clientSocket, messageBuffer, MESSAGEBUFFER - 1, 0)) == -1){
		fprintf(stderr, "%s @ %d": [SERVER] Failure at recv()\n", __FILE__, __LINE__);
		exit(1);
	}

	messageBuffer[recvMsgSize] = '\0';

	sendMsgSize = send(clientSocket, messageBuffer, MESSAGEBUFFER-1, 0);

	if(sendMsgSize != MESSAGEBUFFER-1){
		fprintf(stderr, "%s @ %d: [SERVER] Failure to send() %d bytes\n", __FILE__, __LINE__,MESSAGEBUFFER-1);
		exit(1);
	}

	close(clientSocket);
  }

  close(serverSocket);

  return 0;

}
