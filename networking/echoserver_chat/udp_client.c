/**********************************************************
 *                  TCP ECHO CLIENT
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "socket_utils.h"

#define HOST    "localhost"
#define PORT    5000

int main (int argc, char *argvp[])
{
    int sockfd = INVALID_FD;

    if (!create_socket(&sockfd, SOCK_DGRAM)) {
        printf("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Connecting socket\n");
    if (!connect_socket(sockfd, HOST, PORT)) {
        printf("Connect socket failed\n");
        close_socket(sockfd);
        exit(EXIT_FAILURE);
    }

    chat_session(sockfd, "server", true);

    close_socket(sockfd);

    exit(EXIT_SUCCESS);
}
