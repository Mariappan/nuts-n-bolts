
/* Sample UDP server */

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char**argv)
{
   int sockfd;
   struct sockaddr_in servaddr;

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   int on = 1;
   if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&on, sizeof on)) {
       perror("sesockopt SO_REUSEADDR error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_TOS,(char *)&on, sizeof on)) {
       perror("sesockopt IP_TOS error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_TTL,(char *)&on, sizeof on)) {
       perror("sesockopt IP_TTL error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_OPTIONS,(char *)&on, sizeof on)) {
       perror("sesockopt IP_OPTIONS error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_PKTINFO,(char *)&on, sizeof on)) {
       perror("sesockopt IP_PKTINFO error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_PKTOPTIONS,(char *)&on, sizeof on)) {
       perror("sesockopt IP_PKTOPTIONS error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_RECVTOS,(char *)&on, sizeof on)) {
       perror("sesockopt IP_RECVTOS error");
   }

   if (-1 == setsockopt(sockfd, IPPROTO_IP, IP_MTU,(char *)&on, sizeof on)) {
       perror("sesockopt IP_MTU error");
   }

   for (;;)
   {
#if 1
      {
         struct msghdr child_msg;
         struct iovec iov[1] = {0};
         char buf[1024];
         int recvbytes;

         memset(&child_msg,   0, sizeof(child_msg));

         child_msg.msg_iov        = iov;
         child_msg.msg_iovlen     = 1;
         iov[0].iov_base          = buf;
         iov[0].iov_len           = sizeof buf;

         char cmsgbuf[CMSG_SPACE(1024)];

         child_msg.msg_control    = cmsgbuf;
         child_msg.msg_controllen = sizeof(cmsgbuf);

         // printf("\n\n********************\n");
         printf("Waiting for recvmsg\n");
         recvbytes = recvmsg(sockfd, &child_msg, 0);
         printf("recvmsg return:%d %s\n", recvbytes, buf);

         if (recvbytes >= 0)
         {
            struct cmsghdr *cmsg     = CMSG_FIRSTHDR(&child_msg);
            if (cmsg == NULL) {
               printf("No control structure found.\n");
            }
            else
            {
               /* Receive auxiliary data in msgh */
               for (; cmsg != NULL; cmsg = CMSG_NXTHDR(&child_msg, cmsg)) {
                   uint8_t recvd_control_data;
                   printf("Cmsg Msg_type:%d Level is %d.\n", cmsg->cmsg_type, cmsg->cmsg_level);
                   memcpy(&recvd_control_data, CMSG_DATA(cmsg), sizeof(recvd_control_data));
                   printf("Received data = %u %u\n", recvd_control_data, htons(recvd_control_data));

                   if (cmsg->cmsg_level == IPPROTO_IP
                       && cmsg->cmsg_type == IP_PKTINFO) {
                       struct in_pktinfo *pkt_info = CMSG_DATA(cmsg);
                       printf("PKTINFO: ifindex:%u\n", pkt_info->ipi_ifindex);
                   }
               }
            }
         }
      }
#else
      struct sockaddr_in cliaddr;
      char mesg[1024];
      socklen_t len;
      int n;
      printf("Waiting on recvfrom\n");
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      printf("-------------------------------------------------------\n");
      mesg[n] = 0;
      printf("Received the following:\n");
      printf("%s",mesg);
      printf("-------------------------------------------------------\n");
#endif
   }
}
