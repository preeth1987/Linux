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

#define NTF_USE     0x01
#define NTF_SELF    0x02
#define NTF_MASTER  0x04
#define NTF_PROXY   0x08    /* == ATF_PUBL */
#define NTF_EXT_LEARNED 0x10
#define NTF_OFFLOADED   0x20
#define NTF_STICKY  0x40
#define NTF_ROUTER  0x80


#define ALL_DEV "all"
#define NL_BUFSIZE 8192

#define NDA_RTA(r) \
    ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))

int nl_parse_rtattr(struct rtattr *tb[], int max,
        struct rtattr *rta, unsigned long int len)
{
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
    while (RTA_OK(rta, len))
    {
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


const char *ll_addr_n2a(const unsigned char *addr, int alen, int type,
        char *buf, int blen)
{
    int i;
    int l;

    snprintf(buf, blen, "%02x", addr[0]);
    for (i = 1, l = 2; i < alen && l < blen; i++, l += 3)
        snprintf(buf + l, blen - l, ":%02x", addr[i]);
    return buf;
}


void fdb_print_state(unsigned int state)
{
    if (state & NUD_PERMANENT)
        printf("%s ", "permanent");

    if (state & NUD_NOARP)
        printf("%s ", "static");

    if (state & NUD_STALE)
        printf("%s ", "stale");

    if (state & NUD_REACHABLE)
        printf("%s ", "");

}
void fdb_print_flags(unsigned int flags)
{

    if (flags & NTF_SELF)
        printf("%s ", "self");

    if (flags & NTF_ROUTER)
        printf("%s ", "router");

    if (flags & NTF_EXT_LEARNED)
        printf("%s ", "extern_learn");

    if (flags & NTF_OFFLOADED)
        printf("%s ", "offload");

    if (flags & NTF_MASTER)
        printf("%s ", "master");

    if (flags & NTF_STICKY)
        printf("%s ", "sticky");

}

int get_all_dev(char devs[8000][17])
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int count=0;
    char * pch;
    int skip_begin_lines=2;

    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
        exit(-1);

    while ((read = getline(&line, &len, fp)) != -1) {
        if (skip_begin_lines > 0)
        {
            skip_begin_lines--;
            continue;
        }
        //printf("Retrieved line of length %zu:\n", read);
        pch = strtok(line,":");
        //printf("%s\n", pch);
        if(strlen(pch) > 0)
            strcpy(devs[count++], pch);
    }

    fclose(fp);
    if (line)
        free(line);
    return count;
}

int main(int argc, char *argv[])
{
    ssize_t rc, len;
    int sd;
    struct nlmsghdr *nl_msg;
    struct ndmsg *p_msg;
    char msg_buf[NL_BUFSIZE];

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

    int family = PF_BRIDGE;

    __s32 ifindex = 0;
    __u8 all_dev = 0;
    ssize_t ret;
    struct rtattr *tb[NDA_MAX+1];
    char ip[128];
    __u16 vid = 0;
    const char *lladdr;
    char b1[64];
    int dst_family;
    struct {
        struct nlmsghdr nlh;
        struct ndmsg ndm;
        char buf[256];
    } req;
    struct ndmsg *ndm;
    char devs[8000][17];
    int dev_count = 0;
    char dev_name[17];

    if (argc > 1)
    {
        if (!strncmp(argv[1], ALL_DEV, strlen(ALL_DEV)))
        {
            all_dev = 1;
            dev_count = get_all_dev(devs);
            ifindex = 0;
            /*for (int k=0;k<=dev_count;k++)
                printf("%s\n", devs[k]);*/
        }
        else
        {
            dev_count = 1;
            ifindex = if_nametoindex(argv[1]);
            strncpy(devs[0], argv[1], 16);
        }
    }

    printf("Dev Count: %d\n", dev_count);
    for (int k=0;k<=dev_count;k++)
    {
        strcpy(dev_name, devs[k]);
        //printf("bridge fdb show dev %s\n", dev_name);
        ifindex = if_nametoindex(devs[k]);
        if (!ifindex)
            continue;
        memset(&req, 0, sizeof(req));
        req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
        req.nlh.nlmsg_type = RTM_GETNEIGH;
        req.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
        req.nlh.nlmsg_seq = 0;
        req.ndm.ndm_family = (uint8_t)family;

        ndm = (struct ndmsg*)NLMSG_DATA(&req.nlh);
        ndm->ndm_flags = 0;
        ndm->ndm_ifindex = ifindex;

        ret = send(sd, &req, sizeof(req), 0);
        if (ret < 0)
        {
            printf("Req failed ret: %ld\n", ret);
            perror("req failed\n");
            exit(-1);
        }

        while (1) {
            memset(msg_buf, 0, NL_BUFSIZE);
            rc = read(sd, msg_buf, NL_BUFSIZE);
            //printf("Received message size: %d\n", rc);

            if (rc <= 0) {
                perror("read failed\n");
                exit(-1);
            }
            nl_msg = (struct nlmsghdr *) msg_buf;
            if (nl_msg->nlmsg_type == NLMSG_DONE) {
                //printf("Done with the message\n");
                goto msg_end;
            }
            if (nl_msg->nlmsg_type != RTM_NEWNEIGH) {
                printf("Inavlid message type: %d\n", nl_msg->nlmsg_type);
                exit(-1);
            }
            len = rc;
            p_msg = (struct ndmsg *) NLMSG_DATA(nl_msg);
            //printf("type:%d len: %lu\n", rt_attr->rta_type, rt_len);

            for (; NLMSG_OK(nl_msg,len); nl_msg = NLMSG_NEXT(nl_msg,len)) {
                vid = 0;
                p_msg = (struct ndmsg *) NLMSG_DATA(nl_msg);
                if (p_msg->ndm_ifindex == ifindex || !ifindex)
                {
                    nl_parse_rtattr(tb, NDA_MAX, NDA_RTA(p_msg), nl_msg->nlmsg_len - NLMSG_LENGTH(sizeof(*p_msg)));

                    if (tb[NDA_VLAN])
                        vid = *(__u16 *)RTA_DATA(tb[NDA_VLAN]);

                    if (tb[NDA_LLADDR])
                    {
                        lladdr = ll_addr_n2a((unsigned char *)RTA_DATA(tb[NDA_LLADDR]),
                                (int)RTA_PAYLOAD(tb[NDA_LLADDR]),
                                0,
                                b1, sizeof(b1));
                        printf("%s ", b1);
                    }
                    printf("dev %s ", (ifindex ? dev_name : if_indextoname(p_msg->ndm_ifindex, NULL)));
                    if (tb[NDA_DST])
                    {
                        dst_family = AF_INET;
                        if (RTA_PAYLOAD(tb[NDA_DST]) == sizeof(struct in6_addr))
                            dst_family = AF_INET6;
                        printf("dst %s ", print_host(dst_family, (int)RTA_PAYLOAD(tb[NDA_DST]), RTA_DATA(tb[NDA_DST])));
                    }
                    if (vid)
                        printf("vlan %hu ", vid);
                    if (tb[NDA_PORT])
                        printf("port %u ", ntohs(*(__u16 *)RTA_DATA(tb[NDA_PORT])));
                    if (tb[NDA_VNI])
                        printf("vni %u ", *(__u32 *)RTA_DATA(NDA_VNI));

                    if (tb[NDA_IFINDEX])
                        printf(" via %s ", if_indextoname(*(__u32 *)RTA_DATA(tb[NDA_IFINDEX]), ip));

                    fdb_print_flags(p_msg->ndm_flags);

                    if (tb[NDA_MASTER])
                        printf("master %s ",if_indextoname(*(__u32 *)RTA_DATA(tb[NDA_MASTER]), ip));

                    fdb_print_state(p_msg->ndm_state);
                    printf("\n");
                }
            }
        }
msg_end:
        dev_name[0] = '\0';
        //printf("End of bridge fdb show dev %s\n", dev_name);
    }

    return 0;
}
