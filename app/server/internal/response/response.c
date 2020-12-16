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

#include <sys/ioctl.h>
#include "../../../driver/src/tracer.h"

#define DEVICE_PATH	"/dev/tracer"

char* get_status_string(int status_code);
int process_showall_request(int client_sockfd, int pid);
int process_get_request(int client_sockfd, int pid);
int process_add_request(int client_sockfd, int pid);
int process_remove_request(int client_sockfd, int pid);
int calculate_content_len(void);

int send_response(int client_sockfd, char *method, int pid) {
    int rc = ERR_METHOD_NOT_ALLOWED;

    if (strcasecmp(method, SHOWALL) == 0)
    {
        rc = process_showall_request(client_sockfd, pid);
        return rc;
    }
    if (strcasecmp(method, GET) == 0)
    {
        rc = process_get_request(client_sockfd, pid);
        return rc;
    }
    if (strcasecmp(method, ADD) == 0)
    {
        rc = process_add_request(client_sockfd, pid);
        return rc;
    }
    if (strcasecmp(method, REMOVE) == 0)
    {
        rc = process_remove_request(client_sockfd, pid);
        return rc;
    }
    return rc;
}

int calculate_content_len(void)
{
    int content_len = 0;
    FILE *fp = NULL;
    char *command = "ps -ajx";
    char buf[BUFFER_SIZE] = {0};

    fp = popen(command, "r");
    while (fgets(buf, BUFFER_SIZE, fp) != NULL) {
        content_len += strlen(buf);
    }
    pclose(fp);
    return content_len;
}

int process_add_request(int client_sockfd, int pid)
{
    int fd;
    int rc = 0;
    int content_len = 0;

    fd = open(DEVICE_PATH, O_RDONLY);

    if (fd < 0)
    {
        char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
        if (header == NULL) {
            return ERR_MEMORY_ALLOCATION;
        }
        sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                        "Connection: close" NEW_LINE \
                        "Server: Ubuntu 20.04" NEW_LINE \
                        "Content-Length: %d" NEW_LINE \
                        "Processed PID : %d" NEW_LINE NEW_LINE, \
                        STATUS_DRIVER_ERROR, get_status_string(STATUS_DRIVER_ERROR), content_len, pid);
        send(client_sockfd, header, strlen(header), 0);
        free(header);
        return ERR_DRIVER;
    }

    rc = ioctl(fd, TRACER_ADD_PROCESS, pid);
    close(fd);

    if (rc)
    {
        char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
        if (header == NULL) {
            return ERR_MEMORY_ALLOCATION;
        }
        sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                        "Connection: close" NEW_LINE \
                        "Server: Ubuntu 20.04" NEW_LINE \
                        "Content-Length: %d" NEW_LINE \
                        "Processed PID : %d" NEW_LINE NEW_LINE, \
                        STATUS_IOCTL_ERROR, get_status_string(STATUS_IOCTL_ERROR), content_len, pid);
        send(client_sockfd, header, strlen(header), 0);
        free(header);
        return ERR_IOCTL;
    }

    char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
    if (header == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                    "Connection: close" NEW_LINE \
                    "Server: Ubuntu 20.04" NEW_LINE \
                    "Content-Length: %d" NEW_LINE \
                    "Processed PID : %d" NEW_LINE NEW_LINE, \
                    STATUS_OK, get_status_string(STATUS_OK), content_len, pid);
    send(client_sockfd, header, strlen(header), 0);
    free(header);

    return SUCCESS;
}

int process_remove_request(int client_sockfd, int pid)
{
    int fd;
    int rc = 0;
    int content_len = 0;

    fd = open(DEVICE_PATH, O_RDONLY);

    if (fd < 0)
    {
        char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
        if (header == NULL) {
            return ERR_MEMORY_ALLOCATION;
        }
        sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                        "Connection: close" NEW_LINE \
                        "Server: Ubuntu 20.04" NEW_LINE \
                        "Content-Length: %d" NEW_LINE \
                        "Processed PID : %d" NEW_LINE NEW_LINE, \
                        STATUS_DRIVER_ERROR, get_status_string(STATUS_DRIVER_ERROR), content_len, pid);
        send(client_sockfd, header, strlen(header), 0);
        free(header);
        return ERR_DRIVER;
    }

    rc = ioctl(fd, TRACER_REMOVE_PROCESS, pid);
    close(fd);

    if (rc)
    {
        char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
        if (header == NULL) {
            return ERR_MEMORY_ALLOCATION;
        }
        sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                        "Connection: close" NEW_LINE \
                        "Server: Ubuntu 20.04" NEW_LINE \
                        "Content-Length: %d" NEW_LINE \
                        "Processed PID : %d" NEW_LINE NEW_LINE, \
                        STATUS_IOCTL_ERROR, get_status_string(STATUS_IOCTL_ERROR), content_len, pid);
        send(client_sockfd, header, strlen(header), 0);
        free(header);
        return ERR_IOCTL;
    }

    char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
    if (header == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                    "Connection: close" NEW_LINE \
                    "Server: Ubuntu 20.04" NEW_LINE \
                    "Content-Length: %d" NEW_LINE \
                    "Processed PID : %d" NEW_LINE NEW_LINE, \
                    STATUS_OK, get_status_string(STATUS_OK), content_len, pid);
    send(client_sockfd, header, strlen(header), 0);
    free(header);

    return SUCCESS;
}

