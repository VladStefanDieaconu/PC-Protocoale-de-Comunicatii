// (c) Vlad-Stefan Dieaconu 2019
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
extern "C" {
#include "helpers.h"
}
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <sstream>
#include <math.h>

using namespace std;

struct Subscription {
  string topic;
  bool value_of_SF;
};

void usage(char *file) {
  fprintf(stderr, "Usage: %s server_port\n", file);
  exit(0);
}

// folosit pentru a nu avea cod duplicat cand parsez pentru cazul 0 si cazul 2
int32_t parsed_buffer(char buffer[]) {
  int buff1 = (int8_t)buffer[52] << 24;
  int buff2 = (int8_t)buffer[53] << 16;
  int buff3 = (int8_t)buffer[54] << 8;
  int buff4 = (int8_t)buffer[55];
  return buff1 + buff2 + buff3 + buff4;
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno;
  int fd_udp;
  struct sockaddr_in serv_addr, cli_addr;
  map<int, string> socket_to_id;
  map<string, vector<Subscription>> client_to_subscription;
  map<string, string> clients_messages;
  map<int, string> datatype_to_string;
  map<string, bool> is_client_connected;

  // TCP NO DELAY
  int flag_delay = 1;
  int tcp_no_delay =
      setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag_delay, sizeof(int));

  char buffer[BUFLEN];
  int n, i, ret;
  socklen_t clilen;
  int sockets[10];
  int nr_socks = 0;

  fd_set read_fds;  // multimea de citire folosita in select()
  fd_set tmp_fds;   // multime folosita temporar
  fd_set tmp2_fds;
  int fdmax;  // valoare maxima fd din multimea read_fds

  if (argc < 2) {
    usage(argv[0]);
  }

  // se goleste multimea de descriptori de citire (read_fds) si multimea
  // temporara (tmp_fds)
  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  fd_udp = socket(PF_INET, SOCK_DGRAM, 0);
  DIE(fd_udp < 0, "udp");

  portno = atoi(argv[1]);
  DIE(portno == 0, "atoi");

  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  cli_addr.sin_family = AF_INET;
  cli_addr.sin_addr.s_addr = INADDR_ANY;
  cli_addr.sin_port = htons(portno);

  ret = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
  DIE(ret < 0, "bind");

  ret = bind(fd_udp, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr));
  DIE(ret < 0, "bind");

  ret = listen(sockfd, MAX_CLIENTS);
  DIE(ret < 0, "listen");

  FD_SET(STDIN_FILENO, &read_fds);

  // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in
  // multimea read_fds
  FD_SET(sockfd, &read_fds);
  FD_SET(fd_udp, &read_fds);

  // fdmax este maximul dintre sock de tcp si udp
  fdmax = max(sockfd, fd_udp);

  while (1) {
    bool ok_exit = false;
    tmp_fds = read_fds;
    tmp2_fds = read_fds;

    ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
    DIE(ret < 0, "select");

    for (i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &tmp_fds)) {
        if (i == STDIN_FILENO) {
          scanf("%s", buffer);
          if (!strcmp(buffer, "STOP")) {
            goto end;
          }
        } else if (i == fd_udp) {
          memset(buffer, 0, BUFLEN);
          n = recv(i, buffer, BUFLEN, 0);
          DIE(n < 0, "recv");

          // facem parsarea
          // in topic_p retin topicul parsat
          char topic_p[BUFLEN];
          memset(topic_p, 0, BUFLEN);
          strncpy(topic_p, buffer, 49);

          // retin tipul de data, poate fi 0, 1, 2, sau 3
          int tip_data;
          tip_data = (int8_t)buffer[50];

          // retin continutul mesajului propriu zis
          char continut[1500];
          memset(continut, 0, 1499);
          strncpy(continut, buffer + 51, 1499);

          // in functie de valoarea lui tip_data, transformam continutul
          // imi iau un map in care retin ce string trebuie afisat pt fiecare
          // data type
          switch (tip_data) {
            case 0: {
              datatype_to_string.insert(pair<int, string>(0, "INT"));
              int32_t continut_parsat = parsed_buffer(buffer);
              long long valoare_continut;

              if (buffer[51] == 0) {
                valoare_continut = (long long)continut_parsat;
              } else {
                valoare_continut = (long long)continut_parsat * (-1);
              }

              // pun IP-ul udp-ului, portul si mesajul, apoi trimit
              stringstream ss;
              ss << inet_ntoa(cli_addr.sin_addr) << ":"
                 << htons(cli_addr.sin_port);
              string udp_ip_port = ss.str();

              // golesc stream-ul (de fapt dau swpa cu un constructor)
              std::stringstream().swap(ss);

              ss << " - " << topic_p << " - " << datatype_to_string.at(0)
                 << " - " << valoare_continut;
              string string_de_afisat = udp_ip_port + ss.str();
              int size = string_de_afisat.size();

              for (int cl_sock = 0; cl_sock < nr_socks; cl_sock++) {
                string client_id = socket_to_id.find(sockets[cl_sock])->second;
                vector<Subscription> subscribtions =
                    client_to_subscription.find(client_id)->second;
                if (clients_messages.at(client_id).size() > 0) {
                  n = send(sockets[cl_sock],
                           clients_messages.at(client_id).c_str(),
                           (int)clients_messages.at(client_id).size(), 0);
                  DIE(n < 0, "send");
                  clients_messages.at(client_id) = string();
                }
                if (sockets[cl_sock] != -1) {
                  for (const auto &subbed : subscribtions) {
                    if (strncmp(topic_p, subbed.topic.c_str(),
                                strlen(topic_p)) == 0) {
                      n = send(sockets[cl_sock], string_de_afisat.c_str(), size,
                               0);
                      DIE(n < 0, "send");
                    }
                  }
                }
                if (is_client_connected.at(client_id) == false) {
                  for (const auto &subbed : subscribtions) {
                    if (subbed.value_of_SF == 1) {
                      if (strncmp(topic_p, subbed.topic.c_str(),
                                  strlen(topic_p)) == 0) {
                        clients_messages.at(client_id) += string_de_afisat;
                      }
                    }
                  }
                }
              }
            } break;

            case 1: {
              datatype_to_string.insert(pair<int, string>(1, "SHORT_REAL"));
              int16_t continut_parsat =
                  ((int8_t)buffer[51] << 8) + (int8_t)buffer[52];
              float valoare_continut = (float)continut_parsat / 100;

              // pun IP-ul udp-ului, portul si mesajul, apoi trimit
              stringstream ss;
              ss << inet_ntoa(cli_addr.sin_addr) << ":"
                 << htons(cli_addr.sin_port);
              string udp_ip_port = ss.str();

              // golesc stream-ul (de fapt dau swpa cu un constructor)
              std::stringstream().swap(ss);
              ss << " - " << topic_p << " - " << datatype_to_string.at(1)
                 << " - " << valoare_continut;
              string string_de_afisat = udp_ip_port + ss.str();
              int size = string_de_afisat.size();

              for (int cl_sock = 0; cl_sock < nr_socks; cl_sock++) {
                string client_id = socket_to_id.find(sockets[cl_sock])->second;
                vector<Subscription> subscribtions =
                    client_to_subscription.find(client_id)->second;
                if (clients_messages.at(client_id).size() > 0) {
                  n = send(sockets[cl_sock],
                           clients_messages.at(client_id).c_str(),
                           (int)clients_messages.at(client_id).size(), 0);
                  DIE(n < 0, "send");
                  clients_messages.at(client_id) = string();
                }
                if (sockets[cl_sock] != -1) {
                  for (const auto &subbed : subscribtions) {
                    if (strncmp(topic_p, subbed.topic.c_str(),
                                strlen(topic_p)) == 0) {
                      n = send(sockets[cl_sock], string_de_afisat.c_str(), size,
                               0);
                      DIE(n < 0, "send");
                    }
                  }
                }
                if (is_client_connected.at(client_id) == false) {
                  for (const auto &subbed : subscribtions) {
                    if (subbed.value_of_SF == 1) {
                      if (strncmp(topic_p, subbed.topic.c_str(),
                                  strlen(topic_p)) == 0) {
                        clients_messages.at(client_id) += string_de_afisat;
                      }
                    }
                  }
                }
              }
            } break;
            case 2: {
              datatype_to_string.insert(pair<int, string>(2, "FLOAT"));
              int32_t continut_parsat = parsed_buffer(buffer);
              float valoare_continut =
                  ((float)continut_parsat) /
                  (float)((int32_t)pow(10, (int8_t)buffer[56]));

              // pun IP-ul udp-ului, portul si mesajul, apoi trimit
              stringstream ss;
              ss << inet_ntoa(cli_addr.sin_addr) << ":"
                 << htons(cli_addr.sin_port);
              string udp_ip_port = ss.str();

              // golesc stream-ul (de fapt dau swpa cu un constructor)
              std::stringstream().swap(ss);
              ss << " - " << topic_p << " - " << datatype_to_string.at(2)
                 << " - " << valoare_continut;
              string string_de_afisat = udp_ip_port + ss.str();
              int size = string_de_afisat.size();

              for (int cl_sock = 0; cl_sock < nr_socks; cl_sock++) {
                string client_id = socket_to_id.find(sockets[cl_sock])->second;
                vector<Subscription> subscribtions =
                    client_to_subscription.find(client_id)->second;
                if (clients_messages.at(client_id).size() > 0) {
                  n = send(sockets[cl_sock],
                           clients_messages.at(client_id).c_str(),
                           (int)clients_messages.at(client_id).size(), 0);
                  DIE(n < 0, "send");
                  clients_messages.at(client_id) = string();
                }
                if (sockets[cl_sock] != -1) {
                  for (const auto &subbed : subscribtions) {
                    if (strncmp(topic_p, subbed.topic.c_str(),
                                strlen(topic_p)) == 0) {
                      n = send(sockets[cl_sock], string_de_afisat.c_str(), size,
                               0);
                      DIE(n < 0, "send");
                    }
                  }
                }
                if (is_client_connected.at(client_id) == false) {
                  for (const auto &subbed : subscribtions) {
                    if (subbed.value_of_SF == 1) {
                      if (strncmp(topic_p, subbed.topic.c_str(),
                                  strlen(topic_p)) == 0) {
                        clients_messages.at(client_id) += string_de_afisat;
                      }
                    }
                  }
                }
              }
            } break;
            case 3: {
              datatype_to_string.insert(pair<int, string>(3, "STRING"));
              char *valoare_continut = strdup(continut);

              // pun IP-ul udp-ului, portul si mesajul, apoi trimit
              stringstream ss;
              ss << inet_ntoa(cli_addr.sin_addr) << ":"
                 << htons(cli_addr.sin_port);
              string udp_ip_port = ss.str();

              // golesc stream-ul (de fapt dau swpa cu un constructor)
              std::stringstream().swap(ss);
              ss << " - " << topic_p << " - " << datatype_to_string.at(3)
                 << " - " << valoare_continut;
              string string_de_afisat = udp_ip_port + ss.str();
              int size = string_de_afisat.size();

              for (int cl_sock = 0; cl_sock < nr_socks; cl_sock++) {
                string client_id = socket_to_id.find(sockets[cl_sock])->second;
                vector<Subscription> subscribtions =
                    client_to_subscription.find(client_id)->second;
                if (clients_messages.at(client_id).size() > 0) {
                  n = send(sockets[cl_sock],
                           clients_messages.at(client_id).c_str(),
                           (int)clients_messages.at(client_id).size(), 0);
                  DIE(n < 0, "send");
                  clients_messages.at(client_id) = string();
                }
                if (sockets[cl_sock] != -1) {
                  for (const auto &subbed : subscribtions) {
                    if (strncmp(topic_p, subbed.topic.c_str(),
                                strlen(topic_p)) == 0) {
                      n = send(sockets[cl_sock], string_de_afisat.c_str(), size,
                               0);
                      DIE(n < 0, "send");
                    }
                  }
                }
                if (is_client_connected.at(client_id) == false) {
                  for (const auto &subbed : subscribtions) {
                    if (subbed.value_of_SF == 1) {
                      if (strncmp(topic_p, subbed.topic.c_str(),
                                  strlen(topic_p)) == 0) {
                        clients_messages.at(client_id) += string_de_afisat;
                      }
                    }
                  }
                }
              }
            } break;
            default:
              fprintf(stderr, "Valoarea lui tip_data nu este corecta\n");
          }

        } else if (i == sockfd) {
          // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
          // pe care serverul o accepta
          clilen = sizeof(cli_addr);
          newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
          DIE(newsockfd < 0, "accept");

          // se adauga noul socket intors de accept() la multimea descriptorilor
          // de citire
          FD_SET(newsockfd, &read_fds);
          if (newsockfd > fdmax) {
            fdmax = newsockfd;
          }

          sockets[nr_socks] = newsockfd;
          nr_socks++;

          memset(buffer, 0, BUFLEN);
          n = recv(newsockfd, buffer, BUFLEN, 0);
          DIE(n < 0, "recv");
          printf("New client (%s) connected from %s:%d.\n", buffer,
                 inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

          // creez primul obiect de tip subscription pentru client
          socket_to_id.insert(pair<int, string>(newsockfd, buffer));
          vector<Subscription> aux;
          client_to_subscription.insert(
              pair<string, vector<Subscription>>(buffer, aux));

          // memorez clientul ca avand starea activa
          string client_id = socket_to_id.find(newsockfd)->second;
          if (is_client_connected.find(client_id) !=
              is_client_connected.end()) {
            is_client_connected.at(client_id) = true;
          } else {
            is_client_connected.insert(pair<string, bool>(client_id, true));
            clients_messages.insert(pair<string, string>(client_id, string()));
          }

        } else {
          // s-au primit date pe unul din socketii de client,
          // asa ca serverul trebuie sa le receptioneze
          memset(buffer, 0, BUFLEN);
          n = recv(i, buffer, sizeof(buffer), 0);
          DIE(n < 0, "recv");

          if (n == 0) {
            string client_id = socket_to_id.find(i)->second;
            // conexiunea s-a inchis, afisam ce e nevoie
            printf("Client (%s) disconnected.\n", client_id.c_str());

            // il marchez ca fiind deconectat
            is_client_connected.at(client_id) = false;
            close(i);

            for (int j = 0; j < nr_socks; j++) {
              if (sockets[j] == i) {
                sockets[j] = -1;
              }
            }
            // se scoate din multimea de citire socketul inchis
            FD_CLR(i, &read_fds);
          } else {
            // tokenizam bufferul primit, ca sa vedem ce e in el
            // nu mai aruncam erori aici, pt ca daca cumva ce trimite clientul
            // nu e bun
            // se va inchide clientul, nu vor ajunge niciodata la server. Datele
            // primite
            // aici sunt mereu corecte deci le luam de bune. Verific doar daca
            // exista
            // acele topicuri

            string client_id = socket_to_id.find(i)->second;
            Subscription new_subscription;

            char *pch;
            pch = strtok(buffer, " ");
            if (strncmp(pch, "subscribe", 9) == 0) {
              pch = strtok(NULL, " ");  // pch are acum valoarea topicului
              new_subscription.topic = pch;
              pch = strtok(NULL, " ");  // pch are acum valoarea lui SF(0 sau 1)
              bool value_of_SF = atoi(pch);
              new_subscription.value_of_SF = value_of_SF;
              vector<Subscription> aux_vec =
                  client_to_subscription.find(client_id)->second;
              aux_vec.push_back(new_subscription);
              client_to_subscription.at(client_id) = aux_vec;
            }

            if (strncmp(pch, "unsubscribe", 11) == 0) {
              pch = strtok(NULL, " ");  // pch e acum topicul
              bool topic_invalid = false;

              vector<Subscription> subscribtions =
                  client_to_subscription.find(client_id)->second;
              for (const auto &subbed : subscribtions) {
                if (strncmp(pch, subbed.topic.c_str(), strlen(pch)) == 0) {
                  topic_invalid = true;
                }
              }

              if (topic_invalid == true) {
                fprintf(stderr, "Topic inexistent\n");
                exit(0);
              }

              vector<Subscription> aux_vec =
                  client_to_subscription.find(client_id)->second;
              bool found_topic = false;
              int index = 0;

              // cautam topicul ca sa il stergem
              for (auto &elem : aux_vec) {
                index++;
                if (strncmp(elem.topic.c_str(), pch, elem.topic.size()) == 0) {
                  found_topic = true;
                  break;
                }
              }

              // daca gasim topicul, il stergem
              if (found_topic == true) {
                aux_vec.erase(aux_vec.begin() + index);
              } else {
                fprintf(stderr, "Clientul nu e abonat la acest topic\n");
                exit(0);
              }
            }
          }
        }
      }

      if (FD_ISSET(0, &tmp_fds)) {
        // se citeste de la tastatura, daca cumva primim comanda EXIT in server
        // inchidem toti clientii
        // setbuf(stdin, NULL);
        memset(buffer, 0, BUFLEN);
        fgets(buffer, BUFLEN - 1, stdin);

        if (strncmp(buffer, "exit", 4) == 0) {
          ok_exit = true;
          break;
        } else {
          fprintf(stderr, "Serverul poate primi doar comanda exit\n");
          exit(0);
        }
      }
    }
    // daca s-a primit exit vreodata, trebuie inchisi clientii
    if (ok_exit == true) {
      break;
    }
  }
end:
  close(sockfd);
  close(sockfd);
  return 0;
}
