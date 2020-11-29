#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "response.h"
#include "../../types.h"

char* get_status_string(int status_code);
char* get_content_type(char* path);

int send_response(int client_sockfd, char* path) {
    struct stat st;
    int rc = stat(path, &st);
    if (rc == -1) {
        return ERR_STAT;
    }
    int content_len = st.st_size;
    char* content_type = get_content_type(path);
    char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
    if (header == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(header, PROTOCOL " 200 OK" NEW_LINE \
                    "Connection: close" NEW_LINE \
                    "Server: dpudov" NEW_LINE \
                    "Content-Length: %d" NEW_LINE \
                    "Content-Type: %s" NEW_LINE NEW_LINE, \
                    content_len, content_type);
    send(client_sockfd, header, strlen(header), 0);
    free(header);

    int buf[BUFFER_SIZE] = {0};
    int fd = open(path, O_RDONLY);
    size_t read_bytes = 0;
    while ((read_bytes = read(fd, buf, BUFFER_SIZE * sizeof(int))) > 0) {
        send(client_sockfd, buf, read_bytes, 0);
    }
    close(fd);
    return SUCCESS;
}

int send_error_response(int client_sockfd, int error_code) {
    char* response = (char*) malloc(ERROR_CHUNK_SIZE);
    if (response == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(response, PROTOCOL " %d %s" NEW_LINE, \
                    error_code, get_status_string(error_code));
    send(client_sockfd, response, strlen(response), 0);
    free(response);
    return SUCCESS;
}

char* get_status_string(int status_code) {
    switch (status_code) {
        case STATUS_OK:
            return "OK";
        case STATUS_UNAUTHORIZED:
            return "Unauthorized";
        case STATUS_NOT_FOUND:
            return "Not Found";
        case STATUS_FORBIDDEN:
            return "Forbidden";
        case STATUS_BAD_REQUEST:
            return "Bad Request";
        case STATUS_METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case STATUS_SERVER_INTERNAL_ERROR:
            return "Internal Server Error";
        default:
            return "Not Implemented";
    }
}

char* get_content_type(char* path) {
    if (path == NULL) {
        return "text/plain";
    }
    char* find_dot = strrchr(path, '.');
    if (find_dot == NULL) {
        return "text/plain";
    }
    char* dot = find_dot + 1;
    if (strcmp(dot, "html") == 0) return "text/html";
    if (strcmp(dot, "css") == 0) return "text/css";
    if (strcmp(dot, "xml") == 0) return "text/xml";
    if (strcmp(dot, "png") == 0) return "image/png";
    if (strcmp(dot, "jpg") == 0 ||\
        strcmp(dot, "jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, "gif") == 0) return "image/gif";
    if (strcmp(dot, "php") == 0) return "text/php";

    return "text/plain";
}
