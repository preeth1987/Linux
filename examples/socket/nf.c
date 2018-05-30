#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/netlink.h>
#include <errno.h>
#include <string.h>

int main()
{
	int sockfd;
	sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_NETFILTER);
	if (sockfd < 0) {
		printf("errno: %d %s\n", errno, strerror(errno));
	}
	return 0;
}
