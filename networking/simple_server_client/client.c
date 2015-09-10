/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(){
  int clientSocket;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);

  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(7891);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Connect the socket to the server using the address struct ----*/
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
    if (cmsg == NULL || cmsg -> cmsg_type != SCM_RIGHTS) {
      printf("The first control structure contains no file descriptor.\n");
    }
    else
    {
      int pass_sd;
      memcpy(&pass_sd, CMSG_DATA(cmsg), sizeof(pass_sd));
      printf("Received descriptor = %d\n", pass_sd);
    }
  }

  /*---- Read the message from the server into the buffer ----*/
  recv(clientSocket, buffer, 1024, 0);

  /*---- Print the received message ----*/
  printf("Data received: %s",buffer);

  close(clientSocket);
  return 0;
}
