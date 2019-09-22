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
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if(socketfd == -1){
  	perror("Could not create a socket\n");
	exit(1);
  }

  setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  memset((char *)&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(portno);

  if(bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
	perror("Bind failed\n");
	exit(1);
  }
	
  if(listen(socketfd , maxnpending) < 0){
  	perror("Listen failed\n");
	exit(1);
  }

	
  sockaddr_size = sizeof(struct sockaddr_in);
  while((new_socketfd = accept(socketfd, (struct sockaddr *)&client_addr, (socklen_t*)&sockaddr_size))){
	memset(client_reply, '\0', BUFSIZE);
	if(read(new_socketfd, client_reply , BUFSIZE) < 0){
		printf("Message receive from client failed\n");
		exit(1);
	}
	perror(client_reply);
	fflush(stdout);
	strcpy(message, client_reply);
	write(new_socketfd, message, strlen(message)+1);
	if(close(new_socketfd) < 0){
		printf("Could not close socket\n");
		exit(1);
	}
  }

	if(new_socketfd < 0){
		perror("Connection accept failed\n");
		exit(1);
		
	}



  
  



  return 0;

}
