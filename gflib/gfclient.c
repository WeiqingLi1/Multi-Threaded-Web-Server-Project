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
#include "gfclient.h"
#include "gfclient-student.h"

#define SCHEME "GETFILE"
#define BUFSIZE 2000


typedef struct gfcrequest_t{
    struct sockaddr_in server_addr;
    char* request_path;
    void (*writefunc)(void*, size_t, void *);
    void *writearg;
    void (*headerfunc)(void*, size_t, void *);
    void *headerarg;
    gfstatus_t res_status;
    size_t res_length;
    size_t total_response_size;
} gfcrequest_t;

// optional function for cleaup processing.
void gfc_cleanup(gfcrequest_t **gfr){
	free(gfr);
}

gfcrequest_t *gfc_create(){
    // dummy for now - need to fill this part in
	gfcrequest_t* gfc = (gfcrequest_t*)malloc(sizeof(gfcrequest_t));
	gfc->server_addr.sin_family = AF_INET;
	gfc->headerfunc = 0;
	gfc->writefunc = 0;
	gfc->res_status = GF_INVALID;

    return (gfcrequest_t *)NULL;

}

size_t gfc_get_bytesreceived(gfcrequest_t **gfr){
    // not yet implemented
    return gfr->total_response_size;
}

size_t gfc_get_filelen(gfcrequest_t **gfr){
    // not yet implemented
    return gfr->res_length;
}

gfstatus_t gfc_get_status(gfcrequest_t **gfr){
    // not yet implemented
    return gfr->res_status;
}

void gfc_global_init(){
}

void gfc_global_cleanup(){
}

// add some self function
int connect_to_server(gfcrequest_t* gfr){
    int socketfd;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1){
        perror("Could not create a socket");
        return -1;
    }

    if (connect(socketfd, (struct sockaddr *)&(gfr->server_addr), sizeof(gfr->server_addr)) < 0){
        perror("Error during connect\n");
        return -1;
    }

    printf("Connected to server.\n");

    return socketfd;
}

int send_request_to_server(int socketfd, gfcrequest_t* gfr) {
    char* request_str = calloc(BUFSIZE, sizeof(char));
    sprintf(request_str, "%s %s %s\r\n\r\n", SCHEME, "GET", gfr->request_path);

    if( write(socketfd , request_str, strlen(request_str) + 1) != strlen(request_str) + 1){
        perror("Message send failed\n");
        free(request_str);
        return -1;
    }

    printf("Send request: %s\n", request_str);

    free(request_str);
    return 0;
}

gfstatus_t str_to_status(char* strstatus) {
    if (strcmp(strstatus, "OK") == 0){
        return GF_OK;
    }
    else if (strcmp(strstatus, "FILE_NOT_FOUND") == 0){
        return GF_FILE_NOT_FOUND;
    }
    else if (strcmp(strstatus, "ERROR") == 0){
        return GF_ERROR;
    }
    else {
        return GF_INVALID;
    }
}

ssize_t read_server_response_header(int socketfd, gfcrequest_t* gfr){
    char *read_buffer;
    char *header_buffer;
    char *header;
    char *strstatus;
    char *scheme;
    ssize_t response_size;
    gfstatus_t status;

    read_buffer = (char*)calloc(BUFSIZE, sizeof(char));
    strstatus = (char*)calloc(BUFSIZE, sizeof(char));
    scheme = (char*)calloc(BUFSIZE, sizeof(char));

    if ((response_size = read(socketfd, read_buffer, BUFSIZE)) < 0) {
        perror("Error during header read from response\n");
    }
    else {
        printf("Read buffer: %s\n", read_buffer);

        header_buffer = strtok(read_buffer, "\r\n\r\n");

        sscanf(header_buffer, "%s %s %zu", scheme, strstatus, &(gfr->res_length));

        if (strcmp(scheme, SCHEME) != 0) {
            printf("Scheme from header not GETFILE\n");
            response_size = -1;
        } else if ((status = str_to_status(strstatus)) == GF_INVALID) {
            printf("Invalid status\n");
            response_size = -1;
        } else {
            header = (char*)calloc(strlen(header_buffer) + 1, sizeof(char));
            strncpy(header, header_buffer, strlen(header_buffer));
            gfr->res_status = status;

            printf("Response header: %s\n", header);
            printf("Response status: %s\n", strstatus);
            printf("Response length: %zu\n", gfr->res_length);

		
            if (gfr->headerfunc != 0) {
                printf("Call headerfunc\n");
                (*(gfr->headerfunc))((void*)header, strlen(header), gfr->headerarg);
            }

            free(header);
        }
    }

    free(scheme);
    free(strstatus);
    free(read_buffer);

    return response_size;
}

