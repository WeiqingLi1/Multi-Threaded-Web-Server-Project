
#include "gfclient-student.h"

// optional function for cleaup processing.
void gfc_cleanup(gfcrequest_t **gfr){
	free(gfr);
}

gfcrequest_t *gfc_create(){
    // dummy for now - need to fill this part in
	struct gfcrequest_t *gfr;
	if((gfr = malloc(sizeof(struct gfcrequest_t))) != NULL){
		gfr->thing = 0;
		return gfr;
	}
    return (gfcrequest_t *)NULL;

}

size_t gfc_get_bytesreceived(gfcrequest_t **gfr){
    // not yet implemented
    return gfr->bytes_received;
}

size_t gfc_get_filelen(gfcrequest_t **gfr){
    // not yet implemented
    return gfr->file_size;
}

gfstatus_t gfc_get_status(gfcrequest_t **gfr){
    // not yet implemented
    return gfr->status;
}

void gfc_global_init(){
}

void gfc_global_cleanup(){
}

int gfc_perform(gfcrequest_t **gfr){
    // currently not implemented.  You fill this part in.
    gfr->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct timeval timeout;
	timeout.tv_sec = 5.0;
	timeout.tv_usec = 0;
	setsockopt(gfr->socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	(gfr->server_address).sin_family=AF_INET;
	(gfr->server_address).sin_port=htons(gfr->portno);

	struct hostent *host;
	host = gethostbyname(gfr->host);
	memcpy(&(gfr->server_address).sin_addr, host->h_addr_list[0], host->h_length);

	int status = connect(gfr->socket, (struct sockaddr *) &gfr->server_address, sizeof(gfr->server_address));

	if(status != 0){
		close(gfr->socket);
		return -15;
	}

	sendFileRequest(gfr);
	int header_status = recvStatusAndSize(gfr);

	if(header_status < 0){
		close(gfr->socket);
		return header_status;
	}

	if(gfr->status != GF_OK){
		close(gfr->socket);
		return 0;
	}

	int file_trans_status = recvFileData(gfr);
	if(file_trans_status < 0){
		close(gfr->socket);
		return file_trans_status;
	}

	close(gfr->socket);
	return 0;
}

void gfc_set_headerarg(gfcrequest_t **gfr, void *headerarg){
	gfr->headerarg = headerarg;
}

void gfc_set_headerfunc(gfcrequest_t **gfr, void (*headerfunc)(void*, size_t, void *)){
	gfr->headerfunc = headerfunc;
}

void gfc_set_path(gfcrequest_t **gfr, const char* path){
	memset(&gfr->path, 0, sizeof gfr->path);
	gfr->path = path;
}

void gfc_set_port(gfcrequest_t **gfr, unsigned short port){
	gfr->port = port;
}

void gfc_set_server(gfcrequest_t **gfr, const char* server){
	memset(&gfr->server, 0, sizeof gfr->server);
	gfr->server = server;
}

void gfc_set_writearg(gfcrequest_t **gfr, void *writearg){
	gfr->writearg = writearg;
}

void gfc_set_writefunc(gfcrequest_t **gfr, void (*writefunc)(void*, size_t, void *)){
	gfr->writefunc = writefunc;
}

const char* gfc_strstatus(gfstatus_t status){
    const char *strstatus = NULL;

    switch (status)
    {
        default: {
            strstatus = "UNKNOWN";
        }
        break;

        case GF_INVALID: {
            strstatus = "INVALID";
        }
        break;

        case GF_FILE_NOT_FOUND: {
            strstatus = "FILE_NOT_FOUND";
        }
        break;

        case GF_ERROR: {
            strstatus = "ERROR";
        }
        break;

        case GF_OK: {
            strstatus = "OK";
        }
        break;
        
    }

    return strstatus;
}

