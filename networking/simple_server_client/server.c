/****************** SERVER CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>

int main(){
  int welcomeSocket, newSocket;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

#ifdef SOCK_UNIX
  char *socket_path = "\0socket";
  struct sockaddr_un serverAddr;

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sun_family = AF_UNIX;
  strncpy(serverAddr.sun_path, socket_path, sizeof(serverAddr.sun_path)-1);

  printf ("Using UNIX socket\n");
  welcomeSocket = socket(AF_UNIX, SOCK_STREAM, 0);
#else
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(40899);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  printf ("Using INET socket:%d\n", welcomeSocket);

  int on = 1;
  if (-1 == setsockopt(welcomeSocket, SOL_SOCKET, SO_REUSEADDR,(char *)&on, sizeof on)) {
      perror("sesockopt SO_REUSEADDR error");
  }

  if (-1 == setsockopt(welcomeSocket, IPPROTO_IP, IP_PKTINFO,(char *)&on, sizeof on)) {
      perror("sesockopt IP_PKTINFO error");
  }

#endif

  if (bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
      perror("bind error");
  }

  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

#ifndef SOCK_UNIX
  if (-1 == setsockopt(newSocket, IPPROTO_IP, IP_PKTINFO,(char *)&on, sizeof on)) {
      perror("sesockopt IP_PKTINFO error");
  }
#endif

#if 1
  {
      struct msghdr parent_msg;
      size_t length = 40;
      struct iovec iov[1];
      char buf[1000];

      memset(&parent_msg, 0, sizeof(parent_msg));
      parent_msg.msg_iov = iov; /* vector for scatter read */
      parent_msg.msg_iovlen = 1; /* one buffer for scatter read */
      iov[0].iov_base = buf; /* data we receive from the peer */
      iov[0].iov_len = sizeof buf;

      strcpy(buf, "Hi Mari\n");
      iov[0].iov_len = strlen(buf);

      struct cmsghdr *cmsg;
      char cmsgbuf[CMSG_SPACE(1000)];

      parent_msg.msg_control = cmsgbuf;
      parent_msg.msg_controllen = sizeof(cmsgbuf);

      cmsg = CMSG_FIRSTHDR(&parent_msg);
      cmsg->cmsg_level = IPPROTO_IP;
      cmsg->cmsg_type = IP_TTL;
      cmsg->cmsg_len = CMSG_LEN(sizeof(length));
      memcpy(CMSG_DATA(cmsg), &length, sizeof(length));

      parent_msg.msg_controllen = cmsg->cmsg_len; // total size of all control blocks

      if((sendmsg(newSocket, &parent_msg, 0)) < 0)
      {
          perror("sendmsg()");
          exit(0);
      }
  }
#else

  /*---- Send message to the socket of the incoming connection ----*/
  char buffer[1024];
  strcpy(buffer,"Hello World\n");
  send(newSocket,buffer,13,0);

#endif

  close(newSocket);
  close(welcomeSocket);

  return 0;
}
