#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netfilter_ipv6.h>
#include "../kfile_ops.h"
#include <linux/time.h>
#include <net/ip6_fib.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Preetham");
MODULE_DESCRIPTION("IPv6 pkt inspect");

static struct file *fp;

static unsigned int get_cur_time(unsigned int *hr, unsigned int *min, unsigned int *sec)
{
	unsigned long get_time;
	int tmp1,tmp2;
    struct timeval tv;

    do_gettimeofday(&tv);
    get_time = tv.tv_sec;
    *sec = get_time % 60;
    tmp1 = get_time / 60;
    *min = tmp1 % 60;
    tmp2 = tmp1 / 60;
    *hr = (tmp2 % 24);

	return 1;
}

void get_info_v6(char *prefix, struct sk_buff *skb, const struct net_device *in,
        const struct net_device *out)
{

    const char *indev;
    //struct iphdr *iph = (struct iphdr *)skb_network_header(skb);
    struct ipv6hdr *ip6h = (struct ipv6hdr *)skb_network_header(skb);
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    unsigned int hr, min, sec;
    char buff[256] __attribute__((aligned(sizeof(long))));

    indev = skb->dev ? skb->dev->name : nulldevname;
	
    get_cur_time(&hr, &min, &sec);
    if ((strncmp(indev, "eth", 3) != 0) && (strcmp(indev, "lo") != 0) ) {
	sprintf(buff, "\n[%d:%d:%d] IPv6 PKT: v%d, %x:%x:%x:%x -> %x:%x:%x:%x, proto: %u, dev: %s\n" 
			"\tSKB INFO: len: %u d-len %u size %u\n"
			"\tdata %p head-%p tail-%p end-%p\n"
			"\ttype 0x%x proto 0x%x transport_header: %u network_header: %u\n", 
			hr, min, sec,
			ip6h->version, ntohl(ip6h->saddr.s6_addr32[0]), ntohl(ip6h->saddr.s6_addr32[1]), ntohl(ip6h->saddr.s6_addr32[2]), ntohl(ip6h->saddr.s6_addr32[3]),
			ntohl(ip6h->daddr.s6_addr32[0]), ntohl(ip6h->daddr.s6_addr32[1]), ntohl(ip6h->daddr.s6_addr32[2]), ntohl(ip6h->daddr.s6_addr32[3]),
			ip6h->nexthdr, indev, skb->len, skb->data_len, skb->truesize, skb->data, skb->head, skb->tail, skb->end, 
			skb->pkt_type, ntohs(skb->protocol), skb->transport_header, skb->network_header);
	    pr_info("%s: %s", prefix, buff);

            //ff020000:0:0:16
            if (ntohl(ip6h->daddr.s6_addr32[3]) == 0x16)
                dump_stack();
        }

    return;
}

static unsigned int ipv6_pkt_check(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    const char *indev;
    struct ipv6hdr *ip6h = (struct ipv6hdr *)skb_network_header(skb);
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
	char buff[256] __attribute__((aligned(sizeof(long))));
	unsigned int ret = 0;
	unsigned int hr, min, sec;

	get_cur_time(&hr, &min, &sec);

    //indev = in ? in->name : nulldevname;
    indev = skb->dev ? skb->dev->name : nulldevname;

	if (strncmp(indev, "eth", 3) != 0) {
	sprintf(buff, "\n[%d:%d:%d] IPv6 PKT: v%d, %x:%x:%x:%x -> %x:%x:%x:%x, proto: %u, dev: %s\n" 
			"\tSKB INFO: len: %u d-len %u size %u\n"
			"\tdata %p head-%p tail-%p end-%p\n"
			"\ttype 0x%x proto 0x%x transport_header: %u network_header: %u\n", 
			hr, min, sec,
			ip6h->version, ntohl(ip6h->saddr.s6_addr32[0]), ntohl(ip6h->saddr.s6_addr32[1]), ntohl(ip6h->saddr.s6_addr32[2]), ntohl(ip6h->saddr.s6_addr32[3]),
			ntohl(ip6h->daddr.s6_addr32[0]), ntohl(ip6h->daddr.s6_addr32[1]), ntohl(ip6h->daddr.s6_addr32[2]), ntohl(ip6h->daddr.s6_addr32[3]),
			ip6h->nexthdr, indev, skb->len, skb->data_len, skb->truesize, skb->data, skb->head, skb->tail, skb->end, 
			skb->pkt_type, ntohs(skb->protocol), skb->transport_header, skb->network_header);

	pr_info("%s", buff);

	if (skb_dst(skb) && ip6_dst_idev(skb_dst(skb)))
		pr_info("dst_if: %x\n", ip6_dst_idev(skb_dst(skb))->dev->ifindex);
	else {
		if (skb_dst(skb))
			pr_info("ip6_dst_idev: %p\n", ip6_dst_idev(skb_dst(skb)));
		else if (!skb_dst(skb))
			pr_info("skb dst%p\n", skb_dst(skb));
	}

#if 0
	ret = file_write(fp, 0, buff, strlen(buff)); 
	if ( ret < 0) {
		printk(KERN_ERR "%s:%d Error writing to file\n", __FUNCTION__, __LINE__ );
		goto out;
	}
#endif
	if (strncmp(indev,"vrf-",4) == 0)
	{
		goto drop_out;
	}
	}
#if 0
	struct dst_entry *dst = skb_dst(skb);
	sprintf(buff, "\tSKB INFO: skb: %p sk: %p dev: %p len: %u d-len %u size %u \n\t data %p head-%p tail-%p end-%p type 0x%x proto 0x%x",
			"dst-entry 0x%x dst-dev 0x%x dst-idev: 0x%x \n \t pkt_type: %u transport_header: %u network_header: %u mac_header: %u\n",
		skb, skb->sk, skb->dev, skb->len, skb->data_len, skb->truesize, skb->head, skb->tail, skb->end, skb->pkt_type, skb->protocol,
		0, dst?((uint32_t)dst->dev):0, 0, skb->protocol, skb->transport_header, skb->network_header, skb->mac_header);
	
	ret = file_write(fp, 0, buff, strlen(buff)); 
	if ( ret < 0) {
		printk(KERN_ERR "%s:%d Error writing to file\n", __FUNCTION__, __LINE__ );
	}
#endif
out:
    return NF_ACCEPT;
drop_out:
	return NF_DROP;
}

