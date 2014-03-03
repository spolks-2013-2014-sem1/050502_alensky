#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

int isDigitOnly( const char* str )
{
	int len = strlen( str );
	int i;
	for( i = 0; i < len; i++ )
	{
		if( str[i] < '0' || str[i] > '9' )
			return 0;
	}
	return 1;
}





int bindAddr(struct sockaddr_in* addr_in, const char* addr,const char * port )
{

    addr_in->sin_family = AF_INET;
	
	if( port == NULL )
    {
        printf("port is not provided\n");
        fflush(stdout);
        return -1;
    }


	if( ! isDigitOnly(port) )
    {
		printf( "port is not valid!\n" );
		return -1;
    }
    else
    {
		addr_in->sin_port = htons( atoi(port) );
	}
    
    if( addr == NULL )
	{
		addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
		return 1;
	}
	
	if( !strcmp(addr, "localhost") )
	{
		addr_in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		return 1;
	}
	
	if (inet_pton(AF_INET ,addr, &(addr_in->sin_addr) ) <= 0 )
	{
		perror( "address is not valid!" );
		return -1;
	}
	return 1;
}

int addr_from_hostname(struct sockaddr_in* addr_in, const char* addr, const char * port )
{
	struct hostent* hp;
    addr_in->sin_family = AF_INET;
	
	if( port != NULL )
    {
        if( ! isDigitOnly(port) )
		{
			printf( "port is not valid!\n" );
			return -1;
		}
		else
		{
			addr_in->sin_port = htons( atoi(port) );
		}
    }
	
	addr_in->sin_addr.s_addr = inet_addr(addr);
	
	if(addr_in->sin_addr.s_addr == (u_int) -1 )
	{
		hp = gethostbyname(addr);
		
		if( !hp )
		{
			printf( "unknown host!\n" );
			return -1;
		}
		
		addr_in->sin_family = hp->h_addrtype;
		bcopy(hp->h_addr, (caddr_t)& addr_in->sin_addr, hp->h_length);
	}
	return 1;
}

/* TCP server defines and functions */

int create_socket()
{

    int client_socket;

    client_socket = socket( AF_INET, SOCK_STREAM, 0 );

    if( client_socket < 0 )
    {
        perror( "impossible to create socket" );
    }

    return client_socket;
}

int bindSocket(int socket, struct sockaddr_in* addr_in)
{
	if( bind( socket,(struct sockaddr *) addr_in, sizeof( *addr_in ) ) < 0 )
    {
        perror( "impossible to bind socket" );
        return -1;
    }
    return 1;
}


int createListener(struct sockaddr_in* hostAddr, const char * port )
{
	int result = 0;
	int listener = create_socket();
	if( listener )
		result = bindAddr( hostAddr, NULL, port);
		
	if( result )
		result = bindSocket( listener, hostAddr);
		
	if( result )
		result = listen( listener, 1 );
	if( result < 0 )
	{
		exit(-1);
	}
	return listener;
}

int acceptClient(int listener, struct sockaddr_in* clientAddr)
{
	socklen_t client_addr_len = sizeof( *clientAddr );
	int client = accept(listener, (struct sockaddr*) clientAddr, &client_addr_len );
	if( client < 0 )
	{
		close(listener);
		perror("accepting client");
		exit(-1);
	}
	return client;
}

int connectToRemote(struct sockaddr_in* clientAddr, const char * addr, const char * port)
{
	int result = 0;
	int client = create_socket();
	
	if( client )
		result = bindAddr( clientAddr, addr, port );
	else return -1;
	
	if( result )
		result = connect( client, (struct sockaddr *) clientAddr, sizeof( *clientAddr ) );
		
	if( result < 0 )
	{
		perror("connection failed");
		close( client );
		return result;
	}
	return client;
}

/*UDP functions and defines. Socket creator*/

int createUDPSocket()
{

    int clientSocket;

    clientSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if( clientSocket < 0 )
    {
        perror( "impossible to create UDP socket" );
    }

    return clientSocket;
}

int clientUDP(struct sockaddr_in* clientAddr, const char * addr, const char * port)
{
	int result = 0;
	int client = createUDPSocket();
	
	if( client )
		result = bindAddr( clientAddr, addr, port );
	else return -1;
		
	if( result < 0 )
	{
		perror("connection failed");
		close( client );
		return result;
	}
	return client;
}

int createUDPListener(struct sockaddr_in* hostAddr, const char * port )
{
	int result = 0;
	int listener = createUDPSocket();
	if( listener )
		result = bindAddr( hostAddr, NULL, port);
		
	if( result )
		result = bindSocket( listener, hostAddr);
	if( result < 0 )
	{
		exit(-1);
	}
	return listener;
}


/*ICMP functions and defines. Socket creator*/

int create_socket_ICMP()
{

    int client_socket;

    client_socket = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );

    if( client_socket < 0 )
    {
        perror( "impossible to create ICMP socket" );
    }

    return client_socket;
}



