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
  char buffer[1024];
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  char *socket_path = "./socket";
  struct sockaddr_un serverAddr;

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sun_family = AF_UNIX;
  strncpy(serverAddr.sun_path, socket_path, sizeof(serverAddr.sun_path)-1);
  unlink(socket_path);


  welcomeSocket = socket(PF_UNIX, SOCK_STREAM, 0);

  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
  {
      struct msghdr parent_msg;
      size_t length = 4089;

      memset(&parent_msg, 0, sizeof(parent_msg));
      struct cmsghdr *cmsg;
      char cmsgbuf[CMSG_SPACE(sizeof(newSocket))];
      parent_msg.msg_control = cmsgbuf;
      parent_msg.msg_controllen = sizeof(cmsgbuf);
      cmsg = CMSG_FIRSTHDR(&parent_msg);
      // cmsg->cmsg_level = SOL_SOCKET;
      // cmsg->cmsg_type = SCM_RIGHTS;
      cmsg->cmsg_len = CMSG_LEN(sizeof(newSocket));
      memcpy(CMSG_DATA(cmsg), &length, sizeof(length));
      parent_msg.msg_controllen = cmsg->cmsg_len; // total size of all control blocks

      if((sendmsg(newSocket, &parent_msg, 0)) < 0)
      {
          perror("sendmsg()");
          exit(0);
      }
  }

  /*---- Send message to the socket of the incoming connection ----*/
  int opt = 0;
  strcpy(buffer,"Hello World\n");
  send(newSocket,buffer,13,0);

  close(newSocket);
  close(welcomeSocket);

  return 0;
}
