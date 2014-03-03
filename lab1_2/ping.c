#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <string.h>
#include <unistd.h>
#include "../spolks_lib/sockcore.c" 

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

char* getip()
{
	char buffer[256];
	struct hostent* h;

	gethostname(buffer, 256);
	h = gethostbyname(buffer);
	return inet_ntoa(*(struct in_addr *)h->h_addr);
}
	


int ping( char * dst )
{
	int sockfd, sequence = 1, optval;
	struct iphdr* ip, *ip_reply;
    struct icmphdr* icmp;
    char* packet, * buffer, *src;
	struct sockaddr_in dst_addr, src_addr;
	
	src = getip();
	
	
	addr_from_hostname(&dst_addr, dst, NULL);
	addr_from_hostname(&src_addr, src, NULL);
	
	packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
	buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
	
	ip = (struct iphdr*) packet;
	icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));
	
	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 0;
	ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
	ip->id = htons(0);
	ip->frag_off = 0;
	ip->ttl = 64;
	ip->protocol = IPPROTO_ICMP;
	ip->saddr = src_addr.sin_addr.s_addr;
	ip->daddr = dst_addr.sin_addr.s_addr;
	ip->check = cksum((unsigned short *)ip, sizeof(struct iphdr));
	
	
	sockfd = create_socket_ICMP();
	setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));
	
	while(1)
	{
		
		
		icmp->type = ICMP_ECHO;
		icmp->code = 0;
		icmp->un.echo.id = random();
		icmp->un.echo.sequence = sequence;
		
		icmp-> checksum = cksum((unsigned short *)icmp, sizeof(struct icmphdr));
		
		sendto(sockfd, packet, ip->tot_len, 0,(struct sockaddr *) &dst_addr, sizeof(struct sockaddr));
		
		int addrlen = sizeof(dst_addr);
		int response = recvfrom(sockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0,(struct sockaddr *) &dst_addr, &addrlen);
		if( response == -1 )
		{
			printf("\nnot response");
			fflush(stdout);
		}
		else
		{
			printf("%d bytes from %s: icmp_req = %d ", response , dst, sequence);
			ip_reply = (struct iphdr*) buffer;
			printf("ID: %d ", ntohs(ip_reply->id));
			printf("TTL: %d\n", ip_reply->ttl);
			
			fflush(stdout);
		}
	
	}
	
} 


/*
* in_cksum --
* Checksum routine for Internet Protocol
* family headers (C Version)
*/