int process_get_request(int client_sockfd, int pid)
{
    int fd;
    int rc = 0;
    int content_len = 0;
    char buf[BUFFER_SIZE] = {0};
    int len = 0;
    int min = 0;
    driver_request_t req;
    char *db = req.buf;

    fd = open(DEVICE_PATH, O_RDONLY);

    if (fd < 0)
    {
        char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
        if (header == NULL) {
            return ERR_MEMORY_ALLOCATION;
        }
        sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                        "Connection: close" NEW_LINE \
                        "Server: Ubuntu 20.04" NEW_LINE \
                        "Content-Length: %d" NEW_LINE \
                        "Processed PID : %d" NEW_LINE NEW_LINE, \
                        STATUS_DRIVER_ERROR, get_status_string(STATUS_DRIVER_ERROR), content_len, pid);
        send(client_sockfd, header, strlen(header), 0);
        free(header);
        return ERR_DRIVER;
    }

    req.pid = pid;
    req.len = DRIVER_BUF_SIZE;  
    rc = ioctl(fd, TRACER_GET_INFO, &req);
    //printf("rc %d req %s\n", rc, req.buf);
    close(fd);

    if (rc)
    {
        char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
        if (header == NULL) {
            return ERR_MEMORY_ALLOCATION;
        }
        sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                        "Connection: close" NEW_LINE \
                        "Server: Ubuntu 20.04" NEW_LINE \
                        "Content-Length: %d" NEW_LINE \
                        "Processed PID : %d" NEW_LINE NEW_LINE, \
                        STATUS_IOCTL_ERROR, get_status_string(STATUS_IOCTL_ERROR), content_len, pid);
        send(client_sockfd, header, strlen(header), 0);
        free(header);
        return ERR_IOCTL;
    }

    content_len = DRIVER_BUF_SIZE;

    char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
    if (header == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                    "Connection: close" NEW_LINE \
                    "Server: Ubuntu 20.04" NEW_LINE \
                    "Content-Length: %d" NEW_LINE \
                    "Processed PID : %d" NEW_LINE NEW_LINE, \
                    STATUS_OK, get_status_string(STATUS_OK), content_len, pid);
    send(client_sockfd, header, strlen(header), 0);
    free(header);

    if (content_len <= BUFFER_SIZE)
    {
        send(client_sockfd, req.buf, content_len, 0);
    }
    else
    {
        len = content_len;
        while (len > 0)
        {
            min = (len < BUFFER_SIZE) ? len : BUFFER_SIZE;
            strncpy(buf, db, min);
            send(client_sockfd, buf, min, 0);
            len -= min;
            db = db + min;
        }
    }

    return SUCCESS;
}

int process_showall_request(int client_sockfd, int pid)
{
    int content_len = 0;
    FILE *fp = NULL;
    char *command = "ps -ajx";
    char buf[BUFFER_SIZE] = {0};

    content_len = calculate_content_len();

    char* header = (char*) malloc(HEADER_CHUNK_SIZE * sizeof(char));
    if (header == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(header, MY_PROTOCOL " %d %s" NEW_LINE \
                    "Connection: close" NEW_LINE \
                    "Server: Ubuntu 20.04" NEW_LINE \
                    "Content-Length: %d" NEW_LINE \
                    "Processed PID : %d" NEW_LINE NEW_LINE, \
                    STATUS_OK, get_status_string(STATUS_OK), content_len, pid);
    send(client_sockfd, header, strlen(header), 0);
    free(header);

    fp = popen(command, "r");

    if (fp == NULL) {
        printf("Could not perform ps -ajx\n");
        return ERR_OPEN;
    }
    while (fgets(buf, BUFFER_SIZE, fp) != NULL) {
        send(client_sockfd, buf, strlen(buf), 0);
    }
    pclose(fp);
    return SUCCESS;
}

int send_error_response(int client_sockfd, int error_code) {
    char* response = (char*) malloc(ERROR_CHUNK_SIZE);
    if (response == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }
    sprintf(response, MY_PROTOCOL " %d %s" NEW_LINE \
                    "Connection: close" NEW_LINE \
                    "Server: Ubuntu 20.04" NEW_LINE \
                    "Content-Length: %d" NEW_LINE \
                    "Processed PID : %d" NEW_LINE NEW_LINE, \
                    error_code, get_status_string(error_code), 0, -1);

    send(client_sockfd, response, strlen(response), 0);
    free(response);
    return SUCCESS;
}

char* get_status_string(int status_code) {
    switch (status_code) {
        case STATUS_OK:
            return "SUCCESS";
        case STATUS_PID_INCORRECT:
            return "PID Incorrect";
        case STATUS_DRIVER_ERROR:
            return "Driver error";
        case STATUS_BAD_REQUEST:
            return "Bad Request";
        case STATUS_METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case STATUS_SERVER_INTERNAL_ERROR:
            return "Internal Server Error";
        case STATUS_IOCTL_ERROR:
            return "IOCTL error";
        default:
            return "Not Implemented";
    }
}
