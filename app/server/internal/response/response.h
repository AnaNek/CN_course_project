#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "status_codes.h"
#include "response_constants.h"

#define HEADER_CHUNK_SIZE 128
#define ERROR_CHUNK_SIZE 256
#define BUFFER_SIZE 1024

int send_response(int client_sockfd, char *method, int pid);

int send_error_response(int client_sockfd, int error_code);


#endif
