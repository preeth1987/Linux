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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_vlan.h>
#include <linux/sockios.h>
#include <netpacket/packet.h>

#define NL_BUFSIZE 8192

int main(int argc, char *argv[])
{
	int rc;
	int len;
	int sd;
	int intfc_idx;
    struct nlmsghdr *nl_msg;
    struct ifinfomsg *link_msg;
    struct ifinfomsg *p_info;
    struct rtattr *rt_attr;
    int rt_len;
	int msg_seq = 0;
    char msg_buf[NL_BUFSIZE];

	sd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (sd<0)
	{	
		perror("socket() error\n"); 
		exit (-1);
	}

	memset(msg_buf, 0, NL_BUFSIZE);
    nl_msg = (struct nlmsghdr *) msg_buf;
    link_msg = (struct ifinfomsg *) NLMSG_DATA(nl_msg);

    nl_msg->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    nl_msg->nlmsg_type = RTM_GETLINK;
    nl_msg->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
    nl_msg->nlmsg_seq = msg_seq;
    nl_msg->nlmsg_pid = getpid();

	rc = write(sd, nl_msg, nl_msg->nlmsg_len);
    if (rc < 0) {
		perror("write failed:\n");
		exit(-1);
	}

	while(1) {
		memset(msg_buf, 0, NL_BUFSIZE);
        rc = read(sd, msg_buf, NL_BUFSIZE);
		//printf("Received message size: %d\n", rc);	
		if (rc <= 0) {
			perror("read failed\n");
			exit(-1);
		}
		nl_msg = (struct nlmsghdr *) msg_buf;
        if (nl_msg->nlmsg_seq != msg_seq) {
			printf("Error in seq %d\n", nl_msg->nlmsg_seq);
			continue;
		}
		if (nl_msg->nlmsg_pid != getpid()) {
			printf("message not ment for me pid: %d mypid: %d\n", nl_msg->nlmsg_pid, getpid());
			continue;
		}
		if (nl_msg->nlmsg_type == NLMSG_DONE) {
			printf("Done with the message\n");
            break;
        }
		if (nl_msg->nlmsg_type != RTM_NEWLINK) {
			printf("Inavlid message type: %d\n", nl_msg->nlmsg_type);
			exit(-1);
		}
		len = rc;
		//printf("Got message RTM_NEWLINK len: %d\n", len);
		for (; NLMSG_OK(nl_msg,len); nl_msg = NLMSG_NEXT(nl_msg,len)) {
			
			p_info = (struct ifinfomsg *) NLMSG_DATA(nl_msg);
			//printf(" ------ \n Ifindex: %x ifflags: %x  \n------\n", p_info->ifi_index, p_info->ifi_flags);
			rt_attr = (struct rtattr *) IFLA_RTA(p_info);
            rt_len = IFLA_PAYLOAD(nl_msg);

			for ( ; RTA_OK(rt_attr,rt_len);
                    rt_attr = RTA_NEXT(rt_attr,rt_len)) {
				//printf("RT attributes: type: %x len: %x\n", rt_attr->rta_type, rt_attr->rta_len);
				if (rt_attr->rta_type == IFLA_IFNAME) {
					if (!strncmp((char*)RTA_DATA(rt_attr), "eth0", 4))
						printf("if: %s ifindex: %x ifflags: %x\n", (char*)RTA_DATA(rt_attr), p_info->ifi_index, p_info->ifi_flags);
				}
			}
		}
	}
	return 0;
}
