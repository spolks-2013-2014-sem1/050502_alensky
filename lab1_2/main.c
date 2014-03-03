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
#include <fcntl.h>
#include "ping.c"

int main( int argc, char *argv[] )
{
	int    sock;
	char *address;
    	struct sockaddr_in addr_in;
	
	if (argc < 2)
	{
		printf("no parametres provided\n ping [address]\n");
		return -1;
	}

	ping(argv[1]);
	
	printf("Press any key...\n");
	getchar();
	
	close(sock);
	return 1;
}
