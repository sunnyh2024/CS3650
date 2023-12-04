/*
 * Queue implementation.
 *
 * - Implement each of the functions to create a working circular queue.
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your queue 
 *   functions. 
 */
#include <assert.h>
#include <stdlib.h>

#include "queue.h"

/** The main data structure for the queue. */
struct queue{
  unsigned int back;      /* The next free position in the queue
                           * (i.e. the end or tail of the line) */
  unsigned int front;     /* Current 'head' of the queue
                           * (i.e. the front or head of the line) */
  unsigned int size;      /* How many total elements we currently have enqueued. */
  unsigned int capacity;  /* Maximum number of items the queue can hold */
  long *data;             /* The data our queue holds  */
};

/** 
 * Construct a new empty queue.
 *
 * Returns a pointer to a newly created queue.
 */
queue_t *queue_new(unsigned int capacity) {
  // the queue and the queue data need to be initialized with malloc
  queue_t *q = (queue_t*)malloc(sizeof(queue_t));
  q->data = (long*)malloc(capacity * sizeof(long));
  q->capacity = capacity;
  q->front = 0;
  q->back = 0;
  q->size = 0;

  return q;
}

/**
 * Check if the given queue is empty
 *
 * Returns a non-0 value if the queue is empty, 0 otherwise.
 */
int queue_empty(queue_t *q) {
  assert(q != NULL);
  return !q->size; // returns true when size = 0
}

/**
 * Check if the given queue is full.
 *
 * Returns a non-0 value if the queue is empty, 0 otherwise.
 */
int queue_full(queue_t *q) {
  assert(q != NULL);
  return q->size == q->capacity; // check if we are at capacity
}

/** 
 * Enqueue a new item.
 *
 * Push a new item into our data structure.
 */
void queue_enqueue(queue_t *q, long item) {
  assert(q != NULL);
  assert(q->size < q->capacity);
  // set the next element of the queue (back) to the given item, then increment back
  q->data[q->back] = item;
  q->back = (q->back + 1) % q->capacity; // mod to account for wraparound
  q->size++;
}

/**
 * Dequeue an item.
 *
 * Returns the item at the front of the queue and removes an item from the 
 * queue.
 *
 * Note: Removing from an empty queue is an undefined behavior (i.e., it could 
 * crash the program)
 */
long queue_dequeue(queue_t *q) {
  assert(q != NULL);
  assert(q->size > 0);
  long ret;
  // store the element at the front of queue then increment front
  ret = q->data[q->front];
  q->front = (q->front + 1) % q->capacity; // mod to account for wraparound
  q->size--;
  return ret;
}

/** 
 * Queue size.
 *
 * Queries the current size of a queue (valid size must be >= 0).
 */
unsigned int queue_size(queue_t *q) {
  assert(q != NULL);
  return q->size;
}

/** 
 * Delete queue.
 * 
 * Remove the queue and all of its elements from memory.
 *
 * Note: This should be called before the proram terminates.
 */
void queue_delete(queue_t* q) {
  assert(q != NULL);
  free(q->data); // free the data in the queue
  free(q); // free the queue itself
}

