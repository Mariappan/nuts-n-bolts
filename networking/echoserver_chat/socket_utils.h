#include <stdbool.h>

#define INVALID_FD (-1)

#define MSG_LEN 1024

bool create_socket(int *fd, int ip_type);

bool bind_socket(int fd, uint16_t port);

void close_socket(int fd);

bool connect_socket(int fd, char *hostname, uint16_t port);

void echo_session(int fd, char *end);

void chat_session(int fd, char *end, bool start);
