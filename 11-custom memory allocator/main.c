#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/mman.h>

#define MEMORY_SIZE 1024 * 1024
#define ALIGN(n) (((n) + 15) & ~15)


//Forward declaration
typedef struct Block block_t;

//Struct which maintains information about the data in the memory block it is associated with
typedef struct Block {
  size_t data_size;  //Size of the data that follows
  int free;          //0 means not free and 1 means free
  block_t *next_block;
} block_t;

#define HEADER_SIZE ALIGN(sizeof(block_t))

static block_t *free_list = NULL;

void *custom_malloc(size_t requested_bytes)
{
  requested_bytes = ALIGN(requested_bytes);
  block_t *current = free_list;

  //We keep iterating the list to find the first free block
  while(current)
  {
    if(current -> free == 1 && current->data_size >= requested_bytes)
    {
      //Splitting ( if the remaining block is large enough )
      if(current->data_size >= requested_bytes + HEADER_SIZE + 16)
      {
        block_t *split = (block_t *)((char *)current + HEADER_SIZE + requested_bytes);

        split->data_size   = current->data_size - requested_bytes - HEADER_SIZE;
        split->free   = 1;
        split->next_block   = current ->next_block;

        current -> next_block = split;
        current -> data_size = requested_bytes;
      }

      current->free = 0;
            return (char *)current + HEADER_SIZE;
    }
    
    current = current->next_block;
  }
}

void custom_free(void *ptr)
{
  if (!ptr) return;
  block_t *block = (block_t *)((char *)ptr - HEADER_SIZE);
  block->free  = 1;

  //Coalescing
  block_t *curr = free_list;
    while (curr && curr->next_block) {
        if (curr->free && curr->next_block->free) {
            curr->data_size += HEADER_SIZE + curr->next_block->data_size;
            curr->next_block  = curr->next_block->next_block;
        } else {
            curr = curr->next_block;
        }
    }

}

int main()
{
  //As a first step we request a large block of memory from the OS using mmap operation
  void *mmap_memory = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if(mmap_memory == MAP_FAILED)
  {
    printf("Memory mapping failed\n");
    return 1;
  }

  printf("Starting address of mmap memory %p\n",mmap_memory);

  //Now we create our first free block
  free_list = (block_t *)mmap_memory;
  free_list -> data_size = MEMORY_SIZE - HEADER_SIZE;
  free_list -> free = 1;
  free_list -> next_block = NULL;

  printf("First block — size: %zu, free: %d\n", free_list->data_size, free_list->free);

  void *a = custom_malloc(64);
  void *b = custom_malloc(128);
  custom_free(a);
  custom_free(b);
  return 0;
}