/*
 * netsender.c
 * 
 * Copyright 2013 artur <artur@artur-VirtualBox>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


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

#define BUFFSIZE 1024

int isDigitStr( const char* str )
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

int isFileExists( const char* fileName )
{
	if( fileName == NULL )
	{
		printf("file name is not provided\n");
		fflush(stdout);
		return -1;
	}
	
	int file = open( fileName, O_RDONLY );
	if( file == -1 )
	{
		return 0;
	}
	close(file);
	return 1;
}

int createSocket()
{

    int clientSocket;

    clientSocket = socket( AF_INET, SOCK_STREAM, 0 );

    if( clientSocket < 0 )
    {
        perror( "impossible to create socket" );
    }

    return clientSocket;
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


	if( ! isDigitStr(port) )
    {
		printf( "port is not valid!\n" );
		fflush(stdout);
		return -1;
    }
    else
    {
		addr_in->sin_port = htons( atoi(port) );
	}
    
    if( !strcmp(addr, "-l") )
	{
		addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
		return 1;
	}
	
	if( !strcmp(addr, "-lst") )
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

int bindSocket(int socket, struct sockaddr_in* addr_in)
{
	if( bind( socket,(struct sockaddr *) addr_in, sizeof( *addr_in ) ) < 0 )
    {
        perror( "impossible to bind socket" );
        return -1;
    }
    return 1;
}

int recieveFile( const char* fileName, int sourceSocket )
{
	int file;
	int recieved_bytes = 0;
	char recieved_data[ BUFFSIZE ];
	
	if( fileName == NULL )
	{
		printf("file name is not provided\n");
		fflush(stdout);
		return -1;
	}
	
	file = open( fileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	
	if( file < 0 )
	{
		perror("can not create file");
		return -1;
	}
	
	
	while( 1 )
	{
		recieved_bytes = recv(sourceSocket, recieved_data, BUFFSIZE, 0);
		if( recieved_bytes == -1 )
		{
			close(file);
			perror("connection failed");
			return -1;
		}
		
		if( recieved_bytes == 0 )
		{
			break;
		}
		
		write(file, recieved_data, recieved_bytes);
		
		if( recieved_bytes < BUFFSIZE )
		{
			break;
		}
	}
	
	close(file);
	return 1;
}

int sendFile( const char* fileName, int targetSocket )
{
	int file;
	int bytesToSend = 0, sentBytes = 0;
	char dataToSend[BUFFSIZE];
	
	if( fileName == NULL )
	{
		printf("file name is not provided\n");
		fflush(stdout);
		return -1;
	}
	if( ( file = open( fileName, O_RDONLY ) ) == -1 )
	{
		perror("file is not found");
		return -1;
	}
	while( 1 )
	{
		bytesToSend = read( file, dataToSend, BUFFSIZE );
		
		sentBytes = send(targetSocket, dataToSend, bytesToSend, 0);
		
		if( sentBytes == -1 )
		{
			close(file);
			perror("connection failed");
			return -1;
		}
		
		if( bytesToSend < BUFFSIZE )
		{
			break;
		}
		
	}
	close(file);
	return 1;
}


