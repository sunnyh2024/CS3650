#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "storage.h"
#include "slist.h"
#include "blocks.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"

// Initializes the root directory
void storage_init(const char *path) {
  // Initializes the blocks
  blocks_init(path);

  void *block = get_blocks_bitmap();

  // Initializes the root directory if it's not allocated
  if (!bitmap_get(block, 4)) {
    directory_init();
  }
}

// Populate the given out param with the correct data, return -1 if error
// and 0 if success
int storage_stat(const char *path, struct stat *st) {
  // Get inum for the given path
  int inum = tree_lookup(path);

  // If valid inum, populate st
  if (inum > 0) {
    inode_t* node = get_inode(inum);
    st->st_nlink = node->refs;
    st->st_size = node->size;
    st->st_mode = node->mode;
    st->st_atime = node->access_time;
    st->st_ctime = node->create_time;
    st->st_mtime = node->modification_time;

    return 0;
  }	

  return -1;
}

// Read from file. Return the read size
int storage_read(const char *path, char *buf, size_t size, off_t offset) {
  // Get inode of path
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  int sizeCpy = size, offsetCpy = offset;

  int i = 0;
  while (sizeCpy > 0) {
    int pnum = inode_get_bnum(node, offsetCpy);
    char *block = blocks_get_block(pnum);

    block += offsetCpy % BLOCK_SIZE;

    int min = BLOCK_SIZE - (offsetCpy % BLOCK_SIZE);

    if (sizeCpy < min) {
      min = sizeCpy;
    }

    memcpy(buf + i, block, min);

    i += min;
    offsetCpy += min;
    sizeCpy -= min;
  }

  return size;
}

// Write to file. Return the write size
int storage_write(const char *path, const char *buf, size_t size, off_t offset) {
  // Get inode of path
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  int newSize = size + offset;
  if (node->size < newSize) {
    storage_truncate(path, newSize);
  }

  int sizeCpy = size, offsetCpy = offset;

  int i = 0;

  while (sizeCpy > 0) {
    int pnum = inode_get_bnum(node, offsetCpy);
    char *block = blocks_get_block(pnum);

    block += offsetCpy % BLOCK_SIZE;

    int min = BLOCK_SIZE - (offsetCpy % BLOCK_SIZE);

    if (sizeCpy < min) {
      min = sizeCpy;
    }

    memcpy(block, buf + i, min);
    i += min;
    offsetCpy += min;
    sizeCpy -= min;	
  }

  return size;
}

// Truncate the file by the given size. Return 0 for success.
int storage_truncate(const char *path, off_t size) {
  // Get inode of path of path
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  // Grow or shrink inode to given size
  if (node->size < size) {
    grow_inode(node, size);
  }
  else {
    shrink_inode(node, size);
  }

  return 0;
}

// Helper function that splits the given path into the parent path and current
// directory
void split_path(const char *path, char *parent, char *curr) {
  slist_t *pathList = s_explode(path, '/');
  slist_t *dir = pathList;
  parent[0] = 0;

  while (dir->next) {
    strcat(parent, "/");
    strcat(parent, dir->data);
    dir = dir->next;
  }

  memcpy(curr, dir->data, strlen(dir->data));
  curr[strlen(dir->data)] = 0;
  s_free(pathList);		
}

// Creates a new inode with the given path name & attributes specified
// by mode. Return 0 on success, EEXIST on file already exists, and ENOENT on
// any other error
int storage_mknod(const char *path, int mode) {
  // If file already exists, throw file already exists error
  if (tree_lookup(path) != -1) {
    return -EEXIST;
  }	

  char *curr = malloc(50);
  char *parent = malloc(strlen(path));
  split_path(path, parent, curr);

  // Check that parent inode exists
  int parentInum = tree_lookup(parent);
  if (parentInum < 0) {
    free(curr);
    free(parent);
    return -ENOENT;
  }

  // Initialize new inode
  int newInode = alloc_inode();
  inode_t *node = get_inode(newInode);
  node->mode = mode;
  node->size = 0;
  node->refs = 1;
  inode_t *parentDir = get_inode(parentInum);

  // Put new inode in parent directory
  directory_put(parentDir, curr, newInode);

  free(curr);
  free(parent);

  return 0;
}

// Deletes the given path from the filesystem
// Unlink path from filesystem
int storage_unlink(const char *path) {
  char *curr = malloc(50);
  char *parent = malloc(strlen(path));
  split_path(path, parent, curr);

  int inum = tree_lookup(parent);
  inode_t *parentNode = get_inode(inum);

  // Delete inode
  int rv = directory_delete(parentNode, curr);

  free(curr);
  free(parent);

  return rv;
}


// Creates a new link to the given file. Returns 0 on success and -1 on error.
int storage_link(const char *from, const char *to) {
  // Check that 'to' inode exists
  int inum = tree_lookup(to);
  if (inum < 0) {
    return -ENOENT;
  }

  char *curr = malloc(50);
  char *parent = malloc(strlen(from));
  split_path(from, parent, curr);

  // Get parent inode
  int parentInum = tree_lookup(parent);
  inode_t *parentNode = get_inode(parentInum);

  // Increases references at inode
  directory_put(parentNode, curr, inum);
  get_inode(inum)->refs += 1;

  free(curr);
  free(parent);

  return 0;
}


// Renames the given file. Return 0 on success, ENOENT on error.
int storage_rename(const char *from, const char *to) {
  storage_link(from, to);
  storage_unlink(from);

  return 0;
}

// Sets the timespec for the given file. Returns 0 on success and -1 on error
int storage_set_time(const char *path, const struct timespec ts[2]) {
  // Get inode and check that it exists
  int inum = tree_lookup(path);
  if (inum < 0) {
    return -ENOENT;
  }
  inode_t *node = get_inode(inum);

  // Modify time stats
  node->access_time = ts[0].tv_sec;
  node->modification_time = ts[1].tv_sec;

  return 0;
}

// Returns a list of the contents pointed to by the given path
slist_t *storage_list(const char *path) {
  return directory_list(path);
}

// Access the given path. Return 0 on success and ENOENT on error
int storage_access(const char *path) {
  int inum = tree_lookup(path);
  if (inum >= 0) {
    inode_t *node = get_inode(inum);
    time_t currTime = time(NULL);
    node->access_time = currTime;

    return 0;
  }

  return -ENOENT;
}
