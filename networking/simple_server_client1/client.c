/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>

int main(){
  int clientSocket;
  char buffer[1024];
  socklen_t addr_size;
  char *socket_path = "./socket";
  struct sockaddr_un serverAddr;

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sun_family = AF_UNIX;
  strncpy(serverAddr.sun_path, socket_path, sizeof(serverAddr.sun_path)-1);
  unlink(socket_path);

  clientSocket = socket(PF_UNIX, SOCK_STREAM, 0);

  addr_size = sizeof serverAddr;
  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

  {
      struct msghdr child_msg;
      int rc;

      memset(&child_msg,   0, sizeof(child_msg));
      char cmsgbuf[CMSG_SPACE(sizeof(int))];
      child_msg.msg_control = cmsgbuf; // make place for the ancillary message to be received
      child_msg.msg_controllen = sizeof(cmsgbuf);

      printf("Waiting on recvmsg\n");
      rc = recvmsg(clientSocket, &child_msg, 0);
      printf("recvmsg return:%d\n", rc);

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
  }

  recv(clientSocket, buffer, 1024, 0);
  printf("Data received: %s",buffer);

  close(clientSocket);
  return 0;
}
