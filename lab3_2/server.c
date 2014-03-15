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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXBUF 65536

int main( int argc, char** argv)
{
  int sock, sock_2, status, buflen, buflen_2;
  unsigned sinlen, sinlen_2;
  char buffer[MAXBUF];
  char buffer2[MAXBUF];
  struct sockaddr_in sock_in, sock_in_2;
  int yes = 1;

  sinlen = sizeof(struct sockaddr_in);
  memset(&sock_in, 0, sinlen);

  
  sinlen_2 = sizeof(struct sockaddr_in);
  memset(&sock_in_2, 0, sinlen_2);
  
  sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  sock_2 = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_in.sin_port = htons(0);
  sock_in.sin_family = PF_INET;

  
  sock_in_2.sin_addr.s_addr = htonl(-1);
  sock_in_2.sin_port = sock_in.sin_port;
  sock_in_2.sin_family = PF_INET;
  
  
  status = bind(sock, (struct sockaddr *)&sock_in, sinlen);
  printf("Bind Status = %d\n", status);

  status = bind(sock_2, (struct sockaddr *)&sock_in, sinlen);
  status = setsockopt(sock_2, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
  status = getsockname(sock, (struct sockaddr *)&sock_in, &sinlen);
  printf("Sock port %d\n",htons(sock_in.sin_port));

  
  
  
  sprintf(buffer, argv[1]);
  buflen = strlen(buffer);
  
  
  while(1)
  {
    buflen_2 = MAXBUF;
    memset(buffer2, 0, buflen_2);
    
    status = sendto(sock_2, buffer, buflen, 0, (struct sockaddr *)&sock_in_2, sinlen_2);
    
    status = recvfrom(sock, buffer2, buflen_2, 0, (struct sockaddr *)&sock_in, &sinlen);
    
    
    if(strcmp(buffer, buffer2))
    {
      printf("!!!! FAIL %s != %s\n", buffer, buffer2);
      break;
    }
    
    printf("%s = %s\n", buffer, buffer2);
    
  }

  shutdown(sock, 2);
  close(sock);
  
  shutdown(sock_2, 2);
  close(sock_2);
} 
