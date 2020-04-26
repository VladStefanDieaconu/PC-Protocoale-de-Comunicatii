/*
 * Protocoale de comunicatii:
 * Laborator 6: UDP
 * client mini-server de backup fisiere
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

#include "helpers.h"


void usage(char *file)
{
	fprintf(stderr, "Usage: %s ip_server port_server file\n", file);
	exit(0);
}

/*
 * Utilizare: ./client ip_server port_server nume_fisier_trimis
 */
int main(int argc, char **argv)
{
	if (argc != 4)
		usage(argv[0]);

	int fd, sock;
	struct sockaddr_in to_station;
	char buf[BUFLEN];
	// int ret;

	/* TODO deschidere socket */
	sock = socket(PF_INET, SOCK_DGRAM, 0); // descriptorul fisierului


	/* TODO setare struct sockaddr_in pentru a specifica unde trimit
	 * datele */
	to_station.sin_family = AF_INET;
	to_station.sin_port = htons (atoi(argv[2]));
	inet_aton(argv[1], &(to_station.sin_addr));

	/* TODO deschidere fișier pentru citire */
	fd = open(argv[3], O_RDONLY);

	/*
	 * TODO
	 * cat_timp  mai_pot_citi
	 *		citeste din fisier
	 *		trimite pe socket
	 */

	int copiat;
	memset(buf, 0, BUFLEN);
	while ((copiat = read(fd, buf, BUFLEN))) {
		sendto(sock, buf, copiat, 0, &to_station, sizeof(to_station));
		memset(buf, 0, BUFLEN);
	}

	/* TODO închidere socket */
	close(sock);

	/* TODO închidere fișier */
	close(fd);

	return 0;
}
