// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "blocks.h"
#include <time.h>

typedef struct inode {
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int ptr[2]; // single block pointer (if max file size <= 4K)
  int iptr;
  time_t create_time;
  time_t access_time;
  time_t modification_time;
} inode_t;

void print_inode(inode_t *node);
inode_t *get_inode(int inum);
int alloc_inode();
void free_inode();
int grow_inode(inode_t *node, int size);
int shrink_inode(inode_t *node, int size);
int inode_get_bnum(inode_t *node, int fbnum);

#endif
