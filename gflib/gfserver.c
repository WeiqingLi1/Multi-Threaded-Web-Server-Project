#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <getopt.h>

#include "gfserver.h"
#include "gfserver-student.h"

#define BUFSIZE 2000
#define SCHEME "GETFILE"
/* 
 * Modify this file to implement the interface specified in
 * gfserver.h.
 */

typedef struct gfserver_t{
    struct sockaddr_in server_addr;
    int max_npending;
    ssize_t (*handler)(gfcontext_t *, char *, void*);
    void* arg;
    char* req_path;
} gfserver_t;


typedef struct gfcontext_t{
    int client_socketfd;
} gfcontext_t;


void gfs_abort(gfcontext_t *ctx){
    if (close(ctx->client_socketfd) < 0){
        perror("Could not close socket\n");
        exit(1);
    }

    free(ctx);
}

void gfs_abort(gfcontext_t **ctx){
	close(ctx->socket);
	free(ctx);
}

gfserver_t* gfserver_create(){

	gfserver_t* gfs = (gfserver_t*)malloc(sizeof(gfserver_t));
	gfs->server_addr.sin_family = AF_INET;
	gfs->server_addr.sin_addr.s_addr = INADDR_ANY;
	
    return (gfserver_t *)NULL;
}

ssize_t gfs_send(gfcontext_t **ctx, const void *data, size_t len){

	ssize_t write_size, total_write_transferred = 0;
	do {
		if((write_size = write(ctx->client_socketfd, data, len)) < 0){
			perror("Send failed");
			return -1;
		}
		data += write_size;
		len -= write_size;
		total_write_transferred += write_size;
	} while(write_size > 0)
		
	return total_write_transferred;
}

char* status_to_string(gfstatus_t status) {
    char* status_string;

    switch (status){
        case 200:
            status_string = "OK";
            break;
        case 400:
            status_string = "FILE_NOT_FOUND";
            break;
        case 500:
            status_string = "ERROR";
            break;
    }

    return status_string;
}

//revise starts here
ssize_t gfs_sendheader(gfcontext_t **ctx, gfstatus_t status, size_t file_len){
	char messageBuffer[BUFFERLEN];
	char statmsg[16];
	ssize_t sendMsgSize;
	ctx->bytes_remaining = file_len;
	if(status == GF_OK){
		strcpy(statmsg,"OK");
		sprintf(messageBuffer, "GETFILE %s %d%s", statmsg,file_len,MARKER);
	} else {
		if (status == GF_FILE_NOT_FOUND) 
			strcpy(statmsg,"FILE_NOT_FOUND");
		else if (status == GF_ERROR)
			strcpy(statmsg,"ERROR");
		else
			strcpy(statmsg,"INVALID");
		sprintf(messageBuffer, "GETFILE %s%s", statmsg,MARKER);
	}

	if((sendMsgSize = send(ctx->socket, messageBuffer, BUFFERLEN, 0)) == -1){
		fprintf(stderr, "%s @ %d: [SERVER] Failure to send() header\n", __FILE__, __LINE__);
		return -1;
	}
	if(status != GF_OK)
		gfs_abort(ctx);
    return -1;
}

void gfserver_serve(gfserver_t **gfs){

}

void gfserver_set_handlerarg(gfserver_t **gfs, void* arg){
	int serverSocket;
	char portnumchar[6];
	char messageBuffer[BUFFERLEN];
	struct addrinfo hints, *serverinfo, *p;
	struct sockaddr their_addr;
	socklen_t sin_size;
	int yes=1;
	int returnvalue,recvMsgSize;

	sprintf(portnumchar, "%d", gfs->port);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((returnvalue = getaddrinfo(LOCALHOST, portnumchar, &hints, &serverinfo)) != 0){
		fprintf(stderr, "%s @ %d: [SERVER] Failure at getaddrinfo() (%s)\n", __FILE__, __LINE__, gai_strerror(returnvalue));
		free(gfs);
		exit(1);
	}

	for(p = serverinfo; p != NULL; p = p->ai_next){
		if((serverSocket = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at socket()\n", __FILE__, __LINE__);
			continue;
		}

		if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at setsockopt()\n", __FILE__, __LINE__);
			free(gfs);
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
		free(gfs);
		exit(1);
	}

	if(listen(serverSocket, gfs->max_npending) == -1){
		fprintf(stderr, "%s @ %d: [SERVER] Failure at listen() with backlog (%d)\n", __FILE__, __LINE__, gfs->max_npending);
		free(gfs);
		exit(1);
	}

	char * path;
	if((path = malloc(200)) == NULL){
		fprintf(stderr, "%s @ %d: [SERVER] Failure to malloc() the path variable\n", __FILE__, __LINE__);
		free(gfs);
		exit(1);
	}

	struct gfcontext_t *context;
	if((context = malloc(sizeof(struct gfcontext_t))) == NULL){
		fprintf(stderr, "%s @ %d: [SERVER] Failure to malloc() the context\n", __FILE__, __LINE__);
		free(path);
		free(gfs);
		exit(1);
	}

	while(1){
		sin_size = sizeof their_addr;
		context->socket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
		if(context->socket == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at accept()\n", __FILE__, __LINE__);
			continue;
		}

		if((recvMsgSize = recv(context->socket, messageBuffer, BUFFERLEN-1, 0)) == -1){
			fprintf(stderr, "%s @ %d: [SERVER] Failure at recv()\n", __FILE__, __LINE__);
			free(context);
			free(path);
			free(gfs);
			exit(1);
		}

		messageBuffer[recvMsgSize] = '\0';

		char scheme[8];
		memset(&scheme, 0, sizeof scheme);
		char method[4];
		memset(&method, 0, sizeof method);
		char marker[5];
		memset(&marker, 0, sizeof marker);
		char *token = strtok(messageBuffer, " ");
		strncpy(scheme,token,sizeof scheme);
		token = strtok(NULL, " ");
		strncpy(method,token,sizeof method);
		token = strtok(NULL, " ");
		sscanf( token+sscanf( token, "%[^\r]", path ), "%[\r\n]", marker );
		printf("[debug-server] The path is %s\n",path);
		if((strcmp(scheme,SCHEME) != 0) || (strcmp(method,METHOD) != 0) || (strcmp(marker,MARKER) != 0)){
			fprintf(stderr, "%s @ %d: [SERVER] Client sent malformed header\n", __FILE__, __LINE__);
			gfs_sendheader(context,GF_INVALID,0);
			continue;
		}

		gfs->handler(context,path,gfs->handler_arg);
		memset(&messageBuffer, 0, sizeof messageBuffer);

		close(serverSocket);
		free(gfs);
		free(path); 
	}
}

void gfserver_set_handler(gfserver_t **gfs, gfh_error_t (*handler)(gfcontext_t **, const char *, void*)){
	gfs->handler = handler;
}

void gfserver_set_maxpending(gfserver_t **gfs, int max_npending){
	gfs->max_npending = max_npending;
}

void gfserver_set_port(gfserver_t **gfs, unsigned short port){
	gfs->port = port;
}


