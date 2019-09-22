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
#include "requests.h"
#include <cstring>
#include <iostream>
#include <json-c/json.h>
#include <sys/types.h>

using namespace std;

// functie ca sa scot cookies din mesaj-ul primit
string get_cookie(char *copy_of_resp) {
  string cookie;
  char *token = strtok(copy_of_resp, ";");
  while (token != NULL) {
    // aici caut cookie-ul care incepe cu prajiturica
    char *aux1 = strstr(token, "prajiturica=");
    if (aux1) {
      string aux_string(aux1);
      cookie = cookie + aux_string + ";";
    }
    // aici caut cookie-ul care incepe cu prajiturica.sig
    char *aux2 = strstr(token, "prajiturica.sig");
    if (aux2) {
      string aux_string(aux2);
      cookie = cookie + aux_string + "; ";
    }
    token = strtok(NULL, ";");
  }
  return cookie;
}

// Ca in laboratorul 9 de DNS, primesc un nume si returnez IP
char *get_ip(char *name) {
  struct addrinfo hints;
  struct addrinfo *result;
  struct addrinfo *ptr;
  struct sockaddr_in *address;
  char *ip_translated;

  // setarile clasice pentru protocoalele de internet
  hints.ai_family = AF_INET;
  hints.ai_protocol = IPPROTO_TCP;  // aleg TCP
  hints.ai_socktype = 0;
  hints.ai_flags = AI_CANONNAME;

  // pun aici marimea, ca sa fie mai usor de citit codul
  size_t size_of_addrinfo = sizeof(struct addrinfo);

  memset(&hints, 0, size_of_addrinfo);
  getaddrinfo(name, nullptr, &hints, &result);

  // cauta adresa ta IP
  for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
    // daca a gasit-o, dau return adresei IP
    if (ptr->ai_family == AF_INET) {
      ip_translated = (char *)calloc(INET_ADDRSTRLEN, sizeof(char));
      address = (struct sockaddr_in *)ptr->ai_addr;
      inet_ntop(ptr->ai_family, &address->sin_addr, ip_translated,
                INET_ADDRSTRLEN);
      return ip_translated;
    }
  }
  return nullptr;
}

