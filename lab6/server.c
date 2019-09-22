/*
 * Protocoale de comunicatii:
 * Laborator 6: UDP
 * mini-server de backup fisiere
 */
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "helpers.h"


void usage(char*file)
{
	fprintf(stderr, "Usage: %s server_port file\n", file);
	exit(0);
}

/*
 * Utilizare: ./server server_port nume_fisier
 */
int main(int argc, char **argv)
{
	if (argc != 3)
		usage(argv[0]);

	int fd, sock;
	socklen_t socklen;
	struct sockaddr_in from_station;
	char buf[BUFLEN];
	// int ret;

	/* TODO deschidere socket */
	sock = socket(PF_INET, SOCK_DGRAM, 0);

	/* TODO setare struct sockaddr_in pentru a specifica unde trimit
	 * datele */
	from_station.sin_family = AF_INET;
	from_station.sin_port = htons(atoi(argv[1]));
	from_station.sin_addr.s_addr = INADDR_ANY;

	/* TODO legare proprietăți de socket */
	bind(sock, (struct sockaddr *) &from_station, sizeof(struct sockaddr_in));

	/* TODO deschidere fișier pentru scriere */
	fd = open(argv[2], O_WRONLY | O_CREAT, 0644);

	/*
	 * TODO
	 * cat_timp  mai_pot_citi
	 *		citeste din socket
	 *		pune in fisier
	 */
	while (1) {
		recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from_station, &socklen);
		int copiat = write(fd, buf, BUFLEN);
		if (strlen(buf) < BUFLEN) {
			break;
		}
	}

	/* TODO închidere socket */
	close(sock);

	/* TODO închidere fișier */
	close(fd);

	return 0;
}
