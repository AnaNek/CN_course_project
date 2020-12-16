#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "request/request.h"
#include "response/response.h"
#include "handler.h"
#include "../types.h"

void handle_new_connection(void* arg) {
    int client_sockfd = *(int *)arg;
    int rc = SUCCESS;
    request_t* request = NULL;
    rc = request_parse(&request, client_sockfd);
    printf("[%d] Request parse finished with status %d\n", client_sockfd, rc);

    if (rc != SUCCESS)
    {
        switch (rc) {
          case ERR_PID:
              send_error_response(client_sockfd, STATUS_PID_INCORRECT);
              break;
          case ERR_METHOD_NOT_ALLOWED:
              send_error_response(client_sockfd, STATUS_METHOD_NOT_ALLOWED);
              break;
          case ERR_PROTOCOL:
              send_error_response(client_sockfd, STATUS_PROTOCOL_NOT_ALLOWED);
              break;
          default:
              send_error_response(client_sockfd, STATUS_BAD_REQUEST);
              break;
        }
        close(client_sockfd);
        printf("Couldn't parse request with status: %d\n", rc);
        return;
    }

    int pid = get_pid(request);
    char *method = get_method(request);

    send_response(client_sockfd, method, pid);
    request_free(request);
    close(client_sockfd);
}
