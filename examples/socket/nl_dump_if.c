/*Compile with -o nl_dump_if -lnl-3 -lnl-route-3  -lnl-genl-3 -I/usr/include/libnl3 -g*/
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

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/data.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/neighbour.h>
#include <netlink/genl/genl.h>



#define NL_BUFSIZE 8192

#define IFINFO_RTA(r) \
    ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifinfomsg))))
typedef struct
{
    __u16 flags;
    __u16 bytelen;
    __s16 bitlen;
    /* These next two fields match rtvia */
    __u16 family;
    __u32 data[64];
} inet_prefix;

int nl_parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	//unsigned short type;
	//unsigned short flags = 0;
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len))
	{
		//type = rta->rta_type & ~flags;
		if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	return 0;
}

const char *print_host(int af, int len, const void *addr)
{
	static char buf[256];
	switch (af){
		case AF_INET:
		case AF_INET6:
			return inet_ntop(af, addr, buf, 256);
		default:
			return "???";
	}
}


int nl_addattr_l(struct nlmsghdr *n, int maxlen, unsigned short type, const void *data,
          int alen)
{
#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *) (((uint8_t *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

    unsigned short len = (unsigned short)RTA_LENGTH(alen);
    struct rtattr *rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
        fprintf(stderr,
            "addattr_l ERROR: message exceeded bound of %d\n",
            maxlen);
        return -1;
    }
    rta = NLMSG_TAIL(n);
    rta->rta_type = type;
    rta->rta_len = len;
    if (data)
        memcpy(RTA_DATA(rta), data, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
    return 0;
}

int nl_addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
    return nl_addattr_l(n, maxlen, type, &data, sizeof(__u32));
}

const char *ll_addr_n2a(const unsigned char *addr, int alen, int type,
            char *buf, int blen)
{
    int i;
    int l;

    /*if (alen == 4 &&
        (type == ARPHRD_TUNNEL || type == ARPHRD_SIT
         || type == ARPHRD_IPGRE))
        return inet_ntop(AF_INET, addr, buf, blen);

    if (alen == 16 && (type == ARPHRD_TUNNEL6 || type == ARPHRD_IP6GRE))
        return inet_ntop(AF_INET6, addr, buf, blen);
        */
    snprintf(buf, blen, "%02x", addr[0]);
    for (i = 1, l = 2; i < alen && l < blen; i++, l += 3)
        snprintf(buf + l, blen - l, ":%02x", addr[i]);
    return buf;
}


