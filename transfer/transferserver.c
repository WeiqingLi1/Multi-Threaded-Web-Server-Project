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
    "  transferserver [options]\n"                           \
    "options:\n"                                             \
    "  -f                  Filename (Default: 6200.txt)\n" \
    "  -h                  Show this help message\n"         \
    "  -p                  Port (Default: 19121)\n"

/* OPTIONS DESCRIPTOR ====================================================== */
static struct option gLongOptions[] = {
    {"filename", required_argument, NULL, 'f'},
    {"help", no_argument, NULL, 'h'},
    {"port", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}};

int main(int argc, char **argv)
{
    int option_char;
    int portno = 19121;             /* port to listen on */
    char *filename = "6200.txt"; /* file to transfer */

    setbuf(stdout, NULL); // disable buffering

    // Parse and set command line arguments
    while ((option_char = getopt_long(argc, argv, "p:hf:x", gLongOptions, NULL)) != -1)
    {
        switch (option_char)
        {
        case 'p': // listen-port
            portno = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", USAGE);
            exit(1);
        case 'h': // help
            fprintf(stdout, "%s", USAGE);
            exit(0);
            break;
        case 'f': // file to transfer
            filename = optarg;
            break;
        }
    }


    if ((portno < 1025) || (portno > 65535))
    {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, portno);
        exit(1);
    }
    
    if (NULL == filename)
    {
        fprintf(stderr, "%s @ %d: invalid filename\n", __FILE__, __LINE__);
        exit(1);
    }

    /* Socket Code Here */

	char *hostname = LOCALHOST;
	char messageBuffer[BUFSIZE];
	char portnumchar[6];
	int serverSocket, clientSocket;
	struct addrinfo hints, *serverinfo, *p;
	struct sockaddr their_addr;
	socklen_t sin_size;
	int yes=1;
	int returnvalue,sendMsgSize;
	size_t numbytesread,numbytestrans,numbytesremaining = 0;

	sprintf(portnumchar, "%d", portno);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((returnvalue = getaddrinfo(hostname, portnumchar, &hints, &serverinfo)) != 0){
		fprintf(stderr, "%s @ %d: [SERVER] Failure at getaddrinfo() (%s)\n", __FILE__, __LINE__, gai_strerror(returnvalue));
		exit(1);
	}

	for(p = serverinfo; p != NULL; p = p->ai_next){
		if((serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at socket()\n", __FILE__, __LINE__);
			continue;
		}

		if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at setsockopt()\n", __FILE__, __LINE__);
			exit(1);
		}

		if(bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1){
			close(serverSocket);
			fprintf(stderr, "%s @ %d: [SERVER] Failure at bind()\n", __FILE__, __LINE__);
			continue;
		}

		break;
	}

	freeaddrinfo(serverinfo);

	if(p == NULL){
		fprintf(stderr, "%s @ %d: [SERVER] Failed to bind() to any port\n", __FILE__, __LINE__);
		exit(1);
	}

	if(listen(serverSocket, BACKLOG) == -1){
		fprintf(stderr, "%s @ %d: [SERVER] Failure at listen() with backlog (%d)\n", __FILE__, __LINE__, BACKLOG);
		exit(1);
	}

	while(1){
		sin_size = sizeof their_addr;
		clientSocket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
		
		if(clientSocket == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at accept()\n", __FILE__, __LINE__);
			continue;
		}

		FILE * serverfile;
		serverfile = fopen(filename, "r+");
		
		if(serverfile == NULL){
			fprintf(stderr, "%s @ %d: [SERVER] Failed to fopen() %s\n", __FILE__, __LINE__, filename);
			exit(1);
		}

		while(!(feof(serverfile))){
			numbytestrans = 0;

			if((numbytesread = fread(messageBuffer, 1, BUFSIZE-1, serverfile)) == -1){
				fprintf(stderr, "%s @ %d: [SERVER] Failed to fread() %d bytes\n", __FILE__, __LINE__, BUFSIZE-1);
				exit(1);
			}

			numbytesremaining = numbytesread;

			while(numbytestrans < numbytesread){
				if((sendMsgSize = send(clientSocket, messageBuffer + numbytestrans, numbytesremaining, 0)) == -1){
					fprintf(stderr, "%s @ %d: [SERVER] Failure to send()\n", __FILE__, __LINE__);
					exit(1);
				}

				numbytestrans += sendMsgSize;
				numbytesremaining -= sendMsgSize;
			}
			memset(&messageBuffer, 0, sizeof messageBuffer);
		}

		fclose(serverfile);
		close(clientSocket);
	}

	close(serverSocket);

	return 0;
}
