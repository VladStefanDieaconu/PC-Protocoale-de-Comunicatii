#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc,char** argv){
  msg r,t;
  init(HOST,PORT);

  if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
  }

  printf("[%s] Got msg with payload: %s\n",argv[0],r.payload);

  sprintf(t.payload,"ACK");
  t.len = strlen(t.payload+1);
  send_message(&t);
  return 0;
}
