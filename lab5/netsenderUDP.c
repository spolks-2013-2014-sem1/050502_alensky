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
#include <signal.h>

#define BUFFSIZE 1024

int fileExists( const char* fileName )
{
	if( fileName == NULL )
	{
		printf("file name is not provided\n");
		return -1;
	}
	
	FILE* file = fopen( fileName, "r" );
	if( file == NULL )
	{
		return 0;
	}
	fclose(file);
	return 1;
}

int recieveFileUDP( const char* fileName, int targetSocket )
{
	FILE* file;
	int received_bytes = 0, totalReceivedBytes = 0;
	char received_data[ BUFFSIZE ];
	struct sockaddr_in addr;
	int slen = sizeof(addr);
	if( fileName == NULL )
	{
		printf("file name is not provided\n");
		return -1;
	}
	
	file = fopen( fileName, "wb");
	
	if( file == NULL )
	{
		perror("can not create file");
		return -1;
	}
	
	while( 1 )
	{
		received_bytes = recvfrom(targetSocket, received_data, BUFFSIZE, 0, &addr, &slen);
		totalReceivedBytes += received_bytes;
		if( received_bytes == -1 )
		{
			fclose(file);
			perror("connection failed");
			return -1;
		}
		if( received_bytes == 0 )
		{
			break;
		}
		fwrite(received_data, 1, received_bytes, file);
		if( received_bytes < BUFFSIZE )
		{
			break;
		}
	}
	fclose(file);
	return 1;
}

int sendFileUDP( const char* fileName, int sourceSocket, struct sockaddr_in* addr )
{
	FILE* file;
	struct stat st;
	int bytesToSend = 0, sentBytes = 0, percent;
	char dataToSend[BUFFSIZE];
	
	if( fileName == NULL )
	{
		printf("file name is not provided\n");
		return -1;
	}	
	file = fopen( fileName, "rb" );	
	if( file == NULL )
	{
		perror("file is not found");
		return -1;
	}
	
	stat(fileName, &st);
	
	while( 1 )
	{
		bytesToSend = fread( dataToSend, 1, BUFFSIZE, file );
		percent = bytesToSend * 100 / st.st_size;
		sentBytes = sendto(sourceSocket, dataToSend, bytesToSend, 0, addr,  sizeof(addr) );
		printf("sent %d%%\n", percent);
		if( sentBytes == -1 )
		{
			fclose(file);
			perror("connection failed");
			return -1;
		}	
		if( bytesToSend < BUFFSIZE )
		{
			break;
		}
		
	}
	
	fclose(file);
	return 1;
}
