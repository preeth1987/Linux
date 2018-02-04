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
#include "vrrpd.h"
#include "csum.h"

#define VRRP_ADVT_FRAME_SZ 264
struct vrrp_advt_info
{
  /* Version & Type. */
  uint8_t ver_type;

  /* VrId. */
  uint8_t vrid;

  /* Priority. */
  uint8_t priority;

  /* Number of IP addresses to backup. */
  uint8_t num_ip_addrs;
  union
  {
    struct {
        /* Auth type. */
        u_int8_t auth_type;
        /* Advertisement interval. */
        u_int8_t advt_int; /* rsvd + Adver Int */
    };
    /* Advertisement interval. */
    uint16_t max_advt_int;
  };

  /* VRRP Checksum. */
  u_int16_t cksum;
  union
  {
	  struct in_addr vip_v4[0];
	  struct in6_addr vip_v6[0];
  };
};

typedef struct vrrp_advt_buf {
    int                  sock;
    union {
        struct in_addr  addr;
        struct sockaddr_in sin;
        struct in6_addr  addr_ipv6;
        struct sockaddr_in6 sin_ipv6;
    } u;
    union {
        struct ip iph;
        struct ip6_hdr ip6h;
    };
    uint32_t             ifindex;
    int                  size;
    uint8_t              is_vrrpe;
    uint8_t              af_type ;
    char                 data[VRRP_ADVT_FRAME_SZ];
    int                  vrf_id;
} vrrp_advt_buf_t;

#define PKTLEN 2048

int
vrrpAdvert(char *buff)
{
	struct vrrp_advt_info *vi = NULL;
	int size = 0;

	size = sizeof(struct vrrp_advt_info) + sizeof(struct in_addr);
	size += (2*sizeof(unsigned int));

	vi = (struct vrrp_advt_info *) buff;
	vi->ver_type = 0x21;
	vi->vrid = 1;
	vi->priority = 100;
	vi->num_ip_addrs = 1;
	vi->vip_v4[0].s_addr=htonl(0x380d0138);
	vi->auth_type = 0x0;
	vi->advt_int = 1;
	vi->cksum = htons(0xba52);

	return size;
}

int sndExtra(int fd, void* data, uint32_t len, int ifindex)
{
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct iovec iov;
	int ret=0;
	int alen=0;
	char *adata=NULL;
	struct in_pktinfo pktinfo, *pkt;
	struct sockaddr_in  sin;

	memset (&msg, 0, sizeof (struct msghdr));
	alen = CMSG_SPACE (sizeof (struct in_pktinfo));
	adata = calloc(alen, sizeof(char));
	msg.msg_control = adata;
	msg.msg_controllen = alen;

	iov.iov_base = (void*)data;
	iov.iov_len = len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	struct in_addr src_in;
	struct in_addr ip_dst;

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN (sizeof (struct in_pktinfo));
	//cmsg->cmsg_level = IPPROTO_RAW;
	cmsg->cmsg_level = SOL_IP;
	cmsg->cmsg_type = IP_PKTINFO;
	pkt = (struct in_pktinfo*)CMSG_DATA (cmsg);


	memset(&sin, 0, sizeof (struct sockaddr_in));
	//ip_dst.s_addr = (unsigned long) htonl(0xe0000012);
	ip_dst.s_addr = (unsigned long) htonl(0x380d0138);

	//pkt->ipi_spec_dst = src_in; 
	pkt->ipi_spec_dst = ip_dst; 
	pkt->ipi_ifindex = ifindex;

	//sin.sin_addr = ip_dst;
	sin.sin_family = AF_INET;
	//src_in.s_addr = htonl(0x380d0138);
	src_in.s_addr = htonl(0xe0000012);
	sin.sin_addr = src_in;

	 msg.msg_name = &sin;
	 msg.msg_namelen = sizeof (struct sockaddr_in);


	#if 0
	memset(&pktinfo, 0, sizeof(pktinfo));
	memset(&sin, 0, sizeof(sin));

	/*pktinfo.ipi_spec_dst.s_addr = htonl(0x380d0138);
	pktinfo.ipi_ifindex = ifindex;
	alen = CMSG_SPACE (sizeof (struct in_pktinfo));
	adata = (char*)&pktinfo;
	msg.msg_control = adata;
	msg.msg_controllen = alen;*/

	/*iov[0].iov_base = (void*)&iph;
    iov[0].iov_len  = sizeof(struct ip);*/
	iov.iov_base = (void*)data;
	iov.iov_len = len;

    msg.msg_iov        = &iov;
    msg.msg_iovlen     = 1;

	sin.sin_addr.s_addr = htonl(0xe0000012);
	sin.sin_family = AF_INET;

	msg.msg_name = &sin;
	msg.msg_namelen = sizeof(sin);
	#endif
	printf("Sending %d bytes of data\n", len);
	ret = sendmsg(fd, &msg, MSG_DONTROUTE);
	if (ret < 0) {
		printf("sendmsg err: %s\n", strerror(errno));
	} else {
		printf("sent %d bytes message success\n", ret);
	}
	free(adata);
	return ret;
	
}

int main(int argc, char* argv[])
{
	int sfd, len, ret;

	char buffer[PKTLEN+1];
	struct sockaddr_in sin;
	struct sockaddr_storage inc_packet;
	socklen_t inc_packetl;
	struct ip_mreqn mr;
	struct in_addr group;
	struct ifreq ifr;
	uint32_t ttl = 255;

	memset(buffer,0,PKTLEN);

	if (argc != 2) {
		printf("%%Error Usage: %s <interface>\n", argv[0]);
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

	sfd=socket(AF_INET,SOCK_RAW,112);
	if(sfd<0){
		perror("socket() error\n");
		exit(1);
	}


	bzero(&sin, sizeof(sin));
	bzero(&inc_packet, sizeof(inc_packet));

	sin.sin_family = AF_INET;
	//sin.sin_port = htons(atoi(argv[2]));
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	/*if (inet_aton(argv[1], &sin.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}*/
	
	/*if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin))<0){
		perror("bind() error\n");
		exit(1);
	}*/

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

	ret = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof(mr));
	if (ret<0)
	{
		printf("sock option IP_ADD_MEMBERSHIP failed: %s\n", strerror(errno));
		return -1;
	}

	ret = setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	if (ret<0) {
		printf("sock option IP_MULTICAST_TTL failed: %s\n", strerror(errno));
		return -1;
	}

	int x =0;
	while(x!=10) {
		len = vrrpAdvert(buffer); 
		sndExtra(sfd, (void*)buffer, len, ifr.ifr_ifindex);
		sleep(1);
		x++;
	}

	return 0;
}
