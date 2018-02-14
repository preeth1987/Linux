#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <string.h>
#include <errno.h>
 
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in6 server;
	struct in6_addr addr;
    char message[1000] , server_reply[2000];
     
	if (argc != 4) {
		printf("\nError: %s <ipv6 add> <ifindex> <remote_adxdr>\n", argv[0]);
		return -1;
	}

    sock = socket(AF_INET6 , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin6_family = AF_INET6;
	inet_pton(AF_INET6, argv[1], &addr);
    server.sin6_addr = addr ;
    server.sin6_port = htons(8881);
	server.sin6_scope_id = atoi(argv[2]);
	server.sin6_flowinfo = 0;

	if ( bind (sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("Bind failed: errno: %d", errno);
		return 1;
	}
 
	inet_pton(AF_INET6, argv[3], &addr);
	server.sin6_addr = addr;
    server.sin6_port = htons(8888);
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("connect failed. Error: errno: %d strerror: %s", errno, strerror(errno));
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
        printf("Enter message : ");
        scanf("%s" , message);
         
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("Server reply :");
        puts(server_reply);
    }
     
    close(sock);
    return 0;
}
