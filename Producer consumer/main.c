#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define CAPACITY 10

typedef struct RingBuffer {
  int buffer[CAPACITY];
  int head;
  int tail;
  int count;

  pthread_mutex_t lock;
  pthread_cond_t empty;
  pthread_cond_t full;
} ring_buffer_t;

ring_buffer_t *initialize_ring_buffer(void)
{
  ring_buffer_t *rb = malloc(sizeof(ring_buffer_t));
  rb->head = 0;
  rb->tail = 0;
  rb->count = 0;

  pthread_mutex_init(&rb->lock,NULL);
  pthread_cond_init(&rb->empty,NULL);
  pthread_cond_init(&rb->full,NULL);

  return rb;
}

void enqueue(ring_buffer_t *rb,int value)
{
  pthread_mutex_lock(&rb->lock);

  //If the queue is full we wait
  while(rb->count == CAPACITY)
  {
    pthread_cond_wait(&rb->empty,&rb->lock);
  }

  //Add an element at the tail of the queue
  rb->buffer[rb->tail] = value;
  printf("Adding element %d to the buffer\n",value);
  rb->tail = (rb->tail + 1) % CAPACITY;
  rb->count = rb->count + 1;

  pthread_cond_signal(&rb->full);
  pthread_mutex_unlock(&rb->lock);
}

void dequeue(ring_buffer_t *rb)
{
  pthread_mutex_lock(&rb->lock);

  //If the queue is empty we wait
  while(rb->count == 0)
  {
    pthread_cond_wait(&rb->full,&rb->lock);
  }

  //Take the element at the head of the queue
  int value = rb->buffer[rb->head];
  printf("Removing element %d from the buffer\n",value);
  rb->head = (rb->head + 1) % CAPACITY;
  rb->count = rb->count - 1;

  pthread_cond_signal(&rb->empty);
  pthread_mutex_unlock(&rb->lock);
}

void *producer(void *arg)
{
  ring_buffer_t *rb = (ring_buffer_t *)arg;
  for(int i = 0;i < 16;i++)
  {
    enqueue(rb,i);
  }
}

void *consumer(void *arg)
{
  ring_buffer_t *rb = (ring_buffer_t *)arg;
  for(int i = 0;i < 16;i++)
  {
    dequeue(rb);
  }
}

int main()
{
  ring_buffer_t *rb = initialize_ring_buffer();
  pthread_t producer_thread;
  pthread_t consumer_thread;

  pthread_create(&producer_thread,NULL,producer,(void *)rb);
  pthread_create(&consumer_thread,NULL,consumer,(void *)rb);

  pthread_join(producer_thread,NULL);
  pthread_join(consumer_thread,NULL);

  free(rb);
  return 0;
}