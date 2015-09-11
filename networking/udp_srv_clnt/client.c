/* Sample UDP client */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *SERVER_IP = "127.0.0.1";

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr;
   char sendline[1000];
   char recvline[1000];
   const int on = 1;

   if (argv[1])
       SERVER_IP = argv[1];

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(SERVER_IP);
   servaddr.sin_port=htons(32000);

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_TTL,(char *)&on, sizeof on)) {
       perror("sesockopt IP_PKTINFO error");
   }

   while (fgets(sendline, 10000, stdin) != NULL)
   {
#if 1
      {
         struct msghdr parent_msg;
         struct iovec iov[1];
         char buf[1024];
         int rc;

         memset(&parent_msg, 0, sizeof(parent_msg));
         parent_msg.msg_name       = &servaddr;
         parent_msg.msg_namelen    = sizeof (struct sockaddr_in);

         parent_msg.msg_iov        = iov;
         parent_msg.msg_iovlen     = 1;
         iov[0].iov_base = buf; /* data we receive from the peer */
         iov[0].iov_len = sizeof buf;

         strcpy(buf, sendline);
         iov[0].iov_len = strlen(buf);

         struct cmsghdr *cmsg;
         char cmsgbuf[CMSG_SPACE(1024)];
         uint8_t control_data = 8;

         parent_msg.msg_control    = cmsgbuf;
         parent_msg.msg_controllen = sizeof(cmsgbuf);

         cmsg                      = CMSG_FIRSTHDR(&parent_msg);
         cmsg->cmsg_level          = SOL_SOCKET;
         cmsg->cmsg_type           = IP_PKTINFO;
         cmsg->cmsg_len            = CMSG_LEN(sizeof(sockfd));
         memcpy(CMSG_DATA(cmsg), &control_data, sizeof(control_data));

         parent_msg.msg_controllen = cmsg->cmsg_len; // total size of all control blocks

         if((sendmsg(sockfd, &parent_msg, 0)) < 0)
         {
            perror("sendmsg()");
            exit(0);
         }
         printf("Message Sent\n");
      }
#else
      int opt = 1;
      sendto(sockfd,sendline,strlen(sendline),0,
             (struct sockaddr *)&servaddr,sizeof(servaddr));
      n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
      recvline[n]=0;
      fputs(recvline,stdout);
#endif
   }
}
