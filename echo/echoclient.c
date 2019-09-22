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

/* A buffer large enough to contain the longest allowed string */
#define BUFSIZE 1219

#define USAGE                                                                       \
    "usage:\n"                                                                      \
    "  echoclient [options]\n"                                                      \
    "options:\n"                                                                    \
    "  -s                  Server (Default: localhost)\n"                           \
    "  -p                  Port (Default: 19121)\n"                                  \
    "  -m                  Message to send to server (Default: \"Hello world.\")\n" \
    "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"message", required_argument, NULL, 'm'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 19121;
    char *message = "Hello World!!";

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:m:hx", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 's': // server
            hostname = optarg;
            break;
        case 'p': // listen-port
            portno = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        case 'm': // message
            message = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    setbuf(stdout, NULL); // disable buffering

    if ((portno < 1025) || (portno > 65535))
    {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    if (NULL == message)
    {
        fprintf(stderr, "%s @ %d: invalid message\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == hostname)
    {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd == -1){
		printf("Could not create a socket\n");
		exit(1);

	}

	server_info = gethostbyname(hostname);

	if(server_info == NULL){
		fprintf(stderr,"Cannot find host with name %s\n", hostname);
		exit(1);
	}
	
	memcpy(&server_addr.sin_addr.s_addr, server_info->h_addr, server_info->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portno);
	
	//---------

	if(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Error during connect\n");
		exit(1);
	}


	if(write(socketfd, message, strlen(message)) < 0){
		printf("Message send failed\n");
		exit(1);
	}
	
	memset(server_reply, '\0', BUFSIZE);
	if(recv(socketfd, server_reply , 2000 , 0) < 0){
		printf("Reply receive failed\n");
		exit(1);
	}

	printf(server_reply);

	return 0;
}
