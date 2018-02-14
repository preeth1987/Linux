#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <string.h>
#include <errno.h>
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in6 server , client;
	struct in6_addr addr;
    char client_message[2000];
     
	if (argc != 3) {
		printf("\nError: %s <ipv6 add> <ifindex>\n", argv[0]);
		return (1);
	}

    socket_desc = socket(AF_INET6 , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin6_family = AF_INET6;
	//inet_pton(AF_INET6, argv[1], &addr);
	bzero(&addr, sizeof(struct in6_addr));
    server.sin6_addr = addr;
    server.sin6_port = htons( 8888 );
	server.sin6_scope_id = atoi(argv[2]);
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in6);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        write(client_sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
     
    return 0;
}
