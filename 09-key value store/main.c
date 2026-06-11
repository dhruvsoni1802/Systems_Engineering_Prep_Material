#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define MAX_VALUE_LENGTH 64
#define MAX_BUCKETS 10

typedef struct Bucket bucket_t;
typedef struct Entry entry_t;

typedef struct Entry {
  int key;
  char value[MAX_VALUE_LENGTH];
  entry_t *next;
} entry_t;

typedef struct Bucket {
  entry_t *head;
  pthread_rwlock_t lock;
} bucket_t;

typedef struct HashMap {
  bucket_t buckets[MAX_BUCKETS];
} hashmap_t;

int hash(int key)
{
  //For simplicity this is a modulo hash function. However use complex hash functions here
  return key % 10;
}

//Function to initialize the hashmap
hashmap_t *hashmap_create() {
    hashmap_t *map = calloc(1, sizeof(hashmap_t));
    for (int i = 0; i < MAX_BUCKETS; i++)
        pthread_rwlock_init(&map->buckets[i].lock, NULL);
    return map;
}

//Function to add an element to the bucket
void enqueue_hashmap(hashmap_t *hashmap,int key, char *value)
{
  int hashkey = hash(key);
  bucket_t *current_bucket = &hashmap->buckets[hashkey];

  pthread_rwlock_wrlock(&current_bucket->lock);

  entry_t *curr = current_bucket->head;
  while(curr != NULL)
  {
    if(curr->key == key)
    {
      strncpy(curr->value, value, MAX_VALUE_LENGTH);
      pthread_rwlock_unlock(&current_bucket->lock);
      return;
    }

    curr = curr->next;
  }

  //Key not found. Adding a new key at the front
  entry_t *new_entry = malloc(sizeof(entry_t));
  new_entry->key = key;
  strncpy(new_entry->value, value, MAX_VALUE_LENGTH);
  new_entry->next  = current_bucket->head;
  current_bucket->head = new_entry;

  pthread_rwlock_unlock(&current_bucket->lock);
}

//Function to read an element from the bucket
void read_hashmap(hashmap_t *hashmap,int key)
{
  int hashkey = hash(key);
  bucket_t *current_bucket = &hashmap->buckets[hashkey];

  pthread_rwlock_rdlock(&current_bucket->lock);

  entry_t *curr = current_bucket->head;
  while(curr != NULL)
  {
    if(curr->key == key)
    {
      printf("This is the value we are reading for key %d: %s\n",curr->key,curr->value);
      pthread_rwlock_unlock(&current_bucket->lock);
      return;
    }

    curr = curr->next;
  }

  pthread_rwlock_unlock(&current_bucket->lock);
}

int main()
{
  hashmap_t *shared_map = hashmap_create();
  enqueue_hashmap(shared_map,1, "dhruv");
  enqueue_hashmap(shared_map,2, "dev");

  read_hashmap(shared_map,1);
}


