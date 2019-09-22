// Vlad-Stefan Dieaconu 321CA
#ifndef _REQUESTS_
#define _REQUESTS_

char *compute_get_request(char *host, char *url, char *url_params);
char *compute_post_request(char *host, char *url, char *cookie,
                                 char *form_data);
char *compute_get_request_aux(char *host, char *url, char *cookie,
                                 char *form_data);
char *compute_post_request_aux(char *host, char *url, char *content_type,
                                 char *cookie, char *token, char *form_data);

#endif
