/*
 * main.c
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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

int createSocket();
int bindAddr(struct sockaddr_in*, const char*, const char* );
int bindSocket(int, struct sockaddr_in*);
int recieveFile( const char*, int );
int sendFile( const char*, int );

void sig_urg( int signo );

int    client, oobDataSize;

int main( int argc, char *argv[] )
{
	int    listener, accepted_bytes, fileExists;
    struct sockaddr_in hostAddr;
	struct sockaddr_in clientAddr;
	
	if (argc < 2)
	{
		printf("no parametres provided\n");
		fflush(stdout);
		return -1;
	}
	
	fileExists = isFileExists( argv[4] );
	
	if( fileExists < 0 ) 
		return -1;
	
	
	if( ! strcmp( argv[3], "-s" ) )
	{
		if( fileExists == 0 )
		{
			perror("Error");
			return -1;
		}
	}
	
	if( ! strcmp( argv[1], "-l" ) )
	{
		if( ( listener = createSocket() ) < 0 ) 
			return -1;
		
		if( bindAddr( &hostAddr, argv[1], argv[2] ) < 0 ) 
			return -1;
		
		if( bindSocket( listener, &hostAddr) < 0 ) 
			return -1;
		
		listen( listener, 1 );
		socklen_t client_addr_len = sizeof( clientAddr );
        client = accept(listener, (struct sockaddr*)& clientAddr, &client_addr_len );
		if( client < 0 )
		{
			perror("connection failed");
			close(listener);
			return -1;
		}
	}
	else
	{
		if( ( client = createSocket() ) < 0 ) 
			return -1;
			
		bindAddr( &clientAddr, argv[1], argv[2] );
		
		if( connect( client, (struct sockaddr *) &clientAddr, sizeof( clientAddr ) ) < 0 )
		{
			perror("connection failed");
			close( client );
			return -1;
		}
		
	}
	
	if( ! strcmp( argv[3], "-r" ) )
	{	
		recieveFile( argv[4], client );
	}
	
	if( ! strcmp( argv[3], "-s" ) )
	{
		sendFile( argv[4], client );
	}
	
	close(client);
	close(listener);
	return 1;
}


void sig_urg( int signo )
{
	char buf[1];
	int len = recv( client, buf, 1, MSG_OOB );
	
	if( len < 0 )
	{
		perror("SIGURG HANDLER");
	}
	
	oobDataSize += len;
}
