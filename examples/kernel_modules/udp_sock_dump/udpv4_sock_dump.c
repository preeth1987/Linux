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
#include <net/udp.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Preetham");
MODULE_DESCRIPTION("UDP IPv4 socket dump");

static struct file *fp;
extern struct udp_table udp_table __read_mostly;
static int __init udpv4_sock_dump_init(void)
{
    int ret=0, i=0;
    struct sock *sk;
    struct udp_hslot *hslot;

    pr_info("initialize of udpv4 sock dump module\n");

    if (ret >= 0) {
        pr_info("write success");
    } else {
        printk(KERN_ERR "%s:%d Error writing to file\n", __FUNCTION__, __LINE__ );
        return ret;
    }

    pr_info("udp table mask: %d", udp_table.mask);
    for (i = 0; i <= udp_table.mask; i++) {
        hslot = &udp_table.hash[i];
        sk_for_each_rcu(sk, &hslot->head) {
            pr_info("socket: family: %d prot: %d daddr: %x dport: %d rx_dst: %s ref: %d state: %x flags: %x", 
                    sk->sk_family, (char)sk->sk_protocol ,htons(sk->sk_daddr), htons(sk->sk_dport), (sk->sk_rx_dst?sk->sk_rx_dst->dev->name: "NULL"), sk->sk_refcnt,
                    sk->sk_state, sk->sk_flags);
        }
    }

    return 0;
}

static void __exit udpv4_sock_dump_exit(void)
{
    pr_info("packet inspection module unloaded.\n");
}

module_init(udpv4_sock_dump_init);
module_exit(udpv4_sock_dump_exit);
