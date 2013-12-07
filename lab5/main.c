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
#include <signal.h>
#include "../spolks_lib/sockcore.c"
#include "netsenderUDP.c"


int main( int argc, char *argv[] )
{
	int    sock, fExists, option, listen = 0, fileDirection = 0;
	char *fileName, *address, *port;
    	struct sockaddr_in addr_in;
	
	if (argc < 2)
	{
		printf("no parametres provided\n ns [-a address] [-p port] [-s send_file] [-r receive_file] [-f file_name] [-l listen]\n");
		return -1;
	}
		
	while( (option = getopt(argc, argv, "lsra:p:f:") ) != -1 )
	{
		switch( option )
		{
			case 'f':
				fileName = optarg;
				break;
			case 'a':
				address = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 'l':
				listen = 1;
				break;
			case 's':
				fileDirection = -1;
				break;
			case 'r':
				fileDirection = 1;
				break;
		}
	}
	fExists = fileExists( fileName );
	
	if( listen )
	{
		switch( fExists )
		{
			case 0:
				perror("file doesn't exists");
				return -1;
			case -1:
				printf("file name has not beev provided\n");
				return -1;
		}
		sock = createUDPListener( &addr_in, port);
	}
	else
	{
		sock = clientUDP( &addr_in, address , port);
	}
	
	switch( fileDirection )
	{
		case -1:
			sendFileUDP( fileName, sock);
			break;
		case 1:
			recieveFileUDP( fileName, sock, &addr_in );
			break;
	}
	
	printf("Press any key...\n");
	getchar();
	
	close(sock);
	return 1;
}

