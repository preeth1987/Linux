#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netfilter_ipv4.h>
#include "../kfile_ops.h"
#include <linux/time.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Preetham");
MODULE_DESCRIPTION("Bridge pkt inspect");

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

void get_info(char *prefix, struct sk_buff *skb)
{

    const char *indev;
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    unsigned int hr, min, sec;
    char buff[256] __attribute__((aligned(sizeof(long))));

    indev = skb->dev ? skb->dev->name : nulldevname;
	
    get_cur_time(&hr, &min, &sec);
    if ((strcmp(indev, "eth0") != 0) && (strcmp(indev, "lo") != 0) ) {
	    sprintf(buff, "[%d:%d:%d] ETH PKT: dev: %s dev_ref: %d prio: %d\n" 
			    "\tSKB INFO: len: %u d-len %u size %u type 0x%x proto 0x%x refdst: %x\n",
			    //"\tdata %p head-%p tail-%p end-%p\n"
			    //"\ttransport_header: %u network_header: %u\n" 
			    hr, min, sec,
                indev, skb->dev->pcpu_refcnt, skb->priority, skb->len, skb->data_len,
                skb->truesize, skb->pkt_type, ntohs(skb->protocol),
                            //skb->data, skb->head, skb->tail, skb->end 
			    skb->transport_header, skb->network_header, skb->_skb_refdst, "");
	    pr_info("%s: %s", prefix, buff);
        }

    return;
}

static unsigned int br_nf_forward_arp(void *priv,
                      struct sk_buff *skb,
                      const struct nf_hook_state *state)
{

    const char *indev;
    const char nulldevname[IFNAMSIZ] __attribute__((aligned(sizeof(long))));
    indev = skb->dev ? skb->dev->name : nulldevname;
    get_info("BRIDGE FWD", skb);
out:
    return NF_ACCEPT;
drop_out:
	return NF_DROP;
}

static struct nf_hook_ops hook_ops[] __read_mostly = {
        /*
		{
		.hook     = (nf_hookfn *)ip_post_routing,
        .pf       = NFPROTO_IPV4,
        .hooknum  = NF_INET_POST_ROUTING,
        .priority = NF_IP_PRI_FIRST,
        },
        {
        .hook     = (nf_hookfn *)ip_inet_forward,
        .pf       = NFPROTO_IPV4,
        .hooknum  = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FIRST,
        },
        {
        .hook     = (nf_hookfn *)ip_pre_routing,
        .pf       = NFPROTO_IPV4,
        .hooknum  = NF_INET_PRE_ROUTING,
        .priority = NF_IP_PRI_FIRST,
        },
        {
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
		{
        .hook = br_nf_forward_arp,
        .pf = NFPROTO_BRIDGE,
        .hooknum = NF_BR_FORWARD,
        .priority = NF_BR_PRI_BRNF,
		},
};

static int __init br_pkt_inspect_init(void)
{
    int ret=0;
	fp = file_open("/var/log/pkt_logs.trc", O_RDWR|O_CREAT, 0777);
	if (!fp)
		return -1;

	printk(KERN_ERR "File opened : %p\n", fp);

    pr_info("initialize of Bridge packet inspection module\n");

	if (ret >= 0) {
		pr_info("write success");
	} else {
		printk(KERN_ERR "%s:%d Error writing to file\n", __FUNCTION__, __LINE__ );
		return ret;
	}

    ret = nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops));
    //ret = nf_register_hooks(hook_ops, sizeof(hook_ops));
    if (ret < 0) {
        return ret;
    }

    return 0;
}

static void __exit br_pkt_inspect_exit(void)
{
    nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));
    //nf_unregister_hooks(hook_ops, sizeof(hook_ops));

	file_sync(fp);
	file_close(fp);
    pr_info("packet inspection module unloaded.\n");
}

module_init(br_pkt_inspect_init);
module_exit(br_pkt_inspect_exit);
