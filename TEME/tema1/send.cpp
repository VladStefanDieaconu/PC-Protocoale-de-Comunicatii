// Copyright (C) 2019 Vlad-Stefan Dieaconu 321 CA
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
extern "C" {
#include "link_emulator/lib.h"
}
#define HOST "127.0.0.1"
#define PORT 10000

// pachetul in care retin informatii importante, cum ar fi lungime, paritate etc
struct Package {
	int nr_secv;
	int parity;
	char message[MSGSIZE - 2 * sizeof(int)];
};

// functie in care calculez paritatea unui mesaj
int check_parity(Package p) {
    char result;
    for (int i = 0; i < strlen(p.message); ++i) {
    result ^= p.message[i]; 
    }
    return atoi(&result);
}

int main(int argc, char** argv){
    int file = open(argv[1], O_RDONLY);
    init((char*) HOST, PORT);
    msg t;

    // pachetul pe care il trimit
    Package package;

    // prima data trimit un pachet in care retin numele fisierului
    package.nr_secv = 0; // nr secventa 0, fiind primul pachet trimis
    // copiez in mesaj numele fisierului
    snprintf(package.message, strlen(argv[1]) + 1, "%s", argv[1]);
    // calculez paritatea mesajului
    package.parity = check_parity(package);
    // pun numarul secventei pe prima pozitie in payload
    snprintf(t.payload, sizeof(int), "%d", package.nr_secv);
    // pun paritatea pe a doua pozitie in payload
    snprintf(t.payload + sizeof(int), sizeof(int), "%d", package.parity);
    memcpy(t.payload + 2 * sizeof(int),  package.message,
    strlen(package.message) + 1);
    t.len = strlen(package.message);
    // trimit numele fisierului
    send_message(&t);

    // calculez dimensiunea fisierului pe care trebuie sa il trimit
    int len = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    // trec la urmatorul pachet, cu nr secventa 1: aici trimit dimensiunea
    // fisierului calculat mai sus
    package.nr_secv = 1;
    snprintf(package.message, sizeof(len), "%d", len);
    package.parity = check_parity(package);
    snprintf(t.payload, sizeof(int), "%d", package.nr_secv);
    snprintf(t.payload + sizeof(int), sizeof(int), "%d", package.parity);
    memcpy(t.payload + 2 * sizeof(int), package.message,
        strlen(package.message));
    t.len = len;
    send_message(&t);

    // calculez variabilele de care am nevoie pentru a trimite continutul
    // fisierului
    int FRAMESIZE = (int) MSGSIZE + sizeof(int);
    unsigned int bdp = atoi(argv[2]) * atoi(argv[3]) * 1000;
    int win_size = bdp / (FRAMESIZE * 7);
    // calculez count si tin cont de faptul ca trimis deja 3 pachete mai sus si
    // urmeaza sa mai trimit un pachet imediat sub (acest pachet fiind valoarea
    // lui count pe care o calculez acum)
    int COUNT = len / (MSGSIZE - 2 * sizeof(int)) + sizeof(int);

    // trimit valoarea lui count
    package.nr_secv = 2;
    snprintf(package.message, sizeof(COUNT), "%d", COUNT);
    package.parity = check_parity(package);
    snprintf(t.payload, sizeof(int), "%d", package.nr_secv);
    snprintf(t.payload + sizeof(int), sizeof(int), "%d", package.parity);
    memcpy(t.payload + 2 * sizeof(int), package.message,
        strlen(package.message));
    t.len = COUNT;
    send_message(&t);

    // tin cont de faptul ca am trimis deja 3 pachete, deci voi indexa acum
    // incepand de la 2 (pentru ca 0, 1 si 2 au fost deja trimise)
    int counter = 2;
    for (int i = 0; i < win_size; ++i) {
        int lungime_mesaj = read(file, package.message,
            (MSGSIZE - 2 * sizeof(int)));
        if (lungime_mesaj > 0) {
            // trimis continutul fisierului
            snprintf(t.payload, sizeof(int), "%d", counter++);
            memcpy(t.payload + 2 * sizeof(int), package.message, lungime_mesaj);
            package.parity = check_parity(package);
            snprintf(t.payload + sizeof(int), sizeof(int),
                "%d", package.parity);
            t.len = lungime_mesaj;
            send_message(&t);
        }
    }

    for (int i = 0; i < COUNT - win_size; ++i) {
        int lungime_mesaj = read(file, package.message,
            (MSGSIZE - 2 * sizeof(int)));
        if (lungime_mesaj > 0) {
            snprintf(t.payload, sizeof(int), "%d", counter++);
            snprintf(t.payload + sizeof(int), sizeof(int),
                "%d", package.parity);
            memcpy(t.payload + 2 * sizeof(int), package.message, lungime_mesaj);
            t.len = lungime_mesaj;
            send_message(&t);
        }
        int res = recv_message_timeout(&t, 1.3 * atoi(argv[3]));
        if (res > 0) {
          printf("idk, good! %d\n", i);
        } else {
          break;
        }
    }

    for (int i = 0; i < win_size; ++i) {
        int res = recv_message_timeout(&t, 1.3 * atoi(argv[3]));
        if (res > 0) {
          printf("idk, good! %d\n", i);
        } else {
          break;
        }
    }
    return 0;
}
