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

void transmit(char* file_name) {
  // Deschide fisierul pe care trebuie sa il transmit
  int fd = open(file_name, O_RDONLY);
  if (fd < 0) {
    perror("Missing file!\n");
    exit(1);
  }

  msg t;
  content c;

  // Send name
  c.type = HEADER_NAME;
  // Flush pe continut.
  memset(c.payload, 0, sizeof(c.payload));
  // Punem numele fisierului
  memcpy(c.payload, file_name, strlen(file_name));
  // 5 deoarece vin 4 de la type si 1 de la \0
  t.len = 5 + strlen(file_name);
  
  // Copiaza in msg toata structura content.
  memcpy(t.payload, &c, t.len);
    
  // Primul flux de mesaje - trimite si asteapta confimare (sau Etapa 1)
  send_message(&t);

  if (recv_message(&t) < 0) {
    perror("receive error");
  }

  printf("[./send] Got reply with payload: %s\n",t.payload);

  // Send size
  // Len este lungimea fiserului de transmis.
  int len = lseek(fd, 0, SEEK_END);

  c.type = HEADER_LENGTH;
  memset(c.payload, 0, sizeof(c.payload));
  memcpy(c.payload, &len, 4);
  // 4 de la type si 4 pt ca un int este pe 4 bytes si acum trimitem doar un int.
  t.len = 8;

  memcpy(t.payload, &c, t.len);
  
  // Etapa 2
  send_message(&t);

  if (recv_message(&t) < 0) {
    perror("receive error");
  }

  printf("[./send] Got reply with payload: %s\n",t.payload);

  // Send content
  lseek(fd, 0, SEEK_SET);
  memset(c.payload, 0, sizeof(c.payload));
  c.type = DATA;
  while ((len = read(fd, c.payload, sizeof(c.payload)))) {
    // Cat timp mai am de trimis date din acest fisier impacheteaza intr-un msg
    // si trimite-l la recv
    // len reprezinta cat s-a citit din fisier 
    t.len = 4 + len;
    memcpy(t.payload, &c, t.len);
    
    // Trimite continut si asteapta confimare pt acesta
    send_message(&t);

    if (recv_message(&t) < 0) {
      perror("receive error");
    }

    printf("[./send] Got reply with payload: %s\n",t.payload);
  }
}

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
  
  // Trasmisia propriu zisa a fiserelor
  if (argc <= 1) {
    printf("Need more args!\n");
  } else {
    int i;
    for (i = 1; i < argc; i++)
      transmit(argv[i]);

    // Send EOT - aici nu mai punem nimic in payload dearece ne intereaza doar tipul
    memset(&t, 0, sizeof(msg));
    int type = EOT;
    t.len = 4;
    memcpy(t.payload, &type, 4);
    send_message(&t);

  }
  return 0;
}
