#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>




#define MAXBUF 65536
#define MULTI_ECHO 2
#define START_TYPING 1

struct in_addr localInterface;
struct ip_mreq group;
int sock_rd, sock_wr;
struct sockaddr_in sock_in_rd, sock_in_wr;
char *echo = "*_ECHO", ip;

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
	  status = sendto(sock_wr, ip, strlen(ip), 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
      return MULTI_ECHO;
    }
    else{
      ungetc(ch, stdin);
      return START_TYPING;
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
		
		if(!strcmp(buffer_rd, echo))
		{
		  status = sendto(sock_wr, ip, strlen(ip), 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
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
		if(ch && ch != MULTI_ECHO)
		{
		    memset(buffer_wr, 0, MAXBUF);
		    fflush(stdin);
		    gets(buffer_wr);
		    buflen_wr = strlen(buffer_wr);    
		    status = sendto(sock_wr, buffer_wr, buflen_wr, 0, (struct sockaddr *)&sock_in_wr, sinlen_wr);
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
  
  localInterface.s_addr = inet_addr("203.106.93.94"); // or INADDR_ANY to use DEFAULT interface
  
  group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
  group.imr_interface.s_addr = inet_addr("203.106.93.94"); // or INADDR_ANY to use DEFAULT interface

  setsockopt(sock_rd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  setsockopt(sock_wr, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface));
  setsockopt(sock_wr, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  
  sock_in_wr.sin_family = AF_INET;
  sock_in_wr.sin_addr.s_addr = inet_addr("226.1.1.1");
  sock_in_wr.sin_port = htons(4321);
  
  sock_in_rd.sin_family = AF_INET;
  sock_in_rd.sin_port = htons(4321);
  sock_in_rd.sin_addr.s_addr = INADDR_ANY;
  
  if(bind(sock_rd, (struct sockaddr*)&sock_in_rd, sizeof(sock_in_rd)))
  {
    perror("Binding datagram socket error");
    close(sd);
    exit(1);
  }
  
  setsockopt(sock_rd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));
  
  ip = getip();

  pthread_create( &tid[0], NULL, listen_thread, (void*) arg );
  pthread_create( &tid[1], NULL, speak_thread, (void*) arg );

  pthread_join( tid[0], NULL); 
  pthread_join( tid[1], NULL); 
  
  shutdown(sock_wr, 2);
  close(sock_wr);
  
  shutdown(sock_rd, 2);
  close(sock_rd);
}  
