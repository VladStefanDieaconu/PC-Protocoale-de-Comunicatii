#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

std::vector<int> parse_buffer(char* buffer) {
	std::vector<int> result;
	for (int i = 0; i < strlen(buffer); ++i) {
		int tmp = buffer[i] - '0';
		result.push_back(tmp);
	}
	return result;
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 3) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_aton(argv[1], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	// Receive list of ther connected clients
	// n = recv(sockfd, buffer, sizeof(buffer), 0);
	// DIE(n < 0, "recv");
	// std::vector<int> other_clients = parse_buffer(buffer);
	// other_clients.push_back(sockfd);

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	FD_SET(0, &fds);

	while (1) {
  		// se citeste de la tastatura
		// memset(buffer, 0, BUFLEN);
		
		ret = select(sockfd + 1, &fds, NULL, NULL, NULL);
		if (strncmp(buffer, "exit", 4) == 0) {
			break;
		}
		memset(buffer, 0, BUFLEN);
		if (FD_ISSET(sockfd, &fds)) {
			n = recv(sockfd, buffer, BUFLEN, 0);
			// DIE(n < 0, "recv");

			printf("Received message: %s\n", buffer);
		}
		else if (FD_ISSET(0, &fds)) {
			fgets(buffer, BUFLEN - 1, stdin);

			// se trimite mesaj la server
			n = send(sockfd, buffer, BUFLEN, 0);
			DIE(n < 0, "send");
		}

		// If the other-connected-clients list updated:
		// if ((buffer[0] - '0' > 3) && (buffer[0] - '0' <= 9)) {
		// 	other_clients = parse_buffer(buffer);
		// }
	}

	close(sockfd);

	return 0;
}
