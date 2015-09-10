/* Sample UDP client */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include    <sys/param.h> 

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr;
   char sendline[1000];
   char recvline[1000];

   if (argc != 2)
   {
      printf("usage:  udpcli <IP address>\n");
      exit(1);
   }

   sockfd=socket(AF_UNIX,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_UNIX;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(32000);

   while (fgets(sendline, 10000,stdin) != NULL)
   {
#if 1
      {
         struct msghdr parent_msg;
         size_t length;

         memset(&parent_msg, 0, sizeof(parent_msg));
         struct cmsghdr *cmsg;
         char cmsgbuf[CMSG_SPACE(sizeof(sockfd))];
         parent_msg.msg_name       = &servaddr;
         parent_msg.msg_namelen    = sizeof (struct sockaddr_in);
         parent_msg.msg_control    = cmsgbuf;
         // necessary for CMSG_FIRSTHDR to return the correct value
         parent_msg.msg_controllen = sizeof(cmsgbuf);

         cmsg             = CMSG_FIRSTHDR(&parent_msg);
         cmsg->cmsg_level = SOL_SOCKET;
         cmsg->cmsg_type  = SCM_RIGHTS;
         cmsg->cmsg_len   = CMSG_LEN(sizeof(sockfd));
         memcpy(CMSG_DATA(cmsg), &sockfd, sizeof(sockfd));

         setsockopt(sockfd, IPPROTO_IP, IP_RECVIF, &on, sizeof(on)) < 0;
         parent_msg.msg_controllen = cmsg->cmsg_len; // total size of all control blocks

         if((sendmsg(sockfd, &parent_msg, 0)) < 0)
         {
            perror("sendmsg()");
            exit(0);
         }
         printf("Message Sent\n");
      }
#endif
      sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
      n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
      recvline[n]=0;
      fputs(recvline,stdout);
   }
}
