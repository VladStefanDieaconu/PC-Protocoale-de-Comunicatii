#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc,char** argv){
  init(HOST,PORT);
  msg t;

  sprintf(t.payload,"Hello World of PC");
  t.len = strlen(t.payload)+1;
  send_message(&t);

  if (recv_message(&t)<0){
    perror("receive error");
  }
  else {
    printf("[%s] Got reply with payload: %s\n",argv[0],t.payload);
  }

  return 0;
}
