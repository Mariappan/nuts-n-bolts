/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>

int main(){
  int clientSocket;
  socklen_t addr_size;
  int recvbytes;

#ifdef SOCK_UNIX
  char *socket_path = "\0socket";
  struct sockaddr_un serverAddr;

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sun_family = AF_UNIX;
  strncpy(serverAddr.sun_path, socket_path, sizeof(serverAddr.sun_path)-1);

  clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
  printf ("Using UNIX socket:%d\n", clientSocket);
  int on = 1;
  if (-1 == setsockopt(clientSocket, IPPROTO_IP, IP_PKTINFO,(char *)&on, sizeof on)) {

      perror("sesockopt error");
      printf("Error no is %d\n", errno);
  }
#else
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(40899);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  printf ("Using INET socket:%d\n", clientSocket);

  int on = 1;
  if (-1 == setsockopt(clientSocket, IPPROTO_IP, IP_PKTINFO,(char *)&on, sizeof on)) {
      perror("sesockopt error");
      printf("Error no is %d\n", errno);
  }
#endif

  addr_size = sizeof serverAddr;
  if (-1 == connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)) {
      perror ("Connect error\n");
  }

#if 1
  {
      struct msghdr child_msg;
      struct iovec iov[1];
      char buf[1000];

      memset(&child_msg,   0, sizeof(child_msg));

      child_msg.msg_iov = iov; /* vector for scatter read */
      child_msg.msg_iovlen = 1; /* one buffer for scatter read */
      iov[0].iov_base = buf; /* data we receive from the peer */
      iov[0].iov_len = sizeof buf;

      char cmsgbuf[CMSG_SPACE(1000)];

      child_msg.msg_control = cmsgbuf; // make place for the ancillary message to be received
      child_msg.msg_controllen = sizeof(cmsgbuf);

      printf("Waiting on recvmsg\n");
      recvbytes = recvmsg(clientSocket, &child_msg, 0);
      printf("recvmsg return:%d\n", recvbytes);


      if (recvbytes == -1)
          printf("Error no is %d\n", errno);
      else
          printf("recvbytes: %d\n", recvbytes);

      if (recvbytes >= 0)
      {
          struct cmsghdr *cmsg = CMSG_FIRSTHDR(&child_msg);
          printf("Cmsg is %p Msg_type:%d.\n", cmsg, cmsg?cmsg->cmsg_type:0);
          if (cmsg == NULL) {
              printf("The first control structure contains no file descriptor.\n");
          }
          else
          {
              int pass_sd;
              memcpy(&pass_sd, CMSG_DATA(cmsg), sizeof(pass_sd));
              printf("Received descriptor = %u %u\n", pass_sd, htons(pass_sd));
          }

          printf("Recieved data thro recvmsg is %s\n", buf);
      }
  }

#else

  char buffer[1024];
  recvbytes = recv(clientSocket, buffer, 1024, 0);
  printf("Data received: %d %s", recvbytes, buffer);

#endif

  close(clientSocket);
  return 0;
}
