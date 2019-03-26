#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "socket_utils.h"

bool create_socket(int *fd, int ip_type)
{
    int sockfd = INVALID_FD;

    sockfd = socket(AF_INET, ip_type, 0);
    if(sockfd < 0)
    {
        printf("Error opening socket\n");
        return false;
    }

    *fd = sockfd;
    return true;
}

void close_socket(int fd)
{
    if (INVALID_FD != fd) {
        close(fd);
    }
}

bool bind_socket(int fd, uint16_t port)
{
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if(bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Error binding address and port to socket\n");
        return false;
    }

    return true;
}

bool connect_socket(int fd, char *hostname, uint16_t port)
{
    struct hostent *server = NULL;
    struct sockaddr_in server_addr;

    server = gethostbyname(hostname);
    if(NULL == server)
    {
        printf("gethostbyname() failed");
        return false;
    }

    server_addr.sin_family = AF_INET;
    memcpy(server->h_addr, &(server_addr.sin_addr.s_addr), server->h_length);
    server_addr.sin_port = htons(port);

    // printf("IP:%u port:%u fd:%d\n", server_addr.sin_addr.s_addr, port, fd);
    if(connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connecting to server failed\n");
        return false;
    }

    return true;
}

void echo_session(int fd, char *end)
{
    char message[MSG_LEN];
    size_t bytes = 0;

    do {
        memset(message, 0, sizeof(message));

        bytes = recv(fd, message, sizeof(message), 0);
        if(bytes < 0) {
            printf("Error reading from socket\n");
            break;
        }
        message[MSG_LEN - 1] = '\0';

        printf("Reply from %s: %s\n", end, message);

        bytes = send(fd, message, ((strlen(message)) + 1), 0);
        if(bytes < 0) {
            printf("Error sending message to server\n");
            break;
        }

        if(0 == strncmp(message, "exit", 4)) {
            printf("Exiting..\n");
            break;
        }

    } while(true);

    return;
}

void chat_session(int fd, char *end, bool start_first)
{
    char message[MSG_LEN];
    size_t bytes = 0;
    bool connected = false;

    do {
        if (start_first || connected) {

            memset(message, 0, sizeof(message));

            printf("Enter message: ");
            fgets(message, (sizeof(message)), stdin);
            message[MSG_LEN - 1] = '\0';

            bytes = send(fd, message, ((strlen(message)) + 1), 0);
            if(bytes < 0) {
                printf("Error sending message to server\n");
                break;
            }

            if(0 == strncmp(message, "exit", 4)) {
                printf("Exiting..\n");
                break;
            }
        }

        connected = true;
        memset(message, 0, sizeof(message));

        bytes = recv(fd, message, sizeof(message), 0);
        if(bytes < 0) {
            printf("Error reading from socket\n");
            break;
        }
        message[MSG_LEN - 1] = '\0';

        printf("Reply from %s: %s\n", end, message);

    } while(true);

    return;
}


