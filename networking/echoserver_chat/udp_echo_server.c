/**********************************************************
 *                  TCP ECHO SERVER
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include "socket_utils.h"

#define HOST    "localhost"
#define PORT    5000

int main (int argc, char *argvp[])
{
    char message[MSG_LEN];
    int udp_sockfd = INVALID_FD;
    struct sockaddr_in client_addr;
    socklen_t client_size = sizeof(client_addr);
    size_t bytes = 0;

    if (!create_socket(&udp_sockfd, SOCK_DGRAM)) {
        printf("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    if (!bind_socket(udp_sockfd, PORT)) {
        printf("Bind socket failed\n");
        close_socket(udp_sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connection...\n");

    bytes = recvfrom(udp_sockfd, message, MSG_LEN, 0,(struct sockaddr *)&client_addr, &client_size);
    printf("Reply from client: %s\n", message);
    sendto(udp_sockfd, message, bytes, 0, (struct sockaddr *)&client_addr, client_size);

    if (!connect_socket(udp_sockfd, HOST, htons(client_addr.sin_port))) {
        printf("Connect socket failed\n");
        close_socket(udp_sockfd);
        exit(EXIT_FAILURE);
    }

    echo_session(udp_sockfd, "client");

    close_socket(udp_sockfd);

    exit(EXIT_SUCCESS);
}