int gfc_perform(gfcrequest_t **gfr){
    // currently not implemented.  You fill this part in.
	int socketfd;
	ssize_t response_size;
	size_t total_response_size = 0;
	size_t res_length;
	void* content_buffer;



	if((socketfd = connect_to_server(gfr)) < 0){
	
		return -1;
	}

	if(send_request_to_server(socketfd, gfr) < 0){
		
		return -1;
	}

	if((read_server_response_header(socketfd, gfr)) < 0){
		
		return -1;
	}
	
	if(gfr->res_status == GF_OK){
		printf("Start reading content\n");
		res_length = gfr->res_length;
		content_buffer = malloc(res_length);
		
		do {
			response_size = recv(socketfd, content_buffer, res_length, 0);
			if(response_size < 0){
				perror("Error during content read\n");
				free(content_buffer);
				return -1;
			}
			
			printf("Response size: %zu\n", response_size);
			printf("Content chunk and writecb called: \n");
			if(gfr->writefunc != 0){
				(*(gfr->writefunc))(content_buffer, (size_t)response_size, gfr->writearg);
			}
			
			total_response_size += response_size;
			res_length -= response_size;
			printf("Total response size: %zu\n", total_response_size);
		} while ();
	}

	gfr->total_response_size = total_response_size;
	printf("Final total response size: %zu\n", total_response_size);
	free(content_buffer);
	
	if(total_response_size < gfr->res_length){
		printf("Only %zu out of %zu received\n", total_response_size, gfr->res_length);
		return -1;
	}


	return 0;
}

void gfc_set_headerarg(gfcrequest_t **gfr, void *headerarg){
	gfr->headerarg = headerarg;
}

void gfc_set_headerfunc(gfcrequest_t **gfr, void (*headerfunc)(void*, size_t, void *)){
	gfr->headerfunc = headerfunc;
}

void gfc_set_path(gfcrequest_t **gfr, const char* path){
	gfr->request_path = path;
}

void gfc_set_port(gfcrequest_t **gfr, unsigned short port){
	gfr->server_addr.sin_port = htons(port);
}

void gfc_set_server(gfcrequest_t **gfr, const char* server){
	struct hostent* server_info = gethostbyname(server);
	if(server_info == NULL){
		fprintf(stderr,"Cannot find host with name %s\n", server);
		exit(1);
	}
	memcpy(&(gfr->server_addr.sin_addr.s_addr), server_info->h_addr, server_info->h_length);
}

void gfc_set_writearg(gfcrequest_t **gfr, void *writearg){
	gfr->writearg = writearg;
}

void gfc_set_writefunc(gfcrequest_t **gfr, void (*writefunc)(void*, size_t, void *)){
	gfr->writefunc = writefunc;
}

const char* gfc_strstatus(gfstatus_t status){
    const char *strstatus = NULL;

    switch (status){
    	case GF_OK:
		strstatus = "OK";
		break;
        case GF_ERROR:
            strstatus = "ERROR";
            break;
        case GF_FILE_NOT_FOUND:
            strstatus = "FILE_NOT_FOUND";
            break;
        default:
            strstatus = "INVALID";
    }

    return strstatus;
}

