#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <debug.h> 
#include <sys/mman.h> 
#include <pthread.h>
#include <assert.h>

typedef struct block {
  size_t size;
  struct block *next;
  int free;
} block_t;

// static block_t *head; // header
void *head_ptr = NULL; // header
#define BLOCK_SIZE sizeof(block_t)
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

pthread_mutex_t malloc_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t free_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mmap_mtx = PTHREAD_MUTEX_INITIALIZER;

/**
 * adds the given block to the linked list
 *
 * @arg newblock -> pointer to the newest block just added
 */
void append_block(block_t *newblock) {
  // check if the first block of our linked lists exists
  if (head_ptr == NULL) {
    head_ptr = newblock;
    return;
  }
  // traverse linked list to find last block
  block_t *ptr = head_ptr;
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

  block_t *ptr = head_ptr;
  while (ptr != NULL) {
    if (ptr->free == 1 && ptr->size >= size) { 
      return ptr; // return first free block with sufficient size
    }
    ptr = ptr->next;
  }
  return NULL;
}

/**
 * allocates memory block of less than PAGE_SIZE bytes, and returns pointer to the block
 *
 * @arg size -> size of memory required
 * @return block_t*
 */
block_t *mymalloc_small(size_t s) {
  assert(s > 0);

  block_t *newBlock;
  newBlock = mmap(NULL, BLOCK_SIZE + PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);

  pthread_mutex_lock(&mmap_mtx);
  void *request = mmap(NULL, BLOCK_SIZE + PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);
  pthread_mutex_unlock(&mmap_mtx);

  if (request == (void*) -1) {
    return NULL;
  }

  newBlock->size = BLOCK_SIZE + PAGE_SIZE;
  newBlock->next = NULL;
  newBlock->free = 0;
  return newBlock;
}

/**
 * allocates memory block of PAGE_SIZE bytes or more, and returns pointer to the block
 *
 * @arg size -> size of memory required
 * @return block_t*
 */
block_t *mymalloc_large(size_t s) {
  assert(s > 0);

  pthread_mutex_lock(&mmap_mtx);
  void *request = mmap(NULL, BLOCK_SIZE + s, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANON, -1, 0);
  pthread_mutex_unlock(&mmap_mtx);

  if (request == (void*) -1) {
    return NULL;
  }

  block_t *remainingBlock = NULL;
  size_t remainingBlock_size = (BLOCK_SIZE + s) % PAGE_SIZE;
  
  if (remainingBlock_size <= BLOCK_SIZE) {
    remainingBlock = request + (BLOCK_SIZE + s);
    remainingBlock->size = remainingBlock_size;
    remainingBlock->next = NULL;
    remainingBlock->free = 1;
  }

  block_t *block = (block_t*) request;
  block->size = s + BLOCK_SIZE;
  block->next = remainingBlock;
  block->free = 0;

  return block;
}

/**
 * tries to split the given block such that the first block as size s
*/
void *split_block(void *ptr, size_t s) {
  assert(ptr != NULL);
  assert(s > 0);
  block_t *block = (block_t *) ptr;
  if (block->size < s + (2 * BLOCK_SIZE) + 1) {
    return NULL;
  }
  block_t *newBlock = (block_t *) (s + BLOCK_SIZE + ptr);
  newBlock->size = block->size - (s + BLOCK_SIZE);
  newBlock->free = 1;
  newBlock->next = NULL;

  block->size = s + BLOCK_SIZE;
  block->next = newBlock;
}

/**
 * coalesce the given block with the next block
*/
void *coalesce_block(void *ptr) {
  assert(ptr != NULL);
  block_t *block = (block_t *) ptr;
  block_t *next = block -> next;

  if (next != NULL && next->free == 1) {
    block->size += (BLOCK_SIZE + next->size);
    block->next = next->next;
  }
}

/**
 * allocates memory block of given size, and returns pointer to the block
 *
 * @arg s -> size of memory to be malloc
 */
void *mymalloc(size_t s) {
  assert(s > 0);

  block_t *block;

  if (head_ptr == NULL) {
    block = mymalloc_small(s);
    if (block == NULL) {
      return NULL;
    }
    head_ptr = block;
  }
  else {
    pthread_mutex_lock(&malloc_mtx);
    block = getAvailableBlock(s);
    pthread_mutex_unlock(&malloc_mtx);
    if (block == NULL) {
      if (BLOCK_SIZE + s < PAGE_SIZE) {
        block = mymalloc_small(s);
      }
      else {
        block = mymalloc_large(s);
      }

      if (block == NULL) {
        return NULL;
      }

      append_block(block);
      split_block(block, s);
    }
    else {
      block->free = 0;
    }
  }

  debug_printf("Malloc %zu bytes\n", s);

  return (block + 1);
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
  
  pthread_mutex_lock(&free_mtx);
  block_t *freeBlock = (block_t *)(ptr - BLOCK_SIZE);

  assert(freeBlock->free == 0);

  freeBlock->free = 1;
  if (freeBlock->size < PAGE_SIZE) {
    coalesce_block(freeBlock);
  }

  pthread_mutex_unlock(&free_mtx);

  debug_printf("Freed %zu bytes\n", freeBlock->size);
}
