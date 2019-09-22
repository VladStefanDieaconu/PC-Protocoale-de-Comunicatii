#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void enqueue(queue* q,void* m){
  if (q->first==NULL){
    assert(q->last==NULL);

    q->first = (queue_entry*)malloc(sizeof(queue_entry));
    q->last = q->first;
  }
  else {
    queue_entry * t = (queue_entry*)malloc(sizeof(queue_entry));
    q->first->prev = t;
    q->first = t;
  }

  q->first->prev = NULL;
  q->first->crt = m;

  q->size++;
}

void* dequeue(queue* q){
  queue_entry  * t;
  void* m;

  if (q->first==NULL){
    assert(q->size==0);
    return NULL;
  }
  else if (q->first==q->last){
    t = q->last;
    assert(q->first->prev==NULL);

    q->first = q->last = NULL;
  }
  else {
    t = q->last;
    q->last = q->last->prev;
  }

  m = t->crt;
  free(t);
  q->size--;

  return m;
}

queue* create_queue(){
  queue* q = (queue*)malloc(sizeof(queue));
  q->first = NULL;
  q->last = NULL;
  q->size = 0;
  return q;
}

void destroy_queue(queue* q){
  assert(0);
}
