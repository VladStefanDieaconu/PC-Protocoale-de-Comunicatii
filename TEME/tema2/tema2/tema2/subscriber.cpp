// (c) Vlad-Stefan Dieaconu 2019
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
extern "C" {
#include "helpers.h"
}

// folosit pentru programare defensiva
void usage(char *file) {
  fprintf(stderr, "Usage: %s <ID_CLIENT> <IP_Server> <Port_Server>\n", file);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockfd;
  int n;
  int ret;
  struct sockaddr_in serv_addr;
  char buffer[BUFLEN];
  fd_set read_fds;
  fd_set tmp_fds;
  int fdmax;

  // programare defensiva: ne asiguram ca avem cate argumente trebuie
  if (argc < 4) {
    usage(argv[0]);
  }

  // programare defensiva: ne asiguram ca ID-ul clientului e scris corect
  if (strlen(argv[1]) != 10) {
    fprintf(stderr, "Client ID must have 10 characters\n");
    exit(0);
  }

  // se goleste multimea de descriptori
  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  // initializez socketul tcp
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[3]));
  ret = inet_aton(argv[2], &serv_addr.sin_addr);
  DIE(ret == 0, "inet_aton");

  ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(ret < 0, "connect");

  FD_SET(STDIN_FILENO, &read_fds);
  FD_SET(sockfd, &read_fds);
  fdmax = sockfd;

  // trimit client id-ul catre server ca mesaj TCP
  n = send(sockfd, argv[1], strlen(argv[1]), 0);
  DIE(n < 0, "send");

  while (1) {
    tmp_fds = read_fds;
    ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
    DIE(ret < 0, "select");

    if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
      // se citeste de la tastatura
      memset(buffer, 0, BUFLEN);
      fgets(buffer, BUFLEN - 1, stdin);

      // daca clientul primeste comanda de exit ne deconectam
      if (strncmp(buffer, "exit", 4) == 0) {
        break;
      }
      // fac o copie a lui buffer
      char *buffer_copy = strdup(buffer);

      // daca nu dam break aici, cautam sa vedem daca nu cumva trebuie sa ne
      // abonam
      // gen daca e subscribe sau unsub. Folosim STR TOK
      char *pch;
      pch = strtok(buffer, " ");

      if (strncmp(pch, "subscribe", 9) == 0) {
        pch = strtok(NULL, " ");  // pch are acum valoarea topicului

        if (pch == NULL) {  // daca nu exista parametrul, aruncam 0 eroare
          fprintf(stderr, "Topic-ul lipseste\n");
          exit(0);
        }

        pch = strtok(NULL, " ");  // pch are acum valoarea lui SF(0 sau 1)
        if (pch == NULL) {        // daca nu exista parametrul, aruncam eroare
          fprintf(stderr, "Valorea lui SF (0 sau 1) lipseste\n");
          exit(0);
        }

        int value_of_SF = atoi(pch);
        if (!(value_of_SF == 0 ||
              value_of_SF == 1)) {  // daca exista parametrul dar nu e 0 sau 1
          fprintf(stderr, "Valorea lui SF trebuie sa fie 0 sau 1\n");
          exit(0);
        }
      } else if (strncmp(pch, "unsubscribe", 11) == 0) {
        pch = strtok(NULL, " ");  // pch e acum topicul
        if (pch == NULL) {        // daca nu exista param aruncam eroare
          fprintf(stderr, "Topic-ul lipseste\n");
          exit(0);
        }
      } else {  // daca nu e niciuna dintre comenzile astea, aruncam eroare
        fprintf(stderr, "Accepted commands: subscribe / unsubscribe, exit\n");
        exit(0);
      }

      // se trimite mesaj la server cu comanda, ca sa verifice daca
      // comanda e valida, adica daca exista topicul respectiv si nu cumva
      // esti deja abonat la el
      n = send(sockfd, buffer_copy, strlen(buffer_copy), 0);
      DIE(n < 0, "send");

      // nu mai am nevoie de copie, eliberez memoria
      free(buffer_copy);
    }

    if (FD_ISSET(sockfd, &tmp_fds)) {
      memset(buffer, 0, BUFLEN);
      n = recv(sockfd, buffer, sizeof(buffer), 0);
      if (n == 0) {
        break;
      }
      DIE(n < 0, "recv");
      printf("%s\n", buffer);
    }
  }
  close(sockfd);
  return 0;
}
