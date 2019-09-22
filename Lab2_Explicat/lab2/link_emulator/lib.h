#ifndef LIB
#define LIB

/*          README:

        Laboratorul presupune comunicatia fizica, punct la punct, (nivelul
    legatura de date) intre doua calculatoare, simulata prin intermediul a doua
    procese, send si recv si prin intermediul executabilului link, care
    reprezinta mediul pe care se vor transmite datele dintre send si recv (poate
    fi imaginat ca un cablu Ethernet de exemplu).
        
        Pentru acest laborator presupunem ca mediul fizic(deci executabilul link)
    este unul ideal, adica nu va corupe mesajele transmise pe el si nu va pierde
    mesajele transmise pe el (daca va uitati in run_experiment.sh veti observa ca
    variabilele CORRUPT si LOST sunt 0. Acestea reprezinta probabilitatile ca un
    mesaj sa fie corupt, respectiv pierdut).
        
        Ne propunem sa transmitem mai multe fisiere de pe calculatorul send (sau
    procesul send) spre calculatorul recv (sau procesul recv). Numai send stie
    cate fisiere se vor transmite, care este dimensiunea lor si care este continutul
    lor. Pentru a putea transmite toate fisierele, trebuie ca si recv sa fie
    informat in legatura cu ceea ce urmeaza sa primeasca. 
    
        Astfel se stabileste urmatorul protocol de comunicatie, de tip STOP AND
    WAIT (send trimite un pachet si nu va avansa la pachetul urmator pana cand
    nu primeste confirmare de la recv):
        
        Fluxul de mesaje dintre cele 2 procese arata astfel:
        
        send:               recv: 
        
    t1: SEND_MSG_1    
                        t1: RECV_MSG_1
                        t1: SEND_ACK_FOR_MSG_1
    t1: RECV_ACK_FOR_MSG_1
    
    
    t2: SEND_MSG_2    
                        t2: RECV_MSG_2
                        t2: SEND_ACK_FOR_MSG_2
    t2: RECV_ACK_FOR_MSG_2
    
    etc
    
        Asa cum am mai precizat, pentru a trimite fisierele, trebuie ca recv sa
    fie informat in legatura cu datele pe care le va primi.
        Deci fiecare pachet trimis pe mediul de transmisie trebuie sa aiba un
    anumit tip (identificator) in asa fel incat procesul sa isi dea seama cum
    ar trebui procesat acel pachet (reprezentat de campul type din structura
    content). De asemenea, intrucat pachetele trimise pot avea dimensiuni
    variabile va trebui sa specificam si dimensiunea fiecarui pachet (reprezentata
    de campul len din structura msg).
        Se stabileste urmatoarea regula (de fapt, o succesiune de pachete) pe
    care o vom folosi ca sa putem transmite un fisier.
        Vom imparti transmisia in 3 etape, fiecare etapa fiind alcatuita din
    2 pachete:
        
        Etapa 1:
        Se transmite denumirea fisierului. Mai intai se transmite un pachet de tip
        1 de la send la recv. recv il va primi, il va procesa (adica va crea
        fisierul cu denumirea recv_denumire_primita). Apoi se va trimite un pachet
        de tip 4 (confirmare/ACK) de la recv la send.
        
        Etapa 2:
        Se transmite dimensiunea fisierului. Mai intai se transmite un pachet de tip
        2 de la send la recv. recv il va primi, il va procesa (adica va initializa
        un contor pt a realiza cand s-a terminat transmisia unui fisier). Apoi se
        va trimite un pachet de tip 4 (confirmare/ACK) de la recv la send.
        
        Etapa 3:
        Se transmite din continutul fisierului. Mai intai se transmite un pachet de tip
        3 de la send la recv. recv il va primi, il va procesa (adica va scrie in
        fisierul recv continutul primit si va scadea din contorul initializat
        anterior cantitea de date primita). Apoi se va trimite un pachet de tip 4
        (confirmare/ACK) de la recv la send.
        
        Aceasta etapa se repeta pana se transmite tot continutul fisierului.
       
       * SFARSIT REGULA *
        
        De asemenea, am mai stabilit un tip de mesaj (5). Acesta va fi trimis de la
        send la recv pt a anunta recv ca nu mai sunt fisiere de transmis (fara
        acest tip de mesaj, recv nu poate stii decat ca s-a terminat transmisia
        unui fisier, dar nu are de unde sa stie daca mai sunt de primit fisiere
        sau nu). La receptia unui astfel de mesaj, recv va trimite o confimare
        lui send si isi va incheia executia.
        
        La fel, send imediat dupa ce a primit confimarea unui ACK pt mesajul
        de tip 5, isi va incheia executia.
        
        
        
*/

// Dimensiunea maxmima a unui pachet de pe mediul de transmisie
#define MAX_LEN 1400

// Constante pt tipul posibil al pachetelor

#define HEADER_NAME 1
#define HEADER_LENGTH 2
#define DATA 3
#define ACK 4

// EOT - End of Transmission
#define EOT 5

// Structura care reprezinta continutul si tipul unui pachet
typedef struct {
  int type;
  char payload[MAX_LEN - 4];
} content;

// Structura care reprezinta pachetul propriu zis.
// Mai precis: LEN(4 Bytes) -- TYPE (4 Bytes) -- FLUX CU DATE PROPRIU-ZISE(1392 Bytes)
typedef struct {
  int len;
  char payload[MAX_LEN];
} msg;

void init(char* remote,int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);

// Acestea sunt functiile pe care le veti folosi pentru transmisia si respectiv
// receptia unui pachet de pe mediul de transmisie. 
int send_message(const msg* m);
int recv_message(msg* r);


//msg* receive_message_timeout(int timeout);

#endif
