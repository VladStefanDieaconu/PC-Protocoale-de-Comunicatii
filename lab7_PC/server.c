/*
 * Protocoale de comunicatii
 * Laborator 7 - TCP
 * Echo Server
 * server.c
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "helpers.h"

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    char buf[BUFLEN];
    char buff[BUFLEN];
    struct sockaddr_in serv_addr;
    memset(buf, 0, BUFLEN);

    if(argc != 3)
    {
        printf("\n Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    // deschidere socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t sl = sizeof(struct sockaddr);

    // completare informatii despre adresa serverului
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // inet_aton(INADDR_ANY, &serv_addr.sin_addr);
	serv_addr.sin_port = htons(atoi(argv[2]));

    // legare proprietati de socket
    bind(listenfd, (struct sockaddr*) &serv_addr, sl);

    // ascultare de conexiuni
    listen(listenfd, 0);
    connfd = accept(listenfd, (struct sockaddr*) &serv_addr, &sl);

    // acceptarea unei conexiuni, primirea datelor, trimiterea raspunsului
    while(strcmp(buf, "stop") != 0) {
        memset(buf, 0, BUFLEN);
        recv(connfd, buf, sizeof(buf), 0);

        send(connfd, buf, sizeof(buf), 0);
    }
    

    // inchidere socket(i)
    close(listenfd);
    close(connfd);

    return 0;
}
