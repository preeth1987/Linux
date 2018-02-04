#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_LEN	512

int rcvExtra(int fd,char *rbuf, int len);

int main( int argc, char **argv)
{
	int ret ;
	int sd, i ;
	int netall = 1;
	struct sockaddr_in self, other;

	char rbuf[MAX_LEN];
	int slen = 0;
	int mlen;
	int opt = 1;

	struct data {
		struct cmsghdr	cmh;
		struct in_pktinfo	pktinfo;
	} cmsg;

	struct msghdr msg;
	char dev1[] = "green";
	char dev2[] = "blue";
	socklen_t optlen;
#define BUFFERSIZE 1000
	char buffer[BUFFERSIZE];

	if(argc != 3 )
	{
		printf("udprcv <vrf> <port> \n" );
		printf("ex:udprcv all 12334\n" );
		return 1;
	}

	//sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0 ) {
		perror("socket : " );
		return -1;
	}

	if (strcmp(argv[1], "all") != 0) {
		ret = setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, argv[1], strlen(argv[1]));
	}

	bzero(&self, sizeof(self));
	bzero(&other, sizeof(other));

	self.sin_family = AF_INET ;
	self.sin_port	= htons(atoi(argv[2]));
	self.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(sd, (struct sockaddr *) &self, sizeof(self));
	if(ret < 0 ) 
	{
		perror("bind : \n" );
		close(sd);
		return -1;
	}

	ret=setsockopt(sd, SOL_IP, IP_PKTINFO, &opt, sizeof(opt));
	if(ret<0)
	{
		printf("socket option IP_PKTINFO set failed, %d : %s\n", ret, strerror(errno));
		return -1;
	}

	do {
	ret = recvfrom(sd, rbuf, sizeof(struct ip), MSG_PEEK, (struct sockaddr *)&other, (socklen_t  *) &slen);
	if(ret < 0 ) {
		perror("recvfrom : \n" );
		close(sd);
		return -1;
	}
	printf("received packet of size:%d from %s:%d\nData: %s", 
			slen, inet_ntoa(other.sin_addr), ntohs(other.sin_port), rbuf);
	
	rcvExtra(sd, rbuf, slen);
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
	close(sd);
	
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
