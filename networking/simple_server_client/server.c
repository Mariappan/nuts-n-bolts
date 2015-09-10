/****************** SERVER CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(){
  int welcomeSocket, newSocket;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(7891);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Bind the address struct to the socket ----*/
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

#if 0
  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
#else
  newSocket = welcomeSocket;
#endif

  {
    struct msghdr child_msg;
    int rc;

    memset(&child_msg,   0, sizeof(child_msg));
    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    child_msg.msg_control = cmsgbuf; // make place for the ancillary message to be received
    child_msg.msg_controllen = sizeof(cmsgbuf);

    printf("Waiting on recvmsg\n");
    rc = recvmsg(newSocket, &child_msg, 0);
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

  /*---- Send message to the socket of the incoming connection ----*/
  strcpy(buffer,"Hello World\n");
  send(newSocket,buffer,13,0);

  close(newSocket);
  close(welcomeSocket);

  return 0;
}
