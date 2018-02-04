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
#include "ipudp.h"
#include "csum.h"

#define MAX_LEN 512

int main(int argc, char* argv[])
{
    int sfd, ret, opt=1;
    int x =0, slen=0;
    int bytes_read;
    struct ip_mreqn mr;
    struct in_addr group;
    char rbuf[MAX_LEN];
    struct data {
        struct cmsghdr  cmh;
        struct in_pktinfo   pktinfo;
    } cmsg;
    struct msghdr msg;
    struct sockaddr_in self, other;
    socklen_t optlen;
    uint16_t    ifindex = 0;
    struct ifreq ifr;

	if (argc!=2) {
		printf("Usage: %s interface\n", argv[0]);
		exit(1);
	}
		if ((sfd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
			perror ("socket() failed to get socket descriptor for using ioctl() ");
			exit (1);
		}

		memset (&ifr, 0, sizeof (ifr));
		snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", argv[1]);
		if (ioctl (sfd, SIOCGIFINDEX, &ifr) < 0) {
			perror ("ioctl() failed to find interface ");
			return (1);
		}
		close (sfd);
		printf ("Index for interface %s is %i\n", argv[1], ifr.ifr_ifindex);

	sfd=socket(AF_INET,SOCK_RAW,89/*IPPROTO_OSPF*/);
	if(sfd<0){
		perror("socket() error\n");
		exit(1);
	}

	ret=setsockopt(sfd, SOL_IP, IP_PKTINFO, &opt, sizeof(opt));
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
		if (setsockopt (sfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) {
			perror ("setsockopt() failed to bind to interface ");
			exit (1);
		}
	}

#if 0
	group.s_addr = htonl(0xe0000005);
	mr.imr_multiaddr = group; 
	if (strcmp(argv[1], "all")) {
		mr.imr_ifindex = ifr.ifr_ifindex;
	} else {
		mr.imr_ifindex = 0;
	}
	//mr.imr_address.s_addr = htons(atoi(argv[1]));

	ret = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof(mr));
	if (ret<0)
	{
		printf("sock option IP_ADD_MEMBERSHIP failed: %s\n", strerror(errno));
		return -1;
	}

	group.s_addr = htonl(0xe0000006);
	mr.imr_multiaddr = group; 
	if (strcmp(argv[1], "all")) {
		mr.imr_ifindex = ifr.ifr_ifindex;
	} else {
		mr.imr_ifindex = 0;
	}
	//mr.imr_address.s_addr = htons(atoi(argv[1]));

	ret = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof(mr));
	if (ret<0)
	{
		printf("sock option IP_ADD_MEMBERSHIP failed: %s\n", strerror(errno));
		return -1;
	}
#endif

	group.s_addr = htonl(0xe0000002);
	mr.imr_multiaddr = group; 
	if (strcmp(argv[1], "all")) {
		mr.imr_ifindex = ifr.ifr_ifindex;
	} else {
		mr.imr_ifindex = 0;
	}
	//mr.imr_address.s_addr = htons(atoi(argv[1]));

	ret = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof(mr));
	if (ret<0)
	{
		printf("sock option IP_ADD_MEMBERSHIP failed: %s\n", strerror(errno));
		return -1;
	}
	while (1) {
		sleep (1);
	}
	return 0;
}
