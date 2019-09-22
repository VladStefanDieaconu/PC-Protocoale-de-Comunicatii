
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS	5
#define BUFLEN 256

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    int sockfd, newsockfd, portno, clilen, count;
    char buffer[BUFLEN], v[3*MAX_CLIENTS];
    struct sockaddr_in serv_addr, cli_addr;
    int n, i, j;

    fd_set read_fds;	
    fd_set tmp_fds;	
    int fdmax;		

    if (argc < 2){
    	fprintf(stderr,"Usage : %s port\n", argv[0]);
        exit(1);
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
	}
	
    portno = atoi(argv[1]);

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0){
    	error("ERROR on binding");
	}

    listen(sockfd, MAX_CLIENTS);

    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;

	while (1) {
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1){
			error("ERROR in select");
		}
		
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)){
				if (i == sockfd) {
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					}else{
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) {
							fdmax = newsockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}else{
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0){
						if (n == 0){
							printf("selectserver: socket %d hung up\n", i);
						}else{
							error("ERROR in recv");
						}
						
						close(i);
						FD_CLR(i, &read_fds);
					}else{
						if (!strcmp(buffer, "list\n")){
							count = 0;
							for (j = 0; j <= fdmax; j++){
								if (FD_ISSET(j, &read_fds) && j != i && j != sockfd){
									v[count] = j + '0';
									v[count+1] = ' ';
									count += 2;
								}
							}
							send(i, v, count, 0);
						}else{
					    	if (buffer[0] <= '9' && buffer[0] >= '0'){
								send(buffer[0] - '0', buffer, sizeof(buffer), 0);
							}else{
								if (!strcmp(buffer, "quit\n")){
									FD_CLR(i, &read_fds);
								}
							}
						}		
						printf ("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, buffer);
					}
				}
			}
		}
     }

     close(sockfd);

     return 0;
}


