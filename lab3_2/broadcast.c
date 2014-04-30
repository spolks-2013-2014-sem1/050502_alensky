

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>



#define MAXBUF 65536

int sock_rd, sock_wr;
struct sockaddr_in sock_in_rd, sock_in_wr;
char client[10][1000];
int clientsNum = 0;
char *enter = "*_EC";

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
  
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    if( ch == '-')
    {
      return 2;
    }
    else{
      ungetc(ch, stdin);
      return 1;
    }
  }
 
  return 0;
}


char* getip()
{
	 int fd;
	 struct ifreq ifr;

	 fd = socket(AF_INET, SOCK_DGRAM, 0);

	 ifr.ifr_addr.sa_family = AF_INET;

	 strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

	 ioctl(fd, SIOCGIFADDR, &ifr);

	 close(fd);

	 return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

void listen_thread( void *arg )
{
	char buffer_rd[MAXBUF];
	int  buflen_rd, status;
	unsigned sinlen_rd = sizeof(struct sockaddr_in);
	
	while(1)
	{
		buflen_rd = MAXBUF;
		memset(buffer_rd, 0, buflen_rd);
		status = recvfrom(sock_rd, buffer_rd, buflen_rd, 0, (struct sockaddr *)&sock_in_rd, &sinlen_rd);
		
		if(!strcmp(buffer_rd, enter))
		{
		  buflen_rd = MAXBUF;
		  memset(buffer_rd, 0, buflen_rd);
		  status = recvfrom(sock_rd, buffer_rd, buflen_rd, 0, (struct sockaddr *)&sock_in_rd, &sinlen_rd);
		  strcpy(client[clientsNum++], buffer_rd);
		}
		else printf("%s\n", buffer_rd);
	}
}

void speak_thread( void *arg )
{
	char buffer_wr[MAXBUF];
	int buflen_wr, status = 0;
	unsigned sinlen_wr = sizeof(struct sockaddr_in);
	
	while(1)
	{
		int ch = kbhit();
		if(ch)
		{
		  if( ch == 2 )
		  {
		    int i = 0;
		    for( i =0; i < clientsNum; i++ )
		      printf("%s\n", client[i] );
		  }
		  else{
		    memset(buffer_wr, 0, MAXBUF);
		    fflush(stdin);
		    gets(buffer_wr);
		    buflen_wr = strlen(buffer_wr);    
		    status = sendto(sock_wr, buffer_wr, buflen_wr, 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
		  }
		}
		
	}
}

char* get_broadcast( char* name )
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	char host[NI_MAXHOST];

	getstatus = sendto(sock_wr, buffer_wr, buflen_wr, 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);

	if (ifaddrs(&ifaddr) == -1) 
	{
		perror("getifaddrs");
		exit(-1);
	}


	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {

		if (ifa->ifa_addr == NULL)
			continue;

		if (strcmp(ifa->ifa_name, name)!=0)
			continue;
		
		return inet_ntoa ((( struct sockaddr_in*)ifa->ifa_broadaddr)->sin_addr);
	}
           
	return NULL;
}

int main( int argc, char** argv)
{
  unsigned sinlen_rd, sinlen_wr;
  int yes = 1, port = 0, status = 0, arg, reuse = 1;
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

  sock_in_wr.sin_addr.s_addr = htonl(INADDR_ANY*/);
  sock_in_wr.sin_port = htons( 0 );
  sock_in_wr.sin_family = PF_INET;
  
  status = bind(sock_rd, (struct sockaddr *)&sock_in_rd, sinlen_rd);
  printf("Bind Status Reader = %d\n", status);

  status = bind(sock_wr, (struct sockaddr *)&sock_in_wr, sinlen_wr);
  printf("Bind Status Writer = %d\n", status);
  
  status = setsockopt(sock_wr, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
  setsockopt(sock_wr, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  setsockopt(sock_rd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  status = getsockname(sock_rd, (struct sockaddr *)&sock_in_rd, &sinlen_rd);
  printf("Sock port %d\n",htons(sock_in_rd.sin_port));

  sock_in_wr.sin_addr.s_addr=htonl(-1);
  sock_in_wr.sin_port = sock_in_rd.sin_port;
  sock_in_wr.sin_family = PF_INET;
  
  char* ip = getip();
  status = sendto(sock_wr, enter, strlen(enter), 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
  status = sendto(sock_wr, ip, strlen(ip), 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
  
  
  pthread_create( &tid[0], NULL, listen_thread, (void*) arg );
  pthread_create( &tid[1], NULL, speak_thread, (void*) arg );

  pthread_join( tid[0], NULL); 
  pthread_join( tid[1], NULL); 
  
  shutdown(sock_wr, 2);
  close(sock_wr);
  
  shutdown(sock_rd, 2);
  close(sock_rd);
}  
