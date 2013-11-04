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

int main( int argc, char *argv[] )
{
	int    client, listener, accepted_bytes;
    struct sockaddr_in hostAddr;
	struct sockaddr_in clientAddr;

	if (argc < 2)
	{
		error("ERROR, no parametres provided");
	}

	if( ! strcmp( argv[1], "-l" ) )
	{
		listener = createSocket();
		bindSocket( listener, &hostAddr, argv[1], argv[2]);
		listen( listener, 1 );
		socklen_t client_addr_len = sizeof( clientAddr );
        client = accept(listener, (struct sockaddr*)& clientAddr, &client_addr_len );
		if( client < 0 )
		{
			error("Error, connection failed");
		}
	}
	else
	{
		client = createSocket();
		bindSocket( client, &clientAddr, argv[1], argv[2]);
		if( connect( client, (struct sockaddr *) &clientAddr, sizeof( clientAddr ) ) < 0 )
		{
			error("Error, connection failed");
		};
	}
	
	if( ! strcmp( argv[3], "-ld" ) )
	{
		recieveFile( argv[4], client );
	}
	
	if( ! strcmp( argv[3], "-snd" ) )
	{
		sendFile( argv[4], client );
	}
	return 1;
}


