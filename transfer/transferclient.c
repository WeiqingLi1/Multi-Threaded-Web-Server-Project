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

#define USAGE                                                \
    "usage:\n"                                               \
    "  transferclient [options]\n"                           \
    "options:\n"                                             \
    "  -s                  Server (Default: localhost)\n"    \
    "  -p                  Port (Default: 19121)\n"           \
    "  -o                  Output file (Default cs6200.txt)\n" \
    "  -h                  Show this help message\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"server", required_argument, NULL, 's'},
    {"port", required_argument, NULL, 'p'},
    {"output", required_argument, NULL, 'o'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0}};

/* Main ========================================================= */
int main(int argc, char **argv)
{
    int option_char = 0;
    char *hostname = "localhost";
    unsigned short portno = 19121;
    char *filename = "cs6200.txt";

    setbuf(stdout, NULL);

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "s:p:o:hx", gLongOptions, NULL)) != -1)
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
        case 'o': // filename
            filename = optarg;
            break;
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        }
    }

    if (NULL == hostname)
    {
        fprintf(stderr, "%s @ %d: invalid host name\n", __FILE__, __LINE__);
        exit(1);
    }

    if (NULL == filename)
    {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    if ((portno < 1025) || (portno > 65535))
    {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }

    /* Socket Code Here */

	if(strcmp(hostname, "localhost") == 0){
		hostname = LOCALHOST;
	}

	char messageBuffer[BUFSIZE];
	char portnumchar[6];
	int mySocket;
	struct addrinfo hints, *serverinfo, *p;
	int returnvalue,recvMsgSize;

	sprintf(portnumchar, "%d", portno);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((returnvalue = getaddrinfo(hostname, portnumchar, &hints, &serverinfo)) != 0){
		fprintf(stderr, "%s @ %d: [CLIENT] Failure at getaddrinfo() (%s)\n", __FILE__, __LINE__, gai_strerror(returnvalue));
		exit(1);
	}

	FILE * clientfile;
	clientfile = fopen(filename, "a+");
	if(clientfile == NULL){
		fprintf(stderr, "%s @ %d: [CLIENT] Failed to fopen() %s\n", __FILE__, __LINE__, filename);
		exit(1);
	}

	for(p = serverinfo; p != NULL; p = p->ai_next){
		if((mySocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
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

	freeaddrinfo(serverinfo);

	if(p == NULL){
		fprintf(stderr, "%s @ %d: [CLIENT] Failed to connect() at all\n", __FILE__, __LINE__);
		exit(1);
	}

	if((recvMsgSize = recv(mySocket, messageBuffer, BUFSIZE-1, 0)) == -1){
		fprintf(stderr, "%s @ %d: [CLIENT] Failure at recv()\n", __FILE__, __LINE__);
		exit(1);
	}

	while(recvMsgSize > 0){
		if(fwrite(messageBuffer, 1, recvMsgSize, clientfile) != recvMsgSize){
			fprintf(stderr, "%s @ %d: [CLIENT] Failed to fwrite() %d bytes to %s\n", __FILE__, __LINE__, recvMsgSize,filename);
			exit(1);
		}

		if((recvMsgSize = recv(mySocket, messageBuffer, BUFSIZE-1, 0)) == -1){
			fprintf(stderr, "%s @ %d: [CLIENT] Failure at recv()\n", __FILE__, __LINE__);
			exit(1);
		}
	}

	fclose(clientfile);
	close(mySocket);

	return 0;
}
