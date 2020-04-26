#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con)
{
	struct hostent *host_entity;
	char *ip=(char*)malloc(NI_MAXHOST*sizeof(char));
	int i;

	if ((host_entity = gethostbyname(addr_host)) == NULL) {
		return NULL;
	}

	//filling up address structure
	strcpy(ip, inet_ntoa(*(struct in_addr *)
				host_entity->h_addr));

	(*addr_con).sin_family = host_entity->h_addrtype;
	(*addr_con).sin_port = htons(0);
	(*addr_con).sin_addr.s_addr  = *(long*)host_entity->h_addr;

	return ip;

}

char* reverse_dns_lookup(char *ip_addr)
{
	struct sockaddr_in temp_addr;
	socklen_t len;
	char buf[NI_MAXHOST], *ret_buf;

	temp_addr.sin_family = AF_INET;
	temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
	len = sizeof(struct sockaddr_in);

	if (getnameinfo((struct sockaddr *) &temp_addr, len, buf,
				sizeof(buf), NULL, 0, NI_NAMEREQD)) {
		return NULL;
	}
	ret_buf = (char*)malloc((strlen(buf) +1)*sizeof(char) );
	strcpy(ret_buf, buf);
	return ret_buf;
}

unsigned short
in_cksum(const unsigned short *addr, int len, unsigned short csum)
{
	int nleft = len;
	const unsigned short *w = addr;
	unsigned short answer;
	int sum = csum;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1)
		sum += *(unsigned char *)w; /* le16toh() may be unavailable on old systems */

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);

}
