/*Compile with -o nl_addr -lnl-3 -lnl-route-3  -lnl-genl-3 -I/usr/include/libnl3 -g*/
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
#include <linux/if_addr.h>
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


int main(int argc, char *argv[])
{
	int rc;
	int len;
	int sd;
	int intfc_idx;
    struct nlmsghdr *nl_msg;
    struct ifaddrmsg *p_msg;
    struct rtattr *rt_attr;
    int rt_len;
	int msg_seq = 0;
    char msg_buf[NL_BUFSIZE];
    int ret;

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

#if 0
    struct nl_cache  *nbr_cache;
    struct rtnl_neigh *neighbor;
    int nbr_state;
    char ips[128];
    rtnl_neigh_alloc_cache(nlsd, &nbr_cache);
    for (neighbor = (struct rtnl_neigh *) nl_cache_get_first(nbr_cache);
		    neighbor != NULL;
		    neighbor = (struct rtnl_neigh *) nl_cache_get_next((struct nl_object*) neighbor))
    {
	    nbr_state = rtnl_neigh_get_state(neighbor);

	    printf("if: %x %s state: %x flags: %x\n",
			    rtnl_neigh_get_ifindex(neighbor),
			    nl_addr2str(rtnl_neigh_get_dst(neighbor), ips, sizeof(ips)),
			    rtnl_neigh_get_state(neighbor),
			    rtnl_neigh_get_flags(neighbor));
    }
#endif

    /*rc = nl_socket_add_membership(nlsd, RTNLGRP_NEIGH);
    if (rc < 0) {
	    perror("membership failed:\n");
	    exit(-1);
    }*/
    sd = nl_socket_get_fd(nlsd);
#define NETLINK_DUMP_STRICT_CHK 12
    int option = 1;
    ret = setsockopt(sd, SOL_NETLINK, NETLINK_DUMP_STRICT_CHK, &option, sizeof(int));
    if (ret < 0)
    {
	    printf("setsockopt to NETLINK_DUMP_STRICT_CHK failed ret: %d errno: %d\n", ret, errno);
	    return 0;
    }

    int family = AF_UNSPEC;
    family = AF_INET6;

    uint32_t ifindex = 0;
    if (argc > 1)
        ifindex = if_nametoindex(argv[1]);

//#if 0
/*    struct {
	struct nlmsghdr nlh;
	struct ndmsg ndm;
	char buf[256];
    } req = {
	.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg)),
	.nlh.nlmsg_type = RTM_GETNEIGH,
	.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST,
	.nlh.nlmsg_seq = 0,
	.ifa.ifa_family = family,
    };
*/
     struct {
         struct nlmsghdr nlh;
         struct ifaddrmsg ifa;
         char buf[128];
     } req;
     memset(&req, 0, sizeof(req));
     req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
     req.nlh.nlmsg_type = RTM_GETADDR;
     req.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
     req.nlh.nlmsg_seq = 0;
     req.ifa.ifa_family = (uint8_t)family;

    struct ifaddrmsg *ifa = NLMSG_DATA(&req.nlh);
    //nl_addattr_l(&req.nlh, sizeof(req), IFA_IFINDEX, &ifindex, sizeof(uint32_t));
    ifa->ifa_index = ifindex;

    //nl_addattr_l(&req.nlh, sizeof(req), NDA_IFINDEX, &ifindex, sizeof(uint32_t));
    printf("Filter address for if: %d\n", ifindex);
    //nla_put_u32(msg, NDA_IFINDEX, ifindex);

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

//#endif

#if 0
    struct nl_msg *msg;
    //msg = nlmsg_alloc(); 
    msg = nlmsg_alloc_simple(RTM_GETNEIGH, NLM_F_DUMP); 

    struct {
	    uint32_t ifindex;
	    char buf[256];
    } buf = {
	    .ifindex = ifindex,
	    .buf[0] = '\0',
    }; 
    //genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, RTM_GETNEIGH, 0, NLM_F_DUMP, NLM_F_REQUEST, 0);
    nlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, RTM_GETNEIGH, 4, NLM_F_DUMP);


    //nla_put_u32(msg, NDA_IFINDEX, 0xc);
    //nla_put(msg, NDA_IFINDEX, sizeof(uint32_t), &ifindex);
    nla_put(msg, NDA_IFINDEX, sizeof(buf), &buf);

    int ret = nl_send_auto(nlsd, msg);
    nlmsg_free(msg);
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
#endif


