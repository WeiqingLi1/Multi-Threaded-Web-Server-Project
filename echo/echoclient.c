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
	if(strcmp(hostname, "localhost") == 0){
		hostname = LOCALHOST;

	}

	char messageBuffer[MESSAGEBUFFER];
	char portnumchar[6];
	int mySocket;
	struct addinfo hints, *serverinfo, *p;
	int returnvalue, recvMsgSize, sendMsgSize;

	sprintf(portnumchar, "%d", portno);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((returnvalue = getaddinfo(hostname, portnumchar, &hints, &serverinfo)) != 0){
		fprintf(stderr, "%s @ %d: [CLIENT] Failure at getaddinfo() (%s)\n", __FILE__, __LINE__, gai_strerror(returnvalue));
		exit(1);
	}

	//---------
	for(p = serverinfo; p != NULL; p = p->ai_next){
		if((mySocket = socket(p->ai_family, p->ai_socketype, p->ai_protocol)) == -1){
			fprintf(stderr, "%s @ %d: [CLIENT] Failure at socket()\n", __FILE__, __LINE__);
			continue;
		}

		if(connect(mySocket, p->ai_addr, p->ai_addrlen) == -1){
			close(mySocket);
			fprintf(stderr, "%s @ %d: [CLIENT] Failure at connect()\n", __FILE__, __LINE__);
			continue;
		}
		break;
	}

	freeaddinfo(serverinfo);

	if(p == NULL){
		fprintf(stderr, "%s @ %d: [CLIENT] Failure at connect() at all\n", __FILE__, __LINE__);
		exit(1);
	}

	sendMsgSize = send(mySocket, message, MESSAGEBUFFER -1, 0);
	if(sendMsgSize != MESSAGEBUFFER -1){
		fprintf(stderr, "%s @ %d: [CLIENT] Failure to send() %d bytes\n", __FILE__, __LINE__, MESSAGEBUFFER -1);
		exit(1);
	}

	if((recvMsgSize = recv(mySocket, messageBuffer, MESSAGEBUFFER -1, 0)) == -1){
		fprintf(stderr, "%s @ %d: [CLIENT] Failure to recv()\n", __FILE__, __LINE__);
		exit(1);
	}

	messageBuffer[recvMsgSize] = '\0';

	fprintf(student, "%s", messageBuffer);

	close(mySocket);
	return 0;
}
