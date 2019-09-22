// Copyright (C) 2019 Vlad-Stefan Dieaconu 321 CA
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
extern "C" {
#include "link_emulator/lib.h"
}
#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char** argv) {
    msg r, t;
    init((char*)HOST, PORT);
    // primesc numele fisierului
    if (recv_message(&r) < 0) {
        perror("Receive message");
        return -1;
    }
    // afisez la consola ce primesc
    printf("[%s] Got msg with nr_sec %d, parity %d, message %s and length %d\n",
        argv[0], atoi(r.payload), atoi(r.payload + 4), r.payload + 8, r.len);
    // deschidem fisier si construim denumirea fisierului dupa cum este
    // specificat in enunt
    char name[] = "recv_";
    strcat(name, r.payload + 2 * sizeof(int));
    int file = open(name, O_WRONLY | O_CREAT, 0644);
    sprintf(t.payload, "ACK");
    t.len = strlen(t.payload + 1);
    send_message(&t);

    // primesc lungimea fisierului
    if (recv_message(&r) < 0){
        perror("Receive message");
        return -1;
    }
    printf("[%s] Got msg with nr_sec %d, parity %d, message %d and length %d\n",
        argv[0], atoi(r.payload), atoi(r.payload + sizeof(int)),
        atoi(r.payload + 2 * sizeof(int)), r.len);
    sprintf(t.payload, "ACK");
    t.len = strlen(t.payload + 1);
    send_message(&t);

    // primesc countul, reprezentand in cate pachete este impartit fisierul meu
    if (recv_message(&r) < 0) {
        perror("Receive message");
        return -1;
    }
    printf("[%s] Got msg with nr_sec %d, parity %d, message %d and length %d\n",
        argv[0], atoi(r.payload), atoi(r.payload + sizeof(int)),
        atoi(r.payload + 2 * sizeof(int)), r.len);
    int COUNT = r.len;
    sprintf(t.payload,"ACK");
    t.len = strlen(t.payload + 1);
    send_message(&t);

    while (COUNT > 0) {
        recv_message(&r);
        sprintf(t.payload, "ACK");
        t.len = strlen(t.payload + 1);
        send_message(&t);
        write(file, r.payload + 2 * sizeof(int), r.len);
        COUNT--;
    }
    return 0;
}
