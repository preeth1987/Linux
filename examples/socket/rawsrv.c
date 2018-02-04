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
#include "ipudp.h"
#include "csum.h"

#define PKTLEN 2048

int main(int argc, char* argv[])
{
	int sfd;

	char buffer[PKTLEN+1];
	char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
	struct sockaddr_in sin;
	struct sockaddr_storage inc_packet;
	socklen_t inc_packetl;
	int bytes_read;
	memset(buffer,0,PKTLEN);

	if (argc != 3) {
		printf("%%Error Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sfd=socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
	if(sfd<0){
		perror("socket() error\n");
		exit(1);
	}
	bzero(&sin, sizeof(sin));
	bzero(&inc_packet, sizeof(inc_packet));

	sin.sin_family = AF_INET;
	//sin.sin_port = htons(atoi(argv[2]));
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if (inet_aton(argv[1], &sin.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	
	if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin))<0){
		perror("bind() error\n");
		exit(1);
	}

	int x =0;
	while(x!=10) {
		bytes_read=recvfrom(sfd,buffer,PKTLEN,0,(struct sockaddr*)&inc_packet, (socklen_t*)&inc_packetl);  
		if(bytes_read<0){
			perror("recvfrom: \n");
			close(sfd);
			return -1;
		}
		if(getnameinfo((struct sockaddr*)&inc_packet,inc_packetl,hbuf,sizeof(hbuf),sbuf,sizeof(sbuf),0)<0){
			perror("getnameinfo() error\n");
		}
		printf("Rcvd Buffer of %d Bytes: %s\n",inc_packetl, buffer);
		printf("From Host:%s srv:%s\n", hbuf, sbuf); 
		x++;
	}

	close(sfd);
	return 0;
}
