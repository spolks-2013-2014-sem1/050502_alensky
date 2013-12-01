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

int recieveFile( const char* fileName, int sourceSocket )
{
	FILE* file;
	int received_bytes = 0, totalReceivedBytes = 0;
	char received_data[ BUFFSIZE ];
	
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
	
	if( fcntl( sourceSocket, F_SETOWN, getpid() ) < 0 )
	{
		perror("receiveFile");
		return -1;
	}
	
	while( 1 )
	{
		received_bytes = recv(sourceSocket, received_data, BUFFSIZE, 0);
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

int sendFile( const char* fileName, int targetSocket )
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
		sentBytes = send(targetSocket, dataToSend, bytesToSend, 0);	
		usleep(10000);
		percent = sentBytes * 100 / st.st_size;
		printf(stderr, "sent %d%%\n", percent);
		send(targetSocket, &percent, 1 , MSG_OOB);
		if( sentBytes == -1 )
		{
			fclose(file);
			perror("connection failed");
			return -1;
		}	
		if( bytesToSend < BUFFSIZE )
			break;
		
	}
	
	fclose(file);
	return 1;
}


