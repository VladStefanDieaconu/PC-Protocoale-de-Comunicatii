#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    int i;
    char *message;
    char *response;
    int sockfd;

    /*
    *   Ex 0: Get cs.curs.pub.ro
    *   
    *   Pas 1: Se deschide conexiunea (open_connection)
    *   Pas 2: Se creaza mesajul de request (compute_get_request)
    *   Pas 3: Se trimite la server mesajul (send_to_server)
    *   Pas 4: Se primeste raspuns de la server (receive_from_server)
    *   Pas 5: Se inchide conexiunea cu serverul (close_connection)
    */
    sockfd = open_connection("82.78.81.172", 8081, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("82.78.81.172", "/videos", NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    free(response);
    close_connection(sockfd);

    /*
        Ex 1: Requestul GET pentru Videos
    */
    // sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);

    // message = compute_get_request(IP_SERVER, "/videos", NULL);
    // send_to_server(sockfd, message);
    // response = receive_from_server(sockfd);
    // puts(response);
    // free(response);

    // close_connection(sockfd);

    /*
        EX 2.1: Requestul POST pentru Videos
    */
    sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);

    message = compute_post_request(IP_SERVER, "/videos", "id=15&name=Terminus Paradis");
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    // free(response);

    close_connection(sockfd);

    /*
        Ex 2.2 Verificam noua colectie de videoclipuri
    */
    sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);

    message = compute_get_request(IP_SERVER, "/videos", NULL);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    // free(response);

    close_connection(sockfd);

    /* 
        Ex 3: Autentificare
    */
    sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);

    message = compute_post_request(IP_SERVER, "/weather/login", "username=admin&password=p@ss");
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    free(response);

    close_connection(sockfd);
    return 0;
}