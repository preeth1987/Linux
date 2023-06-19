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
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

struct iplink_req {
    struct nlmsghdr     n;
    struct ifinfomsg    i;
    char            buf[1024];
};

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
          int alen)
{
    int len = RTA_LENGTH(alen);
    struct rtattr *rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
        fprintf(stderr, "addattr_l ERROR: message exceeded bound of %d\n",maxlen);
        return -1;
    }
    rta = NLMSG_TAIL(n);
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy(RTA_DATA(rta), data, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
    return 0;
}


int main (int argc, char *argv[])
{
	int status;	
	int sfd, len, ret;
	struct ifreq ifr;
	struct nlmsghdr *nl_msg;
    int msg_seq = 0;

	struct iplink_req req = {
        .n.nlmsg_len = 40,
        .n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK,
        .n.nlmsg_type = RTM_NEWLINK,
		.n.nlmsg_seq = time(NULL),
		.n.nlmsg_pid = getpid(),
        .i.ifi_family = AF_UNSPEC,
    };

	struct iovec iov = {
        .iov_base = (void*) &req.n,
        .iov_len = req.n.nlmsg_len
    };

	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	char   buf[32768] = {};


	if (argc != 3) {
		printf ("Usage: %s <dev> <alias>\n", argv[0]);
		exit(1);
	}


	if ((sfd = socket (AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE)) < 0) {

		perror ("socket() failed to get socket descriptor for using ioctl() ");
        exit (1);	
	}
	
	int sndbuf = 32768;
	int rcvbuf = 1048576;
	setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
	setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

	struct sockaddr_nl addr = {
		.nl_family=AF_NETLINK,
	};
	
	bind(sfd,(struct sockaddr *)&addr, sizeof(addr));

	socklen_t addr_len;
	addr_len = sizeof(struct sockaddr_nl);
	addr.nl_pid = getpid();
	getsockname(sfd, (struct sockaddr*)&addr, &addr_len);

	int ifindex = if_nametoindex(argv[1]);
	printf("IFI of dev %s : %d\n", argv[1], ifindex);

    req.i.ifi_index = ifindex;

    addattr_l(&req.n, sizeof(req), IFLA_IFALIAS,
            argv[2], strlen(argv[2]));

	status = sendmsg(sfd, &msg, 0);
	if (status < 0) {
		perror("rtnl talk failed");
		return -1;
	}

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	status = recvmsg(sfd, &msg, 0);

    close(sfd);
	printf("Success\n");
	return 0;
}
