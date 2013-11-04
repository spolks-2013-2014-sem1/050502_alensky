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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFFSIZE 1024

int createSocket()
{

    int _socket;

    _socket = socket( AF_INET, SOCK_STREAM, 0 );

    if( _socket < 0 )
    {
        error( "ERROR, impossible to create socket" );
    }

    return _socket;
}

void bindSocket(int _socket, struct sockaddr_in* addr_in, const char* addr,const char * port )
{

    addr_in->sin_family = AF_INET;
	
	if( !strcmp(addr, "-l") || !strcmp(addr, "-lchst") )
	{
		addr = "0.0.0.0";
	}

	if( port == NULL )
    {
        error("Error, port is not provided");
    }

	if (inet_pton(AF_INET ,addr, &(addr_in->sin_addr) ) <= 0 )
	{
		error( "Error, address is not valid!" );
	}

	if( ( addr_in->sin_port = htons( atoi(port) ) ) == 0)
    {
		error( "Error, port is not valid!" );
    }

    if( bind( _socket,(struct sockaddr *) addr_in, sizeof( *addr_in ) ) < 0 )
    {
        error( "ERROR, impossible to bind socket" );
    }

}



void recieveFile( const char* path, int sourceSocket )
{
	int file = open( path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	int recieved_bytes = 0;
	char recieved_data[ BUFFSIZE ];
	
	if( path == NULL )
	{
		error("ERROR, path is not provided");
	}
	
	if( file < 0 )
	{
		close(sourceSocket);
		error("Error, can not create file");
	}
	
	
	while( 1 )
	{
		recieved_bytes = recv(sourceSocket, recieved_data, BUFFSIZE, 0);
		if( recieved_bytes == -1 )
		{
			close(file);
			error("Error, connection failed");
		}
		
		write(file, recieved_data, recieved_bytes);
		
		if( recieved_bytes < BUFFSIZE )
		{
			break;
		}
	}
}

void sendFile( const char* path, int targetSocket )
{
	int file;
	int bytesToSend = 0, sentBytes = 0;
	char dataToSend[BUFFSIZE];
	
	if( path == NULL )
	{
		error("ERROR, file is not provided");
	}
	if( ( file = open( path, O_RDONLY ) ) == -1 )
	{
		error("ERROR, file is not found");
	}
	while( 1 )
	{
		bytesToSend = read( file, dataToSend, BUFFSIZE );
		
		sentBytes = send(targetSocket, dataToSend, bytesToSend, 0);
		
		if( sentBytes == -1 )
		{
			close(file);
			error("Error, connection failed");
		}
		
		if( bytesToSend < BUFFSIZE )
		{
			break;
		}
		
	}
}


