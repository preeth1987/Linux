#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

int main (int argc, char **argv) {
	
	struct sockaddr_in self, other;
	struct ifreq ifr;
	int sd, ret;

	bzero(&ifr, sizeof(ifr));

	if (argc != 2) {
		printf("\n%%Error Usage:%s <vrfname>\n", argv[0]);
		exit(1);
	}

	sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(sd < 0 ) {
		perror("socket : " );
		return -1;
	}

	strncpy(ifr.ifr_name, argv[1], IF_NAMESIZE);
		
	ret = ioctl(sd, SIOCGIFFLAGS, &ifr);
		
	if ( ret < 0) {
		printf("RET: %d : %s", ret, strerror(errno));
	}
	printf("\n ifr_name: %s\n", ifr.ifr_name);
	printf("flags: %x\n", ifr.ifr_ifru.ifru_flags);

	return 0;
}
