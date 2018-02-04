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
#include <errno.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
//#include "ipudp.h"
//#include "csum.h"

#define MAX_LEN	512

int rcvExtra(int fd,char *rbuf, int len);

int main(int argc, char* argv[])
{
	int sfd, ret, opt=1;
	int x =0, slen=0;
	int bytes_read;
	struct ipv6_mreq mr;
	struct in6_addr group;
	char rbuf[MAX_LEN];
	struct data {
		struct cmsghdr	cmh;
		struct in_pktinfo	pktinfo;
	} cmsg;
	struct msghdr msg;
	struct sockaddr_in self, other;
	socklen_t optlen;
	uint16_t	ifindex = 0;
	struct ifreq ifr;

	memset(&group, 0, sizeof(group));

	if (argc!=2) {
		printf("Usage: %s interface\n", argv[0]);
		exit(1);
	}

	if (strcmp(argv[1], "all")) {
		// Submit request for a socket descriptor to look up interface.
		if ((sfd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
			perror ("socket() failed to get socket descriptor for using ioctl() ");
			exit (1);
		}

		// Use ioctl() to look up interface index which we will use to
		// bind socket descriptor sd to specified interface with setsockopt() since
		// none of the other arguments of sendto() specify which interface to use.
		memset (&ifr, 0, sizeof (ifr));
		snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", argv[1]);
		if (ioctl (sfd, SIOCGIFINDEX, &ifr) < 0) {
			perror ("ioctl() failed to find interface ");
			return (1);
		}
		close (sfd);
		printf ("Index for interface %s is %i\n", argv[1], ifr.ifr_ifindex);
	}

	memset(&mr, 0, sizeof(mr));

	sfd=socket(AF_INET6,SOCK_RAW,89/*IPPROTO_VRRP*/);
	if(sfd<0){
		perror("socket() error\n");
		exit(1);
	}

	//struct in6_pktinfo pkt;
	//memset(&pkt, 0, sizeof(pkt));
	//ret=setsockopt(sfd, IPPROTO_IPV6, IPV6_PKTINFO, &pkt, sizeof(pkt));
	ret=setsockopt(sfd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &opt, sizeof(opt));
	if(ret<0)
	{
		printf("socket option IP_PKTINFO set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}

	if (strcmp(argv[1], "all")) {
		if (setsockopt (sfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) {
			perror ("setsockopt() failed to bind to interface ");
			exit (1);
		}
	}

	inet_pton(AF_INET6, "ff02::5", &group);
	mr.ipv6mr_multiaddr = group; 
	if (strcmp(argv[1], "all")) {
		mr.ipv6mr_interface = ifr.ifr_ifindex;
	} else {
		mr.ipv6mr_interface = 0;
	}
	//mr.imr_address.s_addr = htons(atoi(argv[1]));

	ret = setsockopt(sfd, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mr, sizeof(mr));
	if (ret<0)
	{
		printf("sock option IP_ADD_MEMBERSHIP failed: %s\n", strerror(errno));
		return -1;
	}

	do {
		ret = recvfrom(sfd, rbuf, sizeof(struct ip), MSG_PEEK, (struct sockaddr *)&other, (socklen_t  *) &slen);
		if(ret < 0 ) {
			perror("recvfrom : \n" );
			close(sfd);
			return -1;
		}
		printf("received packet of size:%d from %s:%d\nData: %s", 
				slen, inet_ntoa(other.sin_addr), ntohs(other.sin_port), rbuf);

		//rcvExtra(sfd, rbuf, slen);

		printf("\n");
	} while(1);
	close(sfd);

	return 1;
}
