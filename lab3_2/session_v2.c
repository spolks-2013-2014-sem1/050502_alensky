/* $Id$ */

/*
 * build instructions:
 *
 * gcc -o bserver bserver.c
 *
 * Usage:
 * ./bserver
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUF 65536

int sock_rd, sock_wr;
struct sockaddr_in sock_in_rd, sock_in_wr;

void listen_thread( void *arg )
{
	char buffer_rd[MAXBUF];
	int  buflen_rd, status;
	unsigned sinlen_rd = sizeof(struct sockaddr_in);
	
	while(1)
	{
		buffer_rd = MAXBUF;
		memset(buffer_rd, 0, buffer_rd);
		status = recvfrom(sock_rd, buffer_rd, buflen_rd, 0, (struct sockaddr *)&sock_in_rd, &sinlen_rd);
	}
}

void speak_thread( void *arg )
{
	char buffer_wr[MAXBUF];
	int buflen_wr;
	unsigned sinlen_wr = sizeof(struct sockaddr_in);
	while(1)
	{
		memset(buffer_wr, 0, MAXBUF);
		
		scanf( "%s", buffer_wr);
		buflen_wr = strlen(buffer_wr);
		
		status = sendto(sock_wr, buffer_wr, buflen_wr, 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
	}
}

int main( int argc, char** argv)
{
  unsigned sinlen_rd, sinlen_wr;
  int yes = 1, port = 0;
  pthread_t tid[2];
  
  sinlen_wr = sizeof(struct sockaddr_in);
  memset(&sock_in_wr, 0, sinlen_wr);

  sinlen_rd = sizeof(struct sockaddr_in);
  memset(&sock_in_rd, 0, sinlen_rd);
  
  sock_rd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  sock_wr = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  sock_in_rd.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_in_rd.sin_port = htons(    argv[1]  ?  atoi(argv[1]) : port   );
  sock_in_rd.sin_family = PF_INET;

  sock_in_wr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_in_wr.sin_port = htons( 0 );
  sock_in_wr.sin_family = PF_INET;
  
  status = bind(sock_rd, (struct sockaddr *)&sock_in_rd, sinlen_rd);
  printf("Bind Status Reader = %d\n", status);

  status = bind(sock_wr, (struct sockaddr *)&sock_in_wr, sinlen_wr);
  printf("Bind Status Writer = %d\n", status);
  
  status = setsockopt(sock_wr, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
  status = getsockname(sock_rd, (struct sockaddr *)&sock_in_rd, &sinlen_rd);
  printf("Sock port %d\n",htons(sock_in_rd.sin_port));

  sock_in_wr.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
  sock_in_wr.sin_port = sock_in_rd.sin_port; /*htons( port   );*/ /* port number */
  sock_in_wr.sin_family = PF_INET;
  
  pthread_create( &tid[0], NULL, listen_thread, NULL );
  pthread_create( &tid[1], NULL, speak_thread, NULL );

  pthread_join( tid[0], NULL); 
  pthread_join( tid[1], NULL); 
  
  shutdown(sock_wr, 2);
  close(sock_wr);
  
  shutdown(sock_rd, 2);
  close(sock_rd);
}  
