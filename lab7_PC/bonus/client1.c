/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * client.c
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "helpers.h"

int main(int argc, char *argv[])
{
    int sockfd = 0;
    char buf[BUFLEN];
    struct sockaddr_in serv_addr; 

    if(argc != 3)
    {
        printf("\n Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t sl = sizeof(struct sockaddr);

	if (sockfd == -1) {
		perror("Error initializing socket!");
		exit(1);
	}

    // completare informatii despre adresa serverului
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    // inet_aton(argv[1], &serv_addr.sin_addr);
	serv_addr.sin_port = htons(atoi(argv[2]));

    // conectare la server
    int check = connect(sockfd, (struct sockaddr*) &serv_addr, sl);
    if (check < 0) {
        perror("Send failed!");
        exit(1);
    }

    // citire de la tastatura, trimitere de cereri catre server, asteptare raspuns    
    while (strcmp(buf, "stop") != 0) {
        fgets(buf, BUFLEN, stdin);
        
        send(sockfd, buf, sizeof(buf), 0);
        memset(buf, 0, BUFLEN);
        recv(sockfd, buf, sizeof(buf), 0);

        printf("Received msg from server: %s\n", buf);
    }

    sprintf(buf, "%s", "stop");
    send(sockfd, buf, sizeof(buf), 0);

    // inchidere socket
    close(sockfd);

    return 0;
}
