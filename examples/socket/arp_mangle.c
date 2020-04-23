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

#include <linux/netfilter_arp/arp_tables.h>
#include <linux/netfilter/x_tables.h>
#include <limits.h>

#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_SYS_MODPROBE "/proc/sys/kernel/modprobe"

typedef struct arptc_handle *arptc_handle_t;
arptc_handle_t arptc_init(const char *tablename);

const char *program_name;
//const char *program_version = ARPTABLES_VERSION;

static char *get_modprobe(void)
{
    int procfile;
    char *ret;

    procfile = open(PROC_SYS_MODPROBE, O_RDONLY);
    if (procfile < 0)
        return NULL;

    ret = malloc(1024);
    if (ret) {
        int read_bytes = read(procfile, ret, 1024);
        switch (read_bytes) {
        case -1: goto fail;
        case 1024: goto fail; /* Partial read.  Wierd */
        }
        ret[read_bytes] = '\0';
        if (ret[strlen(ret)-1]=='\n')
            ret[strlen(ret)-1]=0;
        close(procfile);
        return ret;
    }
 fail:
    free(ret);
    close(procfile);
    return NULL;
}

int arptables_insmod(const char *modname, const char *modprobe)
{
    char *buf = NULL;
    char *argv[3];

    /* If they don't explicitly set it, read out of kernel */
    if (!modprobe) {
        buf = get_modprobe();
        if (!buf)
            return -1;
        modprobe = buf;
    }

    switch (fork()) {
    case 0:
        argv[0] = (char *)modprobe;
        argv[1] = (char *)modname;
        argv[2] = NULL;
        execv(argv[0], argv);

        /* not usually reached */
        exit(0);
    case -1:
        return -1;

    default: /* parent */
        wait(NULL);
    }

    free(buf);
    return 0;
}

int mangle_init(void)
{
	int sock;
	int ret;
	int len;
    struct arpt_getinfo arpget;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sock < 0)
		return -1;


    len = sizeof(arpget);
    strncpy(arpget.name, "filter", 7);
    ret = getsockopt(sock, IPPROTO_IP, ARPT_SO_SET_REPLACE, &arpget, &len);
    if (ret < 0)
	{
		close(sock);
		return -1;
	}

}
int main()
{
#define IFACE "vlan+" 
        int sock;
        int ret;
        int bufsize;
        char *buf;
        struct arpt_replace *areplace;
        struct arpt_getinfo arpget;
        int len = 0;
        program_name = "arptables";
        char *table = "filter";
        arptc_handle_t handle = NULL;
		const char *modprobe = NULL;

        //handle = arptc_init(table);

#if 0
        sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

        len = sizeof(arpget);
        len -= 2 * sizeof(unsigned int);
        strncpy(arpget.name, "filter", 7);
        ret = getsockopt(sock, IPPROTO_IP, ARPT_SO_SET_REPLACE, &arpget, &len);
        if (ret < 0)
                printf("ERROR: ret:%d errno: %d %s\n", ret, errno, strerror(errno));
#endif
		sock = mangle_init();
		if (sock < 0)
		{
			arptables_insmod("arp_tables", modprobe);
			sock = mangle_init();
			if (sock < 0) {
				printf("Error: ret:%d errno: %d %s\n", sock, errno, strerror(errno));
				exit(0);
			}
		}


        bufsize = (2 * sizeof(struct arpt_replace)) + 1*sizeof(struct arpt_entry);
        areplace = (struct arpt_replace*)malloc(bufsize);
        memset(areplace, 0, bufsize);

        strncpy(areplace->name, "filter", 7);
        areplace->valid_hooks = 7;
        areplace->num_entries = 5;
        areplace->size = 1184;
        areplace->hook_entry[0] = 0;
        areplace->hook_entry[1] = 464;
        areplace->hook_entry[2] = 696;

        areplace->underflow[0] = 232;
        areplace->underflow[1] = 464;
        areplace->underflow[2] = 696;

        areplace->num_counters = 4;

        areplace->entries[0].arp.arhln = 6;
        areplace->entries[0].arp.arhln_mask = 255;
        strncpy(areplace->entries[0].arp.iniface, IFACE, strlen(IFACE));
        memset(areplace->entries[0].arp.iniface_mask, 0xFF, 3);
        memset(areplace->entries[0].arp.iniface_mask + 3, 0, IFNAMSIZ-3);

        ret = setsockopt(sock, IPPROTO_IP, ARPT_SO_SET_REPLACE, areplace, bufsize);
        if (ret < 0)
                printf("ERROR: ret:%d errno: %d %s\n", ret, errno, strerror(errno));

        close(sock);
        free(areplace);
        return 0;
}
