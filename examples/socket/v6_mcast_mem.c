#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <net/if.h>


int main(int argc, char* argv[])
{
    int sfd, ret, opt=1;
    int x =0, slen=0;
    int bytes_read;
    struct ipv6_mreq mr;
    struct in6_addr group;
    struct sockaddr_in self, other;
    socklen_t optlen;
    uint16_t    ifindex = 0;
    struct ifreq ifr;

	if (argc!=2) {
		printf("Usage: %s interface\n", argv[0]);
		exit(1);
	}
		if ((sfd = socket (AF_INET6, SOCK_RAW, IPPROTO_RAW)) < 0) {
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

	sfd=socket(AF_INET6,SOCK_RAW,89/*IPPROTO_VRRP*/);
	if(sfd<0){
		perror("socket() error\n");
		exit(1);
	}

	/*ret=setsockopt(sfd, SOL_IPV6, IPV6_PKTINFO, &opt, sizeof(opt));
	if(ret<0)
	{
		printf("socket option IPV6_PKTINFO set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}*/

	if (strcmp(argv[1], "all")) {
		if (setsockopt (sfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) {
			perror ("setsockopt() failed to bind to interface ");
			exit (1);
		}
	}

	inet_pton(AF_INET6, "ff02::5", &group);
	mr.ipv6mr_multiaddr = group; 
	if (strcmp(argv[1], "all")) {
		//mr.ipv6mr_ifindex = ifr.ifr_ifindex;
		mr.ipv6mr_interface = ifr.ifr_ifindex;
	} else {
		//mr.ipv6mr_ifindex = 0;
		mr.ipv6mr_interface = 0;
	}
	//mr.imr_address.s_addr = htons(atoi(argv[1]));

	ret = setsockopt(sfd, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mr, sizeof(mr));
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
