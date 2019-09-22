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


	if(bind(socketfd,(struct sockaddr *)&server_addr , sizeof(server_addr)) < 0){
		perror("Bind failed\n");
		exit(1);
	}
	
	if(listen(socketfd , 8) < 0){
		perror("Listen failed\n");
		exit(1);
	}

	sockaddr_size = sizeof(struct sockaddr_in);
	while((new_socketfd = accept(socketfd, (struct sockaddr *)&client_addr, (socklen_t*)&sockaddr_size))){
		filefd = open(filename, O_RDONLY, S_IRUSR);
		if(filefd < 0){
			perror("Could not open the file");
			exit(1);
		}

		fstat(filefd, &filestat);
		sent_size = sendfile(new_socketfd, filefd, 0, filestat.st_size);
		if(sent_size == -1){
			perror("Error in sendfile");
			exit(1);
		}

		if(sent_size != filestat.st_size){
			perror("Incomplete transfer from sendfile");
			exit(1);
		}

		if(close(filefd) < 0){
			perror("Could not close file");
			exit(1);
		}
		
		if(close(new_socketfd) < 0){
			perror("Could not close socket\n");
			exit(1);
		}
	}
	if(new_socketfd < 0){
		perror("Connection accept failed\n");
		exit(1);
	}



	return 0;
}
