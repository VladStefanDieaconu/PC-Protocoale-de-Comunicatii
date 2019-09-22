// Vlad-Stefan Dieaconu 321CA
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"

void error(const char *msg) {
  perror(msg);
  exit(0);
}
void compute_message(char *message, char *line) {
  strcat(message, line);
  strcat(message, "\r\n");
  memset(line, 0, LINELEN);
}
int open_connection(char *host_ip, int portno, int ip_type, int socket_type,
                    int flag) {
  struct sockaddr_in serv_addr;
  int sockfd = socket(ip_type, socket_type, flag);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = ip_type;
  serv_addr.sin_port = htons(portno);
  inet_aton(host_ip, &serv_addr.sin_addr);

  if (sockfd < 0) {
    error("ERROR opening socket");
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }

  return sockfd;
}

void close_connection(int sockfd) { close(sockfd); }
void send_to_server(int sockfd, char *message) {
  int bytes, sent = 0;
  int total = strlen(message);
  do {
    bytes = write(sockfd, message + sent, total - sent);
    if (bytes < 0) {
      error("ERROR writing message to socket");
    }
    if (bytes == 0) {
      break;
    }
    sent += bytes;
  } while (sent < total);
}

char *receive_from_server(int sockfd) {
  char *response = (char *)calloc(BUFLEN, sizeof(char));
  int total = BUFLEN;
  int received = 0;
  do {
    int bytes = read(sockfd, response + received, total - received);
    if (bytes < 0) error("ERROR reading response from socket");
    if (bytes == 0) {
      break;
    }
    received += bytes;
  } while (received < total);

  if (received == total) error("ERROR storing complete response from socket");

  return response;
}
