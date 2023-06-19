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
#include <errno.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <errno.h>


#define MAX_LEN 512

int main(int argc, char* argv[])
{
	if (argc!=2) {
		printf("Usage: %s interface\n", argv[0]);
		exit(1);
    }

    const char *tundev = "/dev/net/tun";
    int fd = open(tundev, O_RDWR);
    if (fd < 0)
    {
	    printf("\nTun dev open failed errno: %d\n", errno);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = (short int)(IFF_TAP | IFF_MULTI_QUEUE | IFF_NO_PI);
#define MAX_INTERFACE_NAME_LEN (IFNAMSIZ-1)
    strncpy(ifr.ifr_name, argv[0], MAX_INTERFACE_NAME_LEN);
    
/*#define IFF_TUN_ALIAS 0x0008
#define IFALIASZ 256
	struct tun_parm {
		char alias[IFALIASZ];
	}p;
	ifr.ifr_flags |= IFF_TUN_ALIAS;
	strncpy(p.alias, argv[0], IFALIASZ);
	ifr.ifr_data = (char*)&p;*/

	int err = ioctl(fd, TUNSETIFF, (void *) &ifr);

	if (err < 0)
	{
        printf("\nErr: Failed with err %d %d %s\n", err, errno, strerror(errno));
		close(fd);
		return err;
	}
		close(fd);

	return 0;
}
