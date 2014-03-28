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
  int sock_rd, sock_wr, status, buflen_rd, buflen_wr;
  unsigned sinlen_rd, sinlen_wr;
  char buffer_wr[MAXBUF];
  char buffer_rd[MAXBUF];
  struct sockaddr_in sock_in_rd, sock_in_wr;
  int yes = 1;
  int port = 32943;
  
  sinlen_wr = sizeof(struct sockaddr_in);
  memset(&sock_in_wr, 0, sinlen_wr);

  
  sinlen_rd = sizeof(struct sockaddr_in);
  memset(&sock_in_rd, 0, sinlen_rd);
  
  sock_rd = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  sock_wr = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  
  sock_in_rd.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_in_rd.sin_port = htons(    /*argv[1]  ?  atoi(argv[1]) :*/ port   ); /* port number */
  sock_in_rd.sin_family = PF_INET;

  
  sock_in_wr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_in_wr.sin_port = htons( 0  ); /* port number */
  sock_in_wr.sin_family = PF_INET;
  
  
  status = bind(sock_rd, (struct sockaddr *)&sock_in_rd, sinlen_rd);
  printf("Bind Status Reader = %d\n", status);

  status = bind(sock_wr, (struct sockaddr *)&sock_in_wr, sinlen_wr);
  printf("Bind Status Writer = %d\n", status);
  
  status = setsockopt(sock_wr, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
  status = getsockname(sock_wr, (struct sockaddr *)&sock_in_wr, &sinlen_wr);
  printf("Sock port %d\n",htons(sock_in_rd.sin_port));

  sock_in_wr.sin_addr.s_addr=htonl(-1); /* send message to 255.255.255.255 */
  sock_in_wr.sin_port = sock_in_rd.sin_port; /*htons( port   );*/ /* port number */
  sock_in_wr.sin_family = PF_INET;
  
  sprintf(buffer_wr, "Message");
  buflen_wr = strlen(buffer_wr);
  
  while(1)
  {
    buflen_wr = MAXBUF;
    memset(buffer_wr, 0, buflen_wr);
    
    status = sendto(sock_wr, buffer_wr, buflen_wr, 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
    
    status = recvfrom(sock_rd, buffer_rd, buflen_rd, 0, (struct sockaddr *)&sock_in_rd, &sinlen_rd);
    
    
    if(strcmp(buffer_rd, buffer_wr))
    {
      printf("!!!! FAIL %s != %s\n", buffer_rd, buffer_wr);
      break;
    }
    
    printf("%s = %s\n", buffer_wr, buffer_rd);
    
  }

  shutdown(sock_wr, 2);
  close(sock_wr);
  
  shutdown(sock_rd, 2);
  close(sock_rd);
}  
