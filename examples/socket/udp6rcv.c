#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/socket.h>
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
#define MAX_LEN	512

struct in6_pktinfo {
	struct in6_addr ipi6_addr;
	int     ipi6_ifindex;
};

int rcvExtra(int fd,char *rbuf, int len);

const char *inet6_ntoa(struct in6_addr addr)
{
	static char buf[INET6_ADDRSTRLEN];

	inet_ntop(AF_INET6, &addr, buf, INET6_ADDRSTRLEN);
	return buf;
}

int main( int argc, char **argv)
{
#define UDP_PORT 547
#define DEV "mgmt"
#define MULTIADDR "FF02::1:2"
	int ret ;
	int sd, i ;
	int netall = 1;
	struct sockaddr_in6 self, other;

	char rbuf[MAX_LEN];
	int slen = 0;
	int mlen;
	int opt = 1;

	struct data {
		struct cmsghdr	cmh;
		struct in6_pktinfo	pktinfo;
	} cmsg;

	struct msghdr msg;
	socklen_t optlen;
#define BUFFERSIZE 1000
	char buffer[BUFFERSIZE];

	sd = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if(sd < 0 ) {
		perror("socket : " );
		return -1;
	}

	ret = setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, DEV, strlen(DEV));
	if (ret < 0)
	{
		printf("socket bind to dev %s failed %d %s\n", DEV, ret, strerror(errno));
		return -1;
	}


    struct ipv6_mreq mreq;
    inet_pton(AF_INET6, MULTIADDR, &mreq.ipv6mr_multiaddr);

	mreq.ipv6mr_interface = if_nametoindex("eth0");
    ret = setsockopt(sd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
			&mreq, sizeof(mreq));
    if (ret < 0)
	{
		printf("socket multicast %s join if:%s failed %d %s", MULTIADDR, DEV, ret, strerror(errno));
        return -1;
	}

	ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0)
	{
		printf("socket option SO_REUSEADDR set failed %d %s\n", ret, strerror(errno));
		return -1;
	}

	ret = setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	if (ret < 0)
	{
		printf("socket option SO_REUSEPORT set failed %d %s\n", ret, strerror(errno));
		return -1;
	}

	ret=setsockopt(sd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &opt, sizeof(opt));
	if(ret<0)
	{
		printf("socket option IP_PKTINFO set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}

	ret=setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof (opt));
	if(ret<0)
	{
		printf("socket option SO_BROADCAST set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}

	int sock_size = (1024 * 1024);
	ret = setsockopt(sd, SOL_SOCKET, SO_RCVBUFFORCE, &sock_size, sizeof(sock_size));
	if(ret<0)
	{
		printf("socket option SO_RCVBUFFORCE set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}

	sock_size = (512 * 1024);
	ret = setsockopt(sd, SOL_SOCKET, SO_SNDBUFFORCE, &sock_size, sizeof(sock_size));
	if(ret<0)
	{
		printf("socket option SO_SNDBUFFORCE set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}
	printf("sock options set\n");

	bzero(&self, sizeof(self));
	bzero(&other, sizeof(other));

	self.sin6_family = AF_INET6;
	self.sin6_port	= htons(UDP_PORT);
	struct in6_addr addr = {0};
	self.sin6_addr = addr; 

	ret = bind(sd, (struct sockaddr *) &self, sizeof(self));
	if(ret < 0 ) 
	{
		perror("bind : \n" );
		close(sd);
		return -1;
	}
	printf("sock bind done\n");

	do {
		ret = recvfrom(sd, rbuf, sizeof(struct ip6_hdr), MSG_PEEK, (struct sockaddr *)&other, (socklen_t  *) &slen);
		if(ret < 0 ) {
			perror("recvfrom : \n" );
			close(sd);
			return -1;
		}
		printf("received packet of size:%d from %s:%d\nData: %s", 
				slen, inet6_ntoa(other.sin6_addr), ntohs(other.sin6_port), rbuf);

		rcvExtra(sd, rbuf, slen);

		printf("\n");
	} while(1);
	close(sd);

	return 1;
}

	void *
cmsgLookup(struct msghdr *msg)
{
	struct cmsghdr *cmsg;

	for (cmsg = CMSG_FIRSTHDR (msg); cmsg; cmsg = CMSG_NXTHDR (msg, cmsg))
        if ((cmsg->cmsg_level == IPPROTO_IPV6) && 
				(cmsg->cmsg_type == IPV6_PKTINFO))
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
	struct ip6_hdr iph;
	int alen=0;
	char *adata=NULL;
	struct in6_pktinfo pktinfo, *pkt;

	memset(&iph, 0, sizeof(iph));
	memset(&data, 0, sizeof(char)*MAX_LEN);
	memset(&pktinfo, 0, sizeof(pktinfo));
	memset (&msg, 0, sizeof (struct msghdr));

	alen = CMSG_SPACE (sizeof (struct in6_pktinfo));
	adata = (char*)&pktinfo;
	msg.msg_control = adata;
	msg.msg_controllen = alen;

	iov[0].iov_base = (void*)&iph;
	iov[0].iov_len  = sizeof(struct ip6_hdr);
	iov[1].iov_base = (void*)data;
	iov[1].iov_len = MAX_LEN;

	msg.msg_iov        = iov;
	msg.msg_iovlen     = 2;

	ret = recvmsg(fd, &msg, 0);
	if (ret < 0) {
		printf("recvmsg err: %s\n", strerror(errno));
	} else {
		printf("recvmsg iov\n");
		pkt = (struct in6_pktinfo*)cmsgLookup(&msg);
		if (pkt) {
			printf("ifindex: %x ipi_addr:%s\n", pkt->ipi6_ifindex,
					inet6_ntoa(pkt->ipi6_addr));
		}
	}
}
