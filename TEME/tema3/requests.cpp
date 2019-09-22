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

char *compute_get_request(char *host, char *url, char *url_params) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));
  if (url_params != NULL) {
    sprintf(line, "GET %s?%s HTTP/1.1", url, url_params);
  } else {
    sprintf(line, "GET %s HTTP/1.1", url);
  }
  compute_message(message, line);

  sprintf(line, "HOST: %s", host);
  compute_message(message, line);

  sprintf(line, "Cookie: %s=%s", "isLogged", "true");
  compute_message(message, line);

  memset(line, 0, LINELEN);
  compute_message(message, line);

  free(line);
  return message;
}

char *compute_post_request(char *host, char *url, char *cookie,
                                 char *form_data) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  sprintf(line, "HOST: %s", host);
  compute_message(message, line);

  strcpy(line, "Content-Type: application/x-www-form-urlencoded");
  compute_message(message, line);

  sprintf(line, "Content-Length: %ld", strlen(form_data));
  compute_message(message, line);

  sprintf(line, "Cookie: %s", cookie);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  compute_message(message, line);

  strcpy(line, form_data);
  compute_message(message, line);

  free(line);
  return message;
}

char *compute_get_request_aux(char *host, char *url, char *cookie,
                                char *form_data) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  sprintf(line, "GET %s HTTP/1.1", url);
  compute_message(message, line);
  sprintf(line, "HOST: %s", host);
  compute_message(message, line);

  sprintf(line, "Cookie: %s", cookie);
  compute_message(message, line);

  strcpy(line, form_data);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  compute_message(message, line);

  free(line);
  return message;
}

char *compute_post_request_aux(char *host, char *url, char *content_type,
                                 char *cookie, char *token, char *form_data) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  sprintf(line, "HOST: %s", host);
  compute_message(message, line);

  sprintf(line, "Content-Type: %s", content_type);
  compute_message(message, line);

  sprintf(line, "Content-Length: %ld", strlen(form_data));
  compute_message(message, line);

  sprintf(line, "Cookie: %s", cookie);
  compute_message(message, line);

  strcpy(line, token);
  compute_message(message, line);

  memset(line, 0, LINELEN);
  compute_message(message, line);

  strcpy(line, form_data);
  compute_message(message, line);

  free(line);
  return message;
}
