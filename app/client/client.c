#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>

#define BUFFER_SIZE 128000

#include "types.h"
#define CHUNK_SIZE 4096
#define MAX_REQUEST_SIZE 256

#define REQUEST "%s MY_PROTOCOL %s\r\n"

// ./client address port user

void handle_interrupt(int sig_num) {
    printf("Interrupt: %d\n", sig_num);
    exit(0);
}

void print_usage(void) {
    printf("Usage:\n ./client <adress> <port>\n");
}

int receive_in_chunks(int sockfd, char** response, int timeout) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    int total_size = 0;
    int size_recv = 0;
    int current_size = 0;
    struct timeval begin, now;
    double timediff;
    char* chunk = (char*) malloc(CHUNK_SIZE * sizeof(char));
    if (chunk == NULL) {
        return ERR_MEMORY_ALLOCATION;
    }

    *response = (char *) malloc(CHUNK_SIZE * sizeof(char));
    if (*response == NULL) {
        free(chunk);
        return ERR_MEMORY_ALLOCATION;
    }
    current_size = CHUNK_SIZE;
    gettimeofday(&begin , NULL);
    int count_chunks = 0;
    int content_length = 0;
    while (count_chunks <= 1 || total_size < content_length) {
        gettimeofday(&now , NULL);
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);

        if (timediff > timeout) {
            break;
        } else if (timediff > timeout * 2) {
            break;
        }

        memset(chunk, 0, CHUNK_SIZE);
        size_recv = recv(sockfd, chunk, CHUNK_SIZE, MSG_DONTWAIT);
        if (size_recv == -1) {
            usleep(100 * 1000);
        } else {
			count_chunks++;
			if (count_chunks == 1) {
				char *cl_beg = strstr(chunk, CONTENT_LENGTH);
				if (cl_beg == NULL) {
					free(chunk);
					return ERR_PARAMS;
				}
				char* cl_end = strstr(cl_beg, NEW_LINE);
			    if (cl_end == NULL) {
			        free(chunk);
			        return ERR_PARAMS;
			    }
				content_length = atoi(cl_beg + strlen(CONTENT_LENGTH));
			}
            if (total_size + size_recv > current_size) {
                char* tmp = (char*) realloc(*response, 2*current_size);
                if (tmp == NULL) {
                    free(chunk);
                    return ERR_MEMORY_ALLOCATION;
                }
                *response = tmp;
                current_size *= 2;
            }
            memcpy(*response + total_size, chunk, size_recv);
            total_size += size_recv;
            gettimeofday(&begin , NULL);
        }
    }
    printf("count_chunks:%d total_size: %d content_length: %d\n", count_chunks, total_size , content_length );
    return SUCCESS;
}

int main(int argc, char *argv[])
{

    if (argc < ARGC)
    {
        print_usage();
        return -1;
    }
    char *address = argv[1];
    int port = atoi(argv[2]);
    int rc = 0;
    int client_socket_fd;
    char input[256];
    char method[64];
    char pid[64] = "";
    char request[1024];
    char* response = NULL;

    signal(SIGINT, handle_interrupt);
    printf("Ctrl+C to exit established\n");

    //Создание сокета

    if ((client_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in rem_address;
    struct hostent * rem;

    rem_address.sin_family = AF_INET;
    rem_address.sin_port = htons(port);

    if ((rem = gethostbyname(address)) == NULL)
    {
        herror("Error in gethostbyname");
        exit(EXIT_FAILURE);
    }

    memcpy (&rem_address.sin_addr, rem -> h_addr, rem -> h_length);

    if (connect(client_socket_fd, (struct sockaddr *) &rem_address, sizeof(rem_address)) < 0)
    {
        perror("Error connecting");
        exit(EXIT_FAILURE);
    }

    puts("Give a server request...");
    fgets(input, sizeof(input), stdin);

    sscanf(input,"%s%s", method, pid);
    //printf("%s\n", pid);
    sprintf(request, REQUEST, method, pid);

    printf("%s", request);

    send(client_socket_fd, request, sizeof(request), 0);

    rc = receive_in_chunks(client_socket_fd, &response, 1);

    printf("rc = %d\n", rc);
    printf("\n%s\n", response);

    free(response);
    close(client_socket_fd);

    return 0;
}
