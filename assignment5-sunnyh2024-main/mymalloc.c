#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <debug.h> 
#include <assert.h>

typedef struct block {
  size_t size;
  struct block *next;
  int free;
} block_t;

static block_t *head; // header
#define BLOCK_SIZE sizeof(block_t)

/**
 * adds the given block to the linked list
 *
 * @arg newblock -> pointer to the newest block just added
 */
void appendBlock(block_t *newblock) {
  // check if the first block of our linked lists exists
  if (head == NULL) {
    head = newblock;
    return;
  }
  // traverse linked list to find last block
  block_t *ptr = head;
  while (ptr->next != NULL) {
    ptr = ptr->next; // once exited, traverse is the last block
  }
  ptr->next = newblock;
}

/**
 * gets the pointer to the first available block with enough bytes
 *
 * @arg size -> size of memory required
 * @return block_t*
 */
block_t *getAvailableBlock(size_t size) {
  assert(size > 0); 

  block_t *ptr = head;
  while (ptr != NULL) {
    if (ptr->free == 1 && ptr->size >= size) { 
      return ptr; // return first free block with sufficient size
    }
    ptr = ptr->next;
  }
  return NULL;
}

/**
 * allocates memory block of given size, and returns pointer to the block
 *
 * @arg s -> size of memory to be malloc
 */
void *mymalloc(size_t s) {
  assert(s > 0);

  // checks if there is memory in current linked list that can fit the memory being malloced
  block_t *availableBlock = getAvailableBlock(s);
  if (availableBlock == NULL) {
    // no more room, make new block and connect to list
    block_t *newBlock = sbrk(s + BLOCK_SIZE); // request memory for block
    newBlock->size = s;
    newBlock->next = NULL;
    newBlock->free = 0;
    appendBlock(newBlock);

    debug_printf("malloc %zu bytes\n", s);
    return (void *)(newBlock + 1);
  }
  else { 
    // there is room available
    availableBlock->free = 0;

    debug_printf("malloc %zu bytes\n", availableBlock->size);
    return (void *)(availableBlock + 1);
  }
}

/**
 * allocates memory block of given size with given number of elements to 0, 
 * and returns the pointer to the block
 *
 * @arg nmemb -> number of elements to be allocated
 * @arg s -> size of elements
 * @return block_t*
 */
void *mycalloc(size_t nmemb, size_t s) {
  assert(nmemb > 0);
  assert(s > 0);

  void *ptr = mymalloc(nmemb * s);
  if (ptr == NULL) {
    return NULL; // failed to allocate
  }
  else {
    memset(ptr, 0, nmemb * s); // set to 0
  }
  debug_printf("calloc %zu bytes\n", s);
  return ptr;
}

/**
 * frees the memory at the given block pointer
 *
 * @arg ptr -> void pointer of memory to be freed
 */
void myfree(void *ptr) {
  assert(ptr != NULL);
  
  // find the memory at pointer and free it
  block_t *freeBlock = (block_t *)(ptr - BLOCK_SIZE);
  freeBlock->free = 1;

  debug_printf("Freed %zu bytes\n", freeBlock->size);
}
