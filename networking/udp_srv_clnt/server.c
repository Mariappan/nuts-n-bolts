
/* Sample UDP server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];

   sockfd=socket(AF_UNIX,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_UNIX;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   for (;;)
   {
#if 1
      {
         struct msghdr child_msg;
         int rc;

         memset(&child_msg,   0, sizeof(child_msg));
         char cmsgbuf[CMSG_SPACE(sizeof(int))];
         // make place for the ancillary message to be received
         child_msg.msg_control = cmsgbuf;
         child_msg.msg_controllen = sizeof(cmsgbuf);

         printf("Waiting on recvmsg\n");
         rc = recvmsg(sockfd, &child_msg, 0);
         printf("recvmsg return:%d\n", rc);

         struct cmsghdr *cmsg = CMSG_FIRSTHDR(&child_msg);
         printf("Cmsg is %p Msg_type:%d.\n", cmsg, cmsg?cmsg->cmsg_type:0);
         if (cmsg == NULL || cmsg -> cmsg_type != SCM_RIGHTS) {
            printf("First control structure contains no file descriptor.\n");
         }
         else
         {
            int pass_sd;
            memcpy(&pass_sd, CMSG_DATA(cmsg), sizeof(pass_sd));
            printf("Received descriptor = %d\n", pass_sd);
         }
      }
#endif
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      printf("Received the following:\n");
      printf("%s",mesg);
      printf("-------------------------------------------------------\n");
   }
}
