#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>
#include "storage.h"
#include "inode.h"
#include "bitmap.h"
#define FUSE_USE_VERSION 26
#include <fuse.h>

// Print the inode data
void print_inode(inode_t *node) {
  printf("node position: %p\n", node);
  printf("refs: %d\n", node->refs);
  printf("mode: 0x%X\n", node->mode);
  printf("size: %d\n", node->size);
  printf("pointers: %d, %d\n", node->ptr[0], node->ptr[1]);
  printf("indirect pointer: %d\n", node->iptr);
}

// Get the inode for the given inum 
inode_t *get_inode(int inum) {
  inode_t* inode = get_inode_bitmap() + 32;

  return &inode[inum];
}

// Allocate a new inode and return its inum
int alloc_inode() {
  for (int i = 0; i < 256; ++i) {
    // If inode does not exist, create new one
    if (!bitmap_get(get_inode_bitmap(), i)) {
      bitmap_put(get_inode_bitmap(), i, 1);
      inode_t* newNode = get_inode(i);
      newNode->refs = 1;
      newNode->size = 0;
      newNode->mode = 0;
      newNode->ptr[0] = alloc_block();

      return i;
    }
  }

  return -1;
}

// Free the inode
void free_inode(int inum) {
  inode_t* delete_node = get_inode(inum);
  void* b_map = get_inode_bitmap();
  shrink_inode(delete_node, 0);
  free_block(delete_node->ptr[0]);
  bitmap_put(b_map, inum, 0);
}

// Increase size of the given inode
int grow_inode(inode_t *node, int size) {
  int base = (node->size / 4096) + 1;
  for (int i = base; i <= size / 4096; i++) {
    if (i >= 2) {
      if (node->iptr == 0) {
	node->iptr = alloc_block();
      }

      int* dir = blocks_get_block(node->iptr);
      dir[i - 2] = alloc_block();
    }
    else {
      node->ptr[i] = alloc_block();
    }
  }

  node->size = size;

  return 0;
}

// Shrink size of the given inode
int shrink_inode(inode_t *node, int size) {
  int base = node->size / 4096;
  for (int i = base; i > size / 4096; i--) {
    if (i >= 2) {
      int* dir = blocks_get_block(node->iptr);
      free_block(dir[i - 2]);
      dir[i - 2] = 0;

      if (i == 2) {
	free_block(node->iptr);
	node->iptr = 0;
      }
      else {
	free_block(node->ptr[i]);
	node->ptr[i] = 0;
      }
    }
  }
  node->size = size;

  return 0;
}

// Get the pnum of the given inode
int inode_get_bnum(inode_t *node, int fpn) {
  int count = fpn / 4096;

  if (count >= 2) {
    int* dir = blocks_get_block(node->iptr);

    return dir[count - 2];
  }
  else {
    return node->ptr[count];
  }
}