#if 0
	// type = RTM_GETNEIGH, flags = NLM_F_DUMP
    rc = nl_rtgen_request(nlsd, RTM_GETNEIGH, family, NLM_F_DUMP);
    if (rc < 0) {
	    perror("req failed:\n");
	    exit(-1);
    }
#endif

    struct nl_cache  *nbr_cache;
    struct nl_cache *cache;
    struct rtattr *tb[IFA_MAX+1];
    struct rtnl_neigh *neigh;
    char ip[128];
    while (1)
    {
	    memset(msg_buf, 0, NL_BUFSIZE);
	    rc = read(sd, msg_buf, NL_BUFSIZE);
	    printf("Received message size: %d\n", rc);

#if 0
    int err = rtnl_neigh_parse((struct nlmsghdr *) msg_buf, &neigh);
    nl_addr2str(neigh->n_dst, ip, sizeof(ip));
    printf("%s If: %x state: %x flags %x\n", ip,
		    neigh->n_ifindex, neigh->n_state, neigh->n_flags);
    rtnl_neigh_put(neigh);
#endif
	    if (rc <= 0) {
		    perror("read failed\n");
		    exit(-1);
	    }
	    nl_msg = (struct nlmsghdr *) msg_buf;
	    if (nl_msg->nlmsg_type == NLMSG_DONE) {
		    printf("Done with the message\n");
		    return 0;
	    }
	    if (nl_msg->nlmsg_type != RTM_NEWADDR) {
		    printf("Inavlid message type: %d\n", nl_msg->nlmsg_type);
		    exit(-1);
	    }
#define NL_NDA_RTA(r) \
	    ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
	    len = rc;
	    p_msg = (struct ifaddrmsg *) NLMSG_DATA(nl_msg);
	    rt_attr = (struct rtattr *) IFLA_RTA(p_msg);
	    rt_len = IFLA_PAYLOAD(nl_msg);
	    //nl_parse_rtattr(tb, IFA_MAX, NL_NDA_RTA(p_msg), nl_msg->nlmsg_len - NLMSG_LENGTH(sizeof(*p_msg)));
	    printf("type:%d len: %lu\n", rt_attr->rta_type, rt_len);
	
        for (; NLMSG_OK(nl_msg,len); nl_msg = NLMSG_NEXT(nl_msg,len)) {
            p_msg = (struct ifaddrmsg *) NLMSG_DATA(nl_msg);
            rt_attr = (struct rtattr *) IFLA_RTA(p_msg);
            rt_len = IFLA_PAYLOAD(nl_msg);
            if (p_msg->ifa_index == ifindex || !ifindex)
            {
                printf("Match ifa for if\n");
                nl_parse_rtattr(tb, IFA_MAX, NL_NDA_RTA(p_msg), nl_msg->nlmsg_len - NLMSG_LENGTH(sizeof(*p_msg)));

                if (tb[IFA_LOCAL])
                {
                    printf("Local %s : ", print_host(p_msg->ifa_family, RTA_PAYLOAD(tb[IFA_LOCAL]), RTA_DATA(tb[IFA_LOCAL])));
                }
                if (tb[IFA_ADDRESS])
                {
                    printf("Address %s : ", print_host(p_msg->ifa_family, RTA_PAYLOAD(tb[IFA_ADDRESS]), RTA_DATA(tb[IFA_ADDRESS])));
                }
                printf(" family:%x Ifindex: %x flags: %x\n",
                        p_msg->ifa_family, (unsigned int)p_msg->ifa_index, p_msg->ifa_flags);
#if 0
                for ( ; RTA_OK(rt_attr,rt_len);
                        rt_attr = RTA_NEXT(rt_attr,rt_len)) {
                    printf("RT attributes: type: %x len: %x\n", rt_attr->rta_type, rt_attr->rta_len);
                    if (rt_attr->rta_type == IFLA_IFNAME) {
                        printf("if: %s afi: %x if: %x state: %x flags: %x\n", (char*)RTA_DATA(rt_attr), 
                                p_msg->ifa_family, p_msg->ifa_index, p_msg->ifa_state, p_msg->ifa_flags);
                    }

                }
#endif
            }
            else
            {
                printf("No Match %d ifa for if %d\n", p_msg->ifa_index, ifindex);
            }
        }
    }

	return 0;
}

