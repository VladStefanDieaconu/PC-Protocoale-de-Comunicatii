#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
//#include <asm/param.h>
#include "queue.h"
#include "link.h"

#define DEBUG 0

int BUFFER_SIZE=1000;

int serialization_delay = 1000;
int delay = 1000;
int loss = 0;
int corrupt = 0;

#define CHANNEL_BUSY 1
#define CHANNEL_IDLE 0

#define LOCAL_PORT1 10000
#define LOCAL_PORT2 10001

pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond = PTHREAD_COND_INITIALIZER; 
queue* buffer;

struct sockaddr_in local_addr1, remote_addr1;
struct sockaddr_in local_addr2, remote_addr2;

int s1,s2;
socklen_t sz;

//1 if a message was received on this link
int link_up1 = 0;
int link_up2 = 0;

void init_sockets()
{
  /*LOCAL ADDRESSES*/

  memset((char *) &local_addr1, 0, sizeof(local_addr1));
  local_addr1.sin_family = AF_INET;
  local_addr1.sin_port = htons(LOCAL_PORT1);
  local_addr1.sin_addr.s_addr = htonl(INADDR_ANY);

  memset((char *) &local_addr2, 0, sizeof(local_addr2));
  local_addr2.sin_family = AF_INET;
  local_addr2.sin_port = htons(LOCAL_PORT2);
  local_addr2.sin_addr.s_addr = htonl(INADDR_ANY);

  if ((s1=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
    perror("Error creating socket");
    exit(1);
  }

  //now bind
  if (bind(s1, (struct sockaddr*)&local_addr1, sizeof(local_addr1))==-1){
    perror("Failed to bind s1");
    exit(1);
  }

  if ((s2=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1){
    perror("Error creating socket 2");
    exit(1);
  }

  if (bind(s2, (struct sockaddr*)&local_addr2, sizeof(local_addr2))==-1){
    perror("Failed to bind s2");
    exit(1);
  }
}

int send_message1(const msg* m){
  if (!link_up1){
    printf("Trying to send a message but remote peer is not connected on my port %d\n",LOCAL_PORT1);
  }
  return sendto(s1, m, sizeof(msg), 0,(struct sockaddr*) &remote_addr1, sizeof(remote_addr1));
}

msg* receive_message1(){
  msg* ret;

 a:
  ret = (msg*)malloc(sizeof(msg));
  
  if (!link_up1){
    sz = sizeof(remote_addr1);
    if (recvfrom(s1, ret, sizeof(msg), 0, (struct sockaddr*)&remote_addr1, &sz)==-1){
      free(ret);
      return NULL;
    }
    
    link_up1 = 1;

#if DEBUG
    printf("Link 1 is up, remote addr is %s port %d\n",inet_ntoa(remote_addr1.sin_addr),ntohs(remote_addr1.sin_port));
#endif

    free(ret);
    goto a;
  }
  else if (recvfrom(s1, ret, sizeof(msg), 0, NULL,NULL)==-1){
    free(ret);
    return NULL;
  }
  return ret;
}

int send_message2(const msg* m){
  if (!link_up2){
    printf("Trying to send a message but remote peer is not connected on my port %d\n",LOCAL_PORT2);
  }
  return sendto(s2, m, sizeof(msg), 0,(struct sockaddr*) &remote_addr2, sizeof(remote_addr2));
}

msg* receive_message2(){
  msg* ret;

 a:
  ret = (msg*)malloc(sizeof(msg));
  if (!link_up2){
    sz = sizeof(remote_addr2);
    if (recvfrom(s2, ret, sizeof(msg), 0, (struct sockaddr*)&remote_addr2, &sz)==-1){
      free(ret);
      return NULL;
    }
    link_up2 = 1;

#if DEBUG
    printf("Link 2 is up, remote addr is %s port %d\n",inet_ntoa(remote_addr2.sin_addr),ntohs(remote_addr2.sin_port));
#endif

    free(ret);
    goto a;
  }
  else
  if (recvfrom(s2, ret, sizeof(msg), 0, NULL, NULL)==-1){
    free(ret);
    return NULL;
  }

  return ret;
}

unsigned long long now(){ 
  struct timeval b;
  gettimeofday(&b,NULL);
  return (unsigned long long)b.tv_sec*1000000+b.tv_usec;
}

void* link_scheduler(void *argument)
{
  msg_in_flight* mif;
  queue* in_flight = create_queue();
  long long idle_time = 0;
  long long crt_time,wait_time_idle,wait_time_send;
  int stuff;

  while (1){
    crt_time = now();

#if DEBUG
    printf("In flight size %d at %lld\n",in_flight->size, crt_time);
#endif

    while (in_flight->size>0){
      msg_in_flight* last = (msg_in_flight*)in_flight->last->crt;
      if (crt_time<last->finish_time){
	break;
      }

      //else send first packet on the wire
      mif = (msg_in_flight*)dequeue(in_flight);
      if (!mif){
	printf("Error in deque: expecting non null msg!\n");
	exit(1);
      }
      if (send_message2(mif->m)<=0)
	perror("SNDMSG2");
      
#if DEBUG
      printf("Sending message\n");
#endif

      free(mif->m);
      free(mif);
      mif = NULL;
    }

    pthread_mutex_lock( &buffer_lock );
    stuff = buffer->size>0;
    pthread_mutex_unlock( &buffer_lock );          

#if DEBUG
    printf("Stuff is %d\n",stuff);
#endif

    wait_time_send = wait_time_idle = 0;

    //crt_time = now();
    
    //now see if we can put stuff in flight
    if (stuff && crt_time>=idle_time){
      idle_time = crt_time + serialization_delay;
      
      mif = (msg_in_flight*)malloc(sizeof(msg_in_flight));
      assert(mif);
      
      pthread_mutex_lock( &buffer_lock );
      mif->m = (msg*)dequeue(buffer);
      pthread_mutex_unlock( &buffer_lock );          
      
      assert(mif->m);
      mif->finish_time = crt_time + serialization_delay + delay;
      
      //send message here from buffer to link
      enqueue(in_flight,mif);

#if DEBUG
      printf("Enquing message\n");
#endif
    }
    wait_time_idle = idle_time - crt_time;
    
    if (in_flight->size>0){
      msg_in_flight* last = (msg_in_flight*)in_flight->last->crt;
      wait_time_send = last->finish_time-crt_time;
    }

    if (wait_time_idle>0 || wait_time_send>0){
      long long wait_time = 0;
      if (wait_time_idle>0) wait_time = wait_time_idle;
      if (wait_time_send>0&&wait_time_send<wait_time)
	wait_time = wait_time_send;

      //printf("Sleeping %lld\n", wait_time);
      usleep(wait_time);
    }
    else {
	pthread_mutex_lock( &buffer_lock );
	assert(!buffer->size);
#if DEBUG
	printf("Waiting on cond\n");
#endif
	pthread_cond_wait(&buffer_cond,&buffer_lock);
	pthread_mutex_unlock( &buffer_lock );          
    }
  }
 
  return NULL;
}

void* run_forwarding(void* param){
  msg *m;

  while (1){
    int overflow;
    m = receive_message1();
    if (m==NULL){
      perror("Read error");
      exit(1);
    }
    
    //check queue space
    pthread_mutex_lock( &buffer_lock );
    overflow = buffer->size>=BUFFER_SIZE;
    pthread_mutex_unlock( &buffer_lock );    

    if (overflow || (rand()%100)<loss) {
      //just drop message
      free(m);
      printf("Dropped packet\n");
    } 
    else {
      if (rand()%100 < corrupt){
	m->payload[rand()%m->len] = rand()%128;
      }
      //printf("Enqueue 1.");
      pthread_mutex_lock( &buffer_lock );
      enqueue(buffer,m);
      pthread_cond_signal(&buffer_cond);
      pthread_mutex_unlock(&buffer_lock);      
      //printf("Done!\n");
    }
  }
}

void* run_reverse_forwarding(void* param){
  msg *m;

  while (1){
    m = receive_message2();
    if (m==NULL){
      perror("Read error");
      exit(1);
    }
    
    send_message1(m);
    free(m);
  }
}

#define SPEED 1
#define DELAY 2
#define LOSS 3
#define CORRUPT 4

int split_param(char* p,int * type, double* value){
  char c[100];
  int crt = 0, t=1;
  
  for (;*p!=0;p++){
    if (t && *p=='='){
      t = 0;
      c[crt] = 0;
      crt = 0;
      
      if (!strcasecmp(c,"speed"))
	*type = SPEED;
      else if (!strcasecmp(c,"delay"))
	*type = DELAY;
      else if (!strcasecmp(c,"loss"))
	*type = LOSS;
      else if (!strcasecmp(c,"corrupt"))
	*type = CORRUPT;
      else {
	printf ("Unknown parameter %s\n",c);
	return -1;
      }
    }
    else c[crt++] = *p;
  }
  *value = atof(c);
  return 0;
}

int guess_hz(){
  long long a,b,diff = 0;
  int i;
  
  for (i=0;i<100;i++){
    a = now();
    //0.1ms
    usleep(100);
    b = now();
    diff += (b-a);
  }

  int error = (int)(diff/i-100);
  printf("Average error  100 was %d\n",error);

  diff = 0;
  for (i=0;i<100;i++){
    a = now();
    //0.1ms
    usleep(1000);
    b = now();
    diff += (b-a);
  }

  error = (int)(diff/i-1000);
  printf("Average error 1000 was %d\n",error);
  return  error;
}

int main(int argc, char** argv){
  pthread_t link_thread, fw_thread;
  int i;
  for (i=1;i<argc;i++){
    int type;
    double value;
    if (split_param(argv[i],&type,&value)<0){
      printf("Usage %s speed=[speed in mb/s] delay=[delay in ms] loss=[percent of packets] corrupt=[percent of packets]\n",argv[0]);
      return -1;
    }
    
    switch(type){
    case SPEED:
      printf("Setting speed to %f Mb/s\n",value);
      serialization_delay = 2*11200/value;break;
    case DELAY:
      printf("Setting delay %f to ms\n",value);
      delay = value*1000; break;
    case LOSS:
      printf("Setting loss rate to %f%%\n",value);      
      loss = value; break;
    case CORRUPT:
      printf("Setting corruption rate to %f%%\n",value);      
      corrupt = value; break;
    }
  }

#if DEBUG
  guess_hz();
#endif

  init_sockets();
  srand(time(NULL));
  buffer = create_queue();
  assert(!pthread_create(&link_thread, NULL, link_scheduler, NULL));
  assert(!pthread_create(&fw_thread, NULL, run_forwarding, NULL));

  run_reverse_forwarding(NULL);
  return 0;
}