static unsigned int ip6_pre_routing(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    const char *indev;
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    indev = skb->dev ? skb->dev->name : nulldevname;
    get_info_v6("PRE_ROUTING", skb, in, out);
out:
    return NF_ACCEPT;
drop_out:
	return NF_DROP;
}

static unsigned int ip6_inet_forward(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    const char *indev;
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    indev = skb->dev ? skb->dev->name : nulldevname;
    get_info_v6("INET_FORWARD", skb, in, out);
out:
    return NF_ACCEPT;
drop_out:
	return NF_DROP;
}

static unsigned int ip6_post_routing(unsigned int hooknum, struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
    const char *indev;
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    indev = skb->dev ? skb->dev->name : nulldevname;
    get_info_v6("POST_ROUTING", skb, in, out);
    if ((strcmp(indev, "ve200") == 0) || (strcmp(indev, "vrf-2") == 0))
        goto drop_out;
out:
    return NF_ACCEPT;
drop_out:
	return NF_DROP;
}

static unsigned int ip6_pkt_local_out(void *priv, struct sk_buff *skb, 
        const struct nf_hook_state *state)
{
    const char *indev;
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    indev = skb->dev ? skb->dev->name : nulldevname;
    //get_info_v6("LOCAL_OUT", skb, NULL, NULL);
    //pr_info("LOCAL_OUT: %s\n", indev); 

out:
    return NF_ACCEPT;
drop_out:
	return NF_DROP;
}
#if 0
static struct nf_hook_ops hook_ops[] __read_mostly = {
	{
        .hook     = (nf_hookfn *)ipv6_pkt_check,
        .pf       = NFPROTO_IPV6,
        .hooknum  = NF_INET_PRE_ROUTING,
        .priority = NF_IP6_PRI_FIRST,
    }
};
#endif
static struct nf_hook_ops hook_ops[] __read_mostly = {
	{
        .hook     = (nf_hookfn *)ip6_post_routing,
        .pf       = NFPROTO_IPV6,
        .hooknum  = NF_INET_POST_ROUTING,
        .priority = NF_IP6_PRI_FIRST,
        },
        {
        .hook     = (nf_hookfn *)ip6_inet_forward,
        .pf       = NFPROTO_IPV6,
        .hooknum  = NF_INET_FORWARD,
        .priority = NF_IP6_PRI_FIRST,
        },
        {
        .hook     = (nf_hookfn *)ip6_pre_routing,
        .pf       = NFPROTO_IPV6,
        .hooknum  = NF_INET_PRE_ROUTING,
        .priority = NF_IP6_PRI_FIRST,
        },
        /*{
        .hook     = (nf_hookfn *)ip_pkt_check,
        .pf       = NFPROTO_IPV4,
        .hooknum  = NF_INET_LOCAL_IN,
        .priority = NF_IP_PRI_FIRST,
        },*/
        /*{
        .hook     = (nf_hookfn *)ip_pkt_local_out,
        .pf       = NFPROTO_IPV4,
        .hooknum  = NF_INET_LOCAL_OUT,
        .priority = NF_IP_PRI_FIRST,
        }*/
};

static int __init ipv6_pkt_inspect_init(void)
{
    int ret=0;
	fp = file_open("/var/log/pkt_logs.trc", O_RDWR|O_CREAT, 0777);
	if (!fp)
		return -1;

	printk(KERN_ERR "File opened : %p\n", fp);

    pr_info("initialize of IPv6 packet inspection module\n");

	if (ret >= 0) {
		pr_info("write success");
	} else {
		printk(KERN_ERR "%s:%d Error writing to file\n", __FUNCTION__, __LINE__ );
		return ret;
	}

    ret = nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops));
    if (ret < 0) {
        return ret;
    }

    return 0;
}

static void __exit ipv6_pkt_inspect_exit(void)
{
    nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));

	file_sync(fp);
	file_close(fp);
    pr_info("packet inspection module unloaded.\n");
}

module_init(ipv6_pkt_inspect_init);
module_exit(ipv6_pkt_inspect_exit);
