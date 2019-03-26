/**********************************************************
 *                  TCP ECHO SERVER
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include "socket_utils.h"

#define PORT    5000

int main (int argc, char *argvp[])
{
    int listening_sockfd = 0, connect_sockfd = 0;
    char message[1024] = {'\0'};
    struct sockaddr_in client_addr;
    socklen_t client_size = sizeof(client_addr);
    size_t bytes = 0;

    if (!create_socket(&listening_sockfd, SOCK_STREAM)) {
        printf("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    if (!bind_socket(listening_sockfd, PORT)) {
        printf("Bind socket failed\n");
        close_socket(listening_sockfd);
        exit(EXIT_FAILURE);
    }

    if(listen(listening_sockfd, 10) < 0) {
        printf("Error listening on socket\n");
        close_socket(listening_sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connection...\n");
    connect_sockfd = accept(listening_sockfd, (struct sockaddr *)&client_addr, &client_size);
    if(connect_sockfd < 0) {
        printf("Error on accepting client\n");
        close(listening_sockfd);
        exit(EXIT_FAILURE);
    }

    echo_session(connect_sockfd, "client");

    close_socket(connect_sockfd);
    close_socket(listening_sockfd);

    exit(EXIT_SUCCESS);
}
