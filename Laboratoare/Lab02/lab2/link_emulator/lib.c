#include "lib.h"
#include <arpa/inet.h>
#include <poll.h>
//#include <stropts.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

struct sockaddr_in addr_local, addr_remote;
int s;
struct pollfd fds[1];

void set_local_port(int port)
{
  memset((char *) &addr_local, 0, sizeof(addr_local));
  addr_local.sin_family = AF_INET;
  addr_local.sin_port = htons(port);
  addr_local.sin_addr.s_addr = htonl(INADDR_ANY);
}

void set_remote(char* ip, int port){
  memset((char *) &addr_remote, 0, sizeof(addr_remote));
  addr_remote.sin_family = AF_INET;
  addr_remote.sin_port = htons(port);
  if (inet_aton(ip, &addr_remote.sin_addr)==0) {
    perror("inet_aton failed\n");
    exit(1);
  }
}

void init(char* remote, int REMOTE_PORT){
  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
    perror("Error creating socket");
    exit(1);
  }

  set_local_port(0);
  set_remote(remote,REMOTE_PORT);

  if (bind(s, (struct sockaddr*)&addr_local, sizeof(addr_local))==-1){
    perror("Failed to bind");
    exit(1);
  }

  fds[0].fd = s;
  fds[0].events = POLLIN;

  msg m;
  send_message(&m);
}

int send_message(const msg* m){
  return sendto(s, m, sizeof(msg), 0,(struct sockaddr*) &addr_remote, sizeof(addr_remote));
}

/*msg* receive_message(){
  msg* ret = (msg*)malloc(sizeof(msg));
  if (recvfrom(s, ret, sizeof(msg), 0, NULL, NULL)==-1){
    free(ret);
    return NULL;
  }
  return ret;
  }*/

int recv_message(msg* ret){
  return recvfrom(s, ret, sizeof(msg), 0, NULL, NULL);
}


//timeout in millis
/*msg* receive_message_timeout(int timeout){
  int ret = poll(fds,1,timeout);

  if (ret>0){
    if (fds[0].revents & POLLIN)
      return receive_message();
  }
  
  return NULL;
  }*/
