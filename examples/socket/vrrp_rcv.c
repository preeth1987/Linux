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

#define MAX_LEN	512

int rcvExtra(int fd,char *rbuf, int len);

int main(int argc, char* argv[])
{
	int sfd, ret, opt=1;
	int x =0, slen=0;
	int bytes_read;
	struct ip_mreqn mr;
	struct in_addr group;
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

	sfd=socket(AF_INET,SOCK_RAW,112/*IPPROTO_VRRP*/);
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
	}

	group.s_addr = htonl(0xe0000012);
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

	do {
		ret = recvfrom(sfd, rbuf, sizeof(struct ip), MSG_PEEK, (struct sockaddr *)&other, (socklen_t  *) &slen);
		if(ret < 0 ) {
			perror("recvfrom : \n" );
			close(sfd);
			return -1;
		}
		printf("received packet of size:%d from %s:%d\nData: %s", 
				slen, inet_ntoa(other.sin_addr), ntohs(other.sin_port), rbuf);

		rcvExtra(sfd, rbuf, slen);
		/*memset(&cmsg, 0, sizeof(cmsg));
		  msg.msg_control = &cmsg;
		  msg.msg_controllen = sizeof(cmsg);

		  ret = recvmsg(sd, &msg, 0);
		  if (ret < 0) {
		  perror("recvmsg: len less than 0\n");
		  return -1;
		  }*/

		printf("\n");
	} while(1);
	close(sfd);

	return 1;
}

	void *
cmsgLookup(struct msghdr *msg)
{
	struct cmsghdr *cmsg;

	for (cmsg = CMSG_FIRSTHDR (msg); cmsg; cmsg = CMSG_NXTHDR (msg, cmsg))
      return CMSG_DATA (cmsg);
	
	return NULL;
}

int rcvExtra(int fd,char *rbuf, int len)
{
	struct sockaddr_in  server;
    int optlen, smsg, byterecv,rv,i;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct iovec iov[3];
	char data[MAX_LEN+1];
	int ret=0;
	struct ip iph;
	int alen=0;
	char *adata=NULL;
	struct in_pktinfo pktinfo, *pkt;

	memset(&iph, 0, sizeof(iph));
	memset(&data, 0, sizeof(char)*MAX_LEN);
	memset(&pktinfo, 0, sizeof(pktinfo));
	memset (&msg, 0, sizeof (struct msghdr));

	alen = CMSG_SPACE (sizeof (struct in_pktinfo));
	adata = (char*)&pktinfo;
	msg.msg_control = adata;
	msg.msg_controllen = alen;

	iov[0].iov_base = (void*)&iph;
    iov[0].iov_len  = sizeof(struct ip);
	iov[1].iov_base = (void*)data;
	iov[1].iov_len = MAX_LEN;

    msg.msg_iov        = iov;
    msg.msg_iovlen     = 2;

	ret = recvmsg(fd, &msg, 0);
	if (ret < 0) {
		printf("recvmsg err: %s\n", strerror(errno));
	} else {
		printf("recvmsg iov\n");
		pkt = (struct in_pktinfo*)cmsgLookup(&msg);
		if (pkt) {
			printf("ifindex: %x ipi_spec_dst: %x ipi_addr:%x\n", pkt->ipi_ifindex,
					pkt->ipi_spec_dst.s_addr, pkt->ipi_addr.s_addr);
		}
	}
}