int main(int argc, char *argv[]) {
  int i;
  char *message;
  char *response;
  char delim[] = "{\"enunt\"";
  int sockfd;

  // obiecte in care voi memora ce scot in json
  struct json_object *parsed_json;
  struct json_object *enunt_parsat;
  struct json_object *url_parsat;
  struct json_object *method_parsat;
  struct json_object *type_parsat;
  struct json_object *data_parsat;
  struct json_object *user_parsat;
  struct json_object *pass_parsat;
  struct json_object *token_parsat;
  struct json_object *query_param_parsat;
  struct json_object *id_parsat;
  struct json_object *url_data_parsat;
  struct json_object *city_parsat;
  struct json_object *appid_parsat;

  // ========= ETAPA 1 =========

  // lucruri pe care le fac ca in lab 10, ca sa trimit request-ul si sa primesc
  // raspunsul cu datele de care am nevoie
  sockfd =
      open_connection((char *)"185.118.200.35", 8081, AF_INET, SOCK_STREAM, 0);
  message = compute_get_request((char *)"185.118.200.35",
                                (char *)"/task1/start", NULL);
  send_to_server(sockfd, message);
  response = receive_from_server(sockfd);
  close_connection(sockfd);

  // ========= ETAPA 2 =========

  // fac o copie, in caz ca mai am nevoie de response
  char *copy_of_resp = strdup(response);

  // scot inceputul jsonului din text
  char *json_extracted = strstr(response, delim);

  // obtin jsonul din text
  parsed_json = json_tokener_parse(json_extracted);

  // parsez si extrag obiecte pt fiecare membru
  json_object_object_get_ex(parsed_json, "enunt", &enunt_parsat);
  json_object_object_get_ex(parsed_json, "url", &url_parsat);
  json_object_object_get_ex(parsed_json, "method", &method_parsat);
  json_object_object_get_ex(parsed_json, "type", &type_parsat);
  json_object_object_get_ex(parsed_json, "data", &data_parsat);

  // scot username-ul si parola din data
  json_object_object_get_ex(data_parsat, "username", &user_parsat);
  json_object_object_get_ex(data_parsat, "password", &pass_parsat);

  string data_login =
      "username=" + (string)json_object_get_string(user_parsat) + "&password=" +
      (string)json_object_get_string(pass_parsat);

  // scot cookie-ul meu
  string cookie = get_cookie(copy_of_resp);

  // construires cererea folosind post
  char *post_mes = compute_post_request(
      (char *)"185.118.200.35", (char *)json_object_get_string(url_parsat),
      (char *)cookie.c_str(), (char *)data_login.c_str());

  // deschid conexiunea si trimit get-ul
  sockfd =
      open_connection((char *)"185.118.200.35", 8081, AF_INET, SOCK_STREAM, 0);
  send_to_server(sockfd, post_mes);
  response = receive_from_server(sockfd);
  close_connection(sockfd);

  // ========= ETAPA 3 =========
  string raspuns1 = "omul";
  string raspuns2 = "numele";

  // fac o copie, in caz ca mai am nevoie de response
  copy_of_resp = strdup(response);

  // scot inceputul jsonului din text
  json_extracted = strstr(response, delim);

  // obtin jsonul din text
  parsed_json = json_tokener_parse(json_extracted);

  // parsez si extrag obiecte pt fiecare membru
  json_object_object_get_ex(parsed_json, "enunt", &enunt_parsat);
  json_object_object_get_ex(parsed_json, "url", &url_parsat);
  json_object_object_get_ex(parsed_json, "method", &method_parsat);
  json_object_object_get_ex(parsed_json, "data", &data_parsat);

  // extrag continutul data
  json_object_object_get_ex(data_parsat, "token", &token_parsat);
  json_object_object_get_ex(data_parsat, "queryParams", &query_param_parsat);

  // extrag continutul query params
  json_object_object_get_ex(query_param_parsat, "id", &id_parsat);

  string new_url = (string)json_object_get_string(url_parsat) + "?" +
                   "raspuns1=" + raspuns1 + "&" + "raspuns2=" + raspuns2 +
                   "&id=" + (string)json_object_get_string(id_parsat);

  // scot cookie-ul meu
  cookie = get_cookie(copy_of_resp);

  // construiesc data-ul necesar pt login (concatenarea headerului cu tokenul)
  data_login = string();
  data_login =
      "Authorization: Bearer " + (string)json_object_get_string(token_parsat);

  // construiesc cererea
  post_mes = compute_get_request_aux(
      (char *)"185.118.200.35", (char *)new_url.c_str(), (char *)cookie.c_str(),
      (char *)data_login.c_str());

  // deschid conexiunea si trimit get-ul
  sockfd =
      open_connection((char *)"185.118.200.35", 8081, AF_INET, SOCK_STREAM, 0);
  send_to_server(sockfd, post_mes);
  response = receive_from_server(sockfd);
  close_connection(sockfd);

  // ========= ETAPA 4 =========

  // fac o copie, in caz ca mai am nevoie de response
  copy_of_resp = strdup(response);

  // scot inceputul jsonului din text
  json_extracted = strstr(response, delim);

  // obtin jsonul din text
  parsed_json = json_tokener_parse(json_extracted);

  // parsez si extrag obiecte pt fiecare membru
  json_object_object_get_ex(parsed_json, "enunt", &enunt_parsat);
  json_object_object_get_ex(parsed_json, "url", &url_parsat);
  json_object_object_get_ex(parsed_json, "method", &method_parsat);
  json_object_object_get_ex(parsed_json, "data", &data_parsat);

  // scot cookie-ul meu
  cookie = get_cookie(copy_of_resp);

  // construires cererea
  post_mes = compute_get_request_aux(
      (char *)"185.118.200.35", (char *)json_object_get_string(url_parsat),
      (char *)cookie.c_str(), (char *)data_login.c_str());

  // deschid conexiunea si trimit get-ul
  sockfd =
      open_connection((char *)"185.118.200.35", 8081, AF_INET, SOCK_STREAM, 0);
  send_to_server(sockfd, post_mes);
  response = receive_from_server(sockfd);
  close_connection(sockfd);

  // ========= ETAPA 5 =========

  // fac o copie, in caz ca mai am nevoie de response
  copy_of_resp = strdup(response);

  // scot inceputul jsonului din text
  json_extracted = strstr(response, delim);

  // obtin jsonul din text
  parsed_json = json_tokener_parse(json_extracted);

  // parsez si extrag obiecte pt fiecare membru
  json_object_object_get_ex(parsed_json, "enunt", &enunt_parsat);
  json_object_object_get_ex(parsed_json, "url", &url_parsat);
  json_object_object_get_ex(parsed_json, "method", &method_parsat);
  json_object_object_get_ex(parsed_json, "type", &type_parsat);
  ;
  json_object_object_get_ex(parsed_json, "data", &data_parsat);

  // parsam si ce contine data
  json_object_object_get_ex(data_parsat, "url", &url_data_parsat);
  json_object_object_get_ex(data_parsat, "queryParams", &query_param_parsat);

  // extrag continutul query params
  json_object_object_get_ex(query_param_parsat, "q", &city_parsat);
  json_object_object_get_ex(query_param_parsat, "APPID", &appid_parsat);

  // scot cookie-ul meu
  cookie = get_cookie(copy_of_resp);

  // fac o copie a url-ului parsat, ca sa pot scoate doar domeniul din el
  char *copy_of_url = strdup(json_object_get_string(url_data_parsat));
  char *domain_of_site = strtok(copy_of_url, "/");

  // aici folosesc DNS ca sa obtin ip-ul stiind domeniul site-ului
  char *ip_from_url = get_ip(domain_of_site);

  // valoare auxiliara folosita la parsarea cu strtok a restului de url
  char *pch = domain_of_site;
  new_url = string();

  // extrag restul de chestii din url
  new_url = (string)strstr(json_object_get_string(url_data_parsat), "/");

  // adaug data la URL, respect formatul indicat in enunt
  new_url = new_url + "?q=" + (string)json_object_get_string(city_parsat) +
            "&APPID=" + (string)json_object_get_string(appid_parsat);

  data_login = string();
  data_login =
      "Authorization: Bearer " + (string)json_object_get_string(token_parsat);

  // construires cererea
  post_mes = compute_get_request_aux(ip_from_url, (char *)new_url.c_str(),
                                        (char *)cookie.c_str(),
                                        (char *)data_login.c_str());

  // deschid conexiunea si trimit get-ul
  sockfd = open_connection(ip_from_url, 80, AF_INET, SOCK_STREAM, 0);
  send_to_server(sockfd, post_mes);
  response = receive_from_server(sockfd);
  close_connection(sockfd);

  // acum in response am tot raspunsul de la server, vreau sa scot doar jsonul
  // din el
  // fac o copie, in caz ca mai am nevoie de response
  copy_of_resp = strdup(response);

  // scot inceputul jsonului din text
  json_extracted = strstr(response, "{");

  // construires cererea folosind post
  post_mes = compute_post_request_aux(
      (char *)"185.118.200.35", (char *)json_object_get_string(url_parsat),
      (char *)json_object_get_string(type_parsat), (char *)cookie.c_str(),
      (char *)data_login.c_str(), json_extracted);

  // deschid conexiunea si trimit get-ul
  sockfd =
      open_connection((char *)"185.118.200.35", 8081, AF_INET, SOCK_STREAM, 0);
  send_to_server(sockfd, post_mes);
  response = receive_from_server(sockfd);
  close_connection(sockfd);

  // iau raspunsul de la server si-l afisez
  puts(response);

  // eliberez memoria si inchid socket-ul
  free(response);
  free(post_mes);
  close_connection(sockfd);
  return 0;
}