int main(int argc, char *argv[])
{
	int rc;
	int len;
	int sd;
	int intfc_idx;
    struct nlmsghdr *nl_msg;
    struct rtattr *rt_attr;
    int rt_len;
	int msg_seq = 0;
    char msg_buf[NL_BUFSIZE];
    struct ifinfomsg *link_msg;
    struct ifinfomsg *p_info;

    struct nl_sock *nlsd;
    nlsd = nl_socket_alloc();
    if (nlsd == NULL)
    {
	    perror("socket create failed");
	    exit(-1);
    }
    nl_socket_disable_seq_check(nlsd);
    nl_socket_set_buffer_size(nlsd, 16777216, 0);
    rc = nl_connect(nlsd, NETLINK_ROUTE);
    if (rc < 0) {
	    perror("nlroute connect failed");
	    exit(-1);
    }


    sd = nl_socket_get_fd(nlsd);

#define NETLINK_DUMP_STRICT_CHK 12
    int option = 1;
    int ret = setsockopt(sd, SOL_NETLINK, NETLINK_DUMP_STRICT_CHK, &option, sizeof(int));
    if (ret < 0)
    {
        printf("setsockopt to NETLINK_DUMP_STRICT_CHK failed ret: %d errno: %d\n", ret, errno);
        return 0;
    }

    int family = AF_UNSPEC;

    uint32_t ifindex = 0;
    if (argc > 1)
        ifindex = if_nametoindex(argv[1]);

     struct {
         struct nlmsghdr nlh;
         struct ifinfomsg ifinfo;
         char buf[1024];
     } req;
     memset(&req, 0, sizeof(req));
     req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
     req.nlh.nlmsg_type = RTM_GETLINK;
     req.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
     req.nlh.nlmsg_seq = 0;
     req.ifinfo.ifi_family = family;

    struct ifinfomsg *ifinfo = NLMSG_DATA(&req.nlh);

    if (argc > 1)
        nl_addattr_l(&req.nlh, sizeof(req), IFLA_IFNAME, argv[1], strlen(argv[1]) + 1);

    int filtmask = 1;
    nl_addattr_l(&req.nlh, sizeof(req), IFLA_EXT_MASK, &filtmask, sizeof(uint32_t));

    /*ret = send(sd, &req, sizeof(req), 0);
    if (ret < 0)
    {
	    printf("Req failed ret: %d\n", ret);
	    perror("req failed\n");
	    exit(-1);
    }
    else
    {
    	printf("Sent %d bytes req\n", ret);
    }*/
    //ret = nl_sendto(nlsd, &req, sizeof(req));
    ret = send(sd, &req, sizeof(req), 0);
    if (ret < 0)
    {
	    printf("Req failed ret: %d\n", ret);
	    perror("req failed\n");
	    exit(-1);
    }
    else
    {
    	printf("Sent %d bytes req\n", ret);
    }


    struct nl_cache  *nbr_cache;
    struct nl_cache *cache;
    struct rtattr *tb[NDA_MAX+1];
    struct rtnl_neigh *neigh;
    char ip[128];

#if 0
    int err = rtnl_neigh_parse((struct nlmsghdr *) msg_buf, &neigh);
    nl_addr2str(neigh->n_dst, ip, sizeof(ip));
    printf("%s If: %x state: %x flags %x\n", ip,
		    neigh->n_ifindex, neigh->n_state, neigh->n_flags);
    rtnl_neigh_put(neigh);
#endif
    while(1) {
	    memset(msg_buf, 0, NL_BUFSIZE);
	    rc = read(sd, msg_buf, NL_BUFSIZE);
	    printf("Received message size: %d\n", rc);
        if (rc <= 0) {
            perror("read failed\n");
            exit(-1);
        }
        nl_msg = (struct nlmsghdr *) msg_buf;
        if (nl_msg->nlmsg_type == NLMSG_DONE) {
            printf("Done with the message\n");
            return 0;
        }
        if (nl_msg->nlmsg_type != RTM_NEWLINK) {
            printf("Inavlid message type: %d\n", nl_msg->nlmsg_type);
            exit(-1);
        }
        len = rc;
		printf("Got message RTM_NEWLINK len: %d\n", len);
		for (; NLMSG_OK(nl_msg,len); nl_msg = NLMSG_NEXT(nl_msg,len)) {
			
			p_info = (struct ifinfomsg *) NLMSG_DATA(nl_msg);
			printf(" ------ \n Ifindex: %x ifflags: %x  \n------\n", p_info->ifi_index, p_info->ifi_flags);
			rt_attr = (struct rtattr *) IFLA_RTA(p_info);
            rt_len = IFLA_PAYLOAD(nl_msg);

			for ( ; RTA_OK(rt_attr,rt_len);
                    rt_attr = RTA_NEXT(rt_attr,rt_len)) {
				//printf("RT attributes: type: %x len: %x\n", rt_attr->rta_type, rt_attr->rta_len);
				if (rt_attr->rta_type == IFLA_IFNAME) {
						printf("if: %s ifindex: %x ifflags: %x\n", (char*)RTA_DATA(rt_attr), p_info->ifi_index, p_info->ifi_flags);
				}
			}
		}
#if 0
        p_msg = (struct ndmsg *) NLMSG_DATA(nl_msg);
        rt_attr = (struct rtattr *) IFLA_RTA(p_msg);
        rt_len = IFLA_PAYLOAD(nl_msg);
        //nl_parse_rtattr(tb, NDA_MAX, NL_NDA_RTA(p_msg), nl_msg->nlmsg_len - NLMSG_LENGTH(sizeof(*p_msg)));
        printf("type:%d len: %lu\n", rt_attr->rta_type, rt_len);

        for (; NLMSG_OK(nl_msg,len); nl_msg = NLMSG_NEXT(nl_msg,len)) {
            //p_msg = (struct ndmsg*) ((char*)nl_msg + sizeof(struct nlmsghdr))	
            p_msg = (struct ndmsg *) NLMSG_DATA(nl_msg);
            rt_attr = (struct rtattr *) IFLA_RTA(p_msg);
            rt_len = IFLA_PAYLOAD(nl_msg);
            if (p_msg->ndm_ifindex == ifindex)
            {
                nl_parse_rtattr(tb, NDA_MAX, NL_NDA_RTA(p_msg), nl_msg->nlmsg_len - NLMSG_LENGTH(sizeof(*p_msg)));

                if (tb[NDA_DST])
                {
                    printf("%s : ", print_host(p_msg->ndm_family, RTA_PAYLOAD(tb[NDA_DST]), RTA_DATA(tb[NDA_DST])));
                }
                if (tb[NDA_LLADDR])
                {
                    const char *lladdr;
                    char b1[64];
                    lladdr = ll_addr_n2a(RTA_DATA(tb[NDA_LLADDR]),
                            RTA_PAYLOAD(tb[NDA_LLADDR]),
                            0,
                            b1, sizeof(b1));
                    printf(" %s : ", b1);
                }
                printf(" family:%x Ifindex: %x state: %x flags: %x\n",
                        p_msg->ndm_family, (unsigned int)p_msg->ndm_ifindex, p_msg->ndm_state, p_msg->ndm_flags);
                for ( ; RTA_OK(rt_attr,rt_len);
                        rt_attr = RTA_NEXT(rt_attr,rt_len)) {
                    printf("RT attributes: type: %x len: %x\n", rt_attr->rta_type, rt_attr->rta_len);
                    if (rt_attr->rta_type == IFLA_IFNAME) {
                        printf("if: %s afi: %x if: %x state: %x flags: %x\n", (char*)RTA_DATA(rt_attr), 
                                p_msg->ndm_family, p_msg->ndm_ifindex, p_msg->ndm_state, p_msg->ndm_flags);
                    }

                }
            }
        }
#endif
    }

	return 0;
}

