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
#include "../spolks_lib/sockcore.c" 
#include <sys/ioctl.h>


unsigned short cksum(unsigned short *addr, int len)
{
	register int sum = 0;
	u_short answer = 0;
	register u_short *w = addr;
	register int nleft = len;
	/*
	* Our algorithm is simple, using a 32 bit accumulator (sum), we add
	* sequential 16 bit words to it, and at the end, fold back all the
	* carry bits from the top 16 bits into the lower 16 bits.
	*/
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	/* mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		*(u_char *) (&answer) = *(u_char *) w;
		sum += answer;
	}
	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
	sum += (sum >> 16); /* add carry */
	answer = ~sum; /* truncate to 16 bits */
	return (answer);
}

/*char* getip()
{
	char buffer[256];
	struct hostent* h;

	gethostname(buffer, 256);
	h = gethostbyname(buffer);
	return inet_ntoa(*(struct in_addr *)h->h_addr);
}*/


char* getip()
{
	 int fd;
	 struct ifreq ifr;

	 fd = socket(AF_INET, SOCK_DGRAM, 0);

	 /* I want to get an IPv4 IP address */
	 ifr.ifr_addr.sa_family = AF_INET;

	 /* I want IP address attached to "eth0" */
	 strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

	 ioctl(fd, SIOCGIFADDR, &ifr);

	 close(fd);

	 /* display result */
	 return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

int sockfd, sequence = 0, optval, addrlen;
struct sockaddr_in dst_addr, src_addr;
int ident;

void pinger()
{
	struct iphdr* ip;
	struct icmphdr* icmp;
	
	char* packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
	ip = (struct iphdr*) packet;
	icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));
	
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
	ip->id = htons(getpid());
	ip->frag_off = 0;
	ip->ttl = 64;
	ip->protocol = IPPROTO_ICMP;
	ip->saddr = src_addr.sin_addr.s_addr;
	ip->daddr = dst_addr.sin_addr.s_addr;
	ip->check = cksum((unsigned short *)ip, sizeof(struct iphdr));
	
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = ident;
	icmp->un.echo.sequence = sequence++;
	icmp-> checksum = cksum((unsigned short *)icmp, sizeof(struct icmphdr));
	
	//sendto(sockfd, packet + sizeof(struct iphdr), sizeof(struct icmphdr), 0,(struct sockaddr *) &dst_addr, sizeof(struct sockaddr));
	sendto(sockfd, packet, ip->tot_len, 0,(struct sockaddr *) &dst_addr, sizeof(struct sockaddr));
}

void clock_handler(int sig)
{
	pinger();
}

int ping( char * dst )
{
	struct iphdr *ip_reply;
	struct icmphdr *icmp_reply;
	char *buffer, *src;
	int response;
	
	ident = getpid() & 0xFFFF;
	src = getip();

	addr_from_hostname(&dst_addr, dst, NULL);
	addr_from_hostname(&src_addr, src, NULL);
	
	int timeout = 1000;
	
	sockfd = create_socket_ICMP();
	setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout, sizeof(timeout));
	
	printf("Localhost - %s\n", getip());
	
	signal(SIGALRM, clock_handler);
	buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
	alarm(2);
	while( 1 )
	{	
		addrlen = sizeof(dst_addr);
		response = recvfrom(sockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0,(struct sockaddr *) &dst_addr, &addrlen);
		if( response == -1 )
		{
			printf("\nnot response");
		}
		else
		{
			ip_reply = (struct iphdr*) buffer;
			icmp_reply = (struct icmphdr*)(buffer + sizeof(struct iphdr));
			printf("%d bytes from %s: icmp_req = %d ", response , inet_ntoa(src_addr.sin_addr), icmp_reply->un.echo.sequence);
			printf("ID: %d ", ntohs(ip_reply->id));
			printf("TTL: %d\n", ip_reply->ttl);
			
		}
		free(buffer);
		buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
		alarm(2);
	}
	close(sockfd);
} 


/*
* in_cksum --
* Checksum routine for Internet Protocol
* family headers (C Version)
*/
