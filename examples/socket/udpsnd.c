#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define BUFLEN	512
#define COUNT 20
int main(int argc, char **argv)
{
	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN+1];

	if(argc != 3)
	{
		printf("%s <IP> <port> \n", argv[0]);
		printf("ex:%s 10.0.0.1 12334\n", argv[0]);
		return 1;
	}

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
		perror("socket");
		exit(1);
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	for (i=0; i<COUNT; i++) {
		printf("Sending packet %d\n", i);
		sprintf(buf, "This is packet %d\n", i);
		if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
		{
			perror("sendto()");
			exit(1);
		}
	}

	close(s);
	return 0;
}
