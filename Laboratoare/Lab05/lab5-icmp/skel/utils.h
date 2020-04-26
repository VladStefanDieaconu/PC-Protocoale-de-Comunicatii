#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>

#define DIE(condition, msg) \
do { \
    if (condition) { \
	fprintf(stderr, "(%s:%d): %s\n", __FILE__, __LINE__, msg); \
	exit(1); \
} \
} while (0)

struct icmp_response {
	struct icmphdr header;
	double timeElapsed;
};

char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con);
char* reverse_dns_lookup(char *ip_addr);
unsigned short
in_cksum(const unsigned short *addr, int len, unsigned short csum);

#endif /* _UTILS_H_ */
