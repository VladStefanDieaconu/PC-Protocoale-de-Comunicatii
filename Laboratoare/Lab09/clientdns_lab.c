#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>

#define PORT 53

struct dnshdr {
  uint16_t id;

  uint8_t rd :1; // recursivitate
  uint8_t tc :1; // mesaj truncat
  uint8_t aa :1; // raspuns autoritativ
  uint8_t opcode :4; // cod mesaj
  uint8_t qr :1; // query sau raspuns?

  uint8_t rcode :4; // cod de raspuns
  uint8_t cd :1; // checking disabled
  uint8_t ad :1; // authenticated data
  uint8_t z :1; // rezervat
  uint8_t ra :1; // recursion available

  /* numar intrebari */
  uint16_t qdcount;
  /* numar de resource records in raspunsuri */
  uint16_t ancount;
  /* numar de NS resource records in zona de inregistrari autoritative*/
  uint16_t nscount;
  /* numar de resource records in zona aditionala*/
  uint16_t arcount;
} __attribute__ ((packed));

struct dnsquery {
  uint16_t qtype;
  uint16_t qclass;
} __attribute__ ((packed));

struct dnsrr {
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t rdlength;   /* lungimea rdata */
  char     rdata[];
} __attribute__ ((packed));

// Driver code
int main(int argc,char** argv)
{
  char buffer[1000];
  char query[500];

  char* name = "facebook.com";
  char* server_addr = "8.8.8.8";

  if (argc>1)
    name = argv[1];

  if (argc>2)
    server_addr = argv[2];

  int sockfd, n;
  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_addr.s_addr = inet_addr(server_addr);
  servaddr.sin_port = htons(PORT);
  servaddr.sin_family = AF_INET;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  bzero(query,500);

  struct dnshdr * hdr = (struct dnshdr*)&query;
  hdr->id = htons(113);
  hdr->rd = 1;
  hdr->qdcount = htons(1);

  int len = sizeof(struct dnshdr);

  int crt = 0;
  int namesz = strlen(name);

  while (crt < namesz){
    //find position of next '.'
    int sz=0;
    while (sz + crt < namesz && name[sz+crt]!='.')
      sz++;

    //primul octet este dimensiunea numelui
    query[len++] = sz;

    if (sz==0)
      break;

    //urmat de nume
    for (int i=0;i<sz;i++){
      query[len++] = name[crt+i];
    }
    crt = crt + sz+1;
  }
  query[len++] = 0;

  struct dnsquery * q = (struct dnsquery*)&query[len];

  q->qtype = htons(1);
  q->qclass = htons(1);

  len += sizeof(struct dnsquery);
  sendto(sockfd, query, len, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

  int r = recvfrom(sockfd, buffer, 1000, 0, (struct sockaddr*)NULL, NULL);

  //TODO
  //parsati raspunsul si afisati inregistrarile de tip A
  //nu este necesar sa adaugati suport pentru pointeri
  struct dnshdr* ssj = (struct dnshdr * ) (buffer);
  char* unu = buffer + sizeof(struct dnshdr);
  printf("%s\n", unu);
  int k = 0;
  int count = ntohs(ssj->ancount);
  while(k < count){
    int deplasament = sizeof(struct dnshdr) + strlen(unu) + 7 + k*sizeof(struct dnsrr);
    struct dnsrr* rasp = buffer + deplasament;
    // printf("%d\n",ntohs(rasp->type) );
    // printf("%d\n",ntohs(rasp->class) );
    // printf("%d\n",ntohs(rasp->ttl) );
    // printf("%d\n",ntohs(rasp->rdlength) );
    int nr;
    memcpy(&nr, rasp->rdata, 1);
    printf("%d ", nr);
    memcpy(&nr, rasp->rdata+1, 1);
    printf("%d ", nr);
    memcpy(&nr, rasp->rdata+2, 1);
    printf("%d ", nr);
    memcpy(&nr, rasp->rdata+3, 1);
    printf("%d\n", nr);
    k++;
  }

  close(sockfd);
}
