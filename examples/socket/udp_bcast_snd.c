#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <string.h>
#include <netinet/ip6.h>
#include <errno.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#define PORT1 67
#define PORT2 68
#define DEST_ADDR "255.255.255.255"
#define RECV_TIMEOUT_MS             100000
#define      IPHELP_RCV_BUF_SIZE               (1024 * 1024)
 
int main(int argc, char *argv[])
{
	int sockfd;
	int broadcast=1;
	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;
	int numbytes, state=1;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = RECV_TIMEOUT_MS;
	int i=1, ret = 0;
	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *cmptr = 0;
	struct sockaddr_in sa;
	union {
		struct cmsghdr align; /* this ensures alignment */
		char control[CMSG_SPACE(sizeof(struct in_pktinfo))];

	} control_u;
	struct in_pktinfo pktinfo;
	char vrf_mdev[64] = "vrf-1";
	int vrf_ifindex;
	struct cmsghdr *cmsg = NULL;
	
	if((sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1)
	{
		perror("sockfd");
		exit(1);
	}

	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
				sizeof(timeout)) < 0) {
		perror("SO_RCVTIMEO failed");
		close(sockfd);
		return -1;
	}

	if (setsockopt (sockfd, IPPROTO_IP, IP_PKTINFO, &state, sizeof (state)) < 0 ) {
		perror("IP_PKTINFO failed");
		close(sockfd);
		return -1;
	}

	if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, &state, sizeof (state))) {
		perror("SO_BROADCAST failed");
		close(sockfd);
		return -1;
	}
	
   /*if((setsockopt(sockfd,SOL_SOCKET,SO_DONTROUTE,
   			&broadcast,sizeof broadcast)) == -1)
   {
      perror("setsockopt - SO_DONTROUTE");
      exit(1);
   }*/
        	
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = PORT1;
	sendaddr.sin_addr.s_addr = INADDR_ANY;
	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);
	
	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");
		exit(1);
	}

	/* Sendto address */
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT2);
	sa.sin_addr.s_addr = htonl(0xffffffff);
	msg.msg_controllen = sizeof(control_u);
	msg.msg_control = control_u.control;
	msg.msg_namelen = sizeof(sa);
	msg.msg_name = &sa;

	/* Prepare the buffer */
	char buf[100] = "abcd\0";
	struct in_pktinfo *pkt1;
	iov.iov_base = (void *)&buf;
	iov.iov_len = 5;

	/* Prepare the message header */
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	memset(&control_u, 0, sizeof(control_u));

	bzero(&pktinfo, sizeof(struct in_pktinfo));

	vrf_ifindex = if_nametoindex(vrf_mdev);
	cmsg = CMSG_FIRSTHDR(&msg);

	cmsg->cmsg_len = CMSG_LEN (sizeof(pktinfo));
	cmsg->cmsg_level = IPPROTO_IP;
	cmsg->cmsg_type = IP_PKTINFO;

	//pktinfo.ipi_ifindex = vrf_ifindex;
	pkt1 = (struct in_pktinfo *)CMSG_DATA (cmsg);
	pkt1->ipi_spec_dst.s_addr = htonl(0x0b010101);
	pktinfo.ipi_ifindex = 1207959653;
	printf("Sending on if: %x\n", pktinfo.ipi_ifindex);
	*(struct in_pktinfo *)CMSG_DATA(cmsg) = pktinfo;

	while ( i<=10 ) {
		i++;
		ret = sendmsg(sockfd, &msg, 0);
		if ( ret < 0 ) {
			printf("Error: sendmsg failed errno: %d\n", errno);
		}
	}
	
	close(sockfd);
	
	return 0;
}
