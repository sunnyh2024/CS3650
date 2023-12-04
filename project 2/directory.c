#include <stdio.h>
#include <string.h>

#include "directory.h"
#include "bitmap.h"
#include "blocks.h"
#include "inode.h"
#include "slist.h"
#include <errno.h>

// Set up the root directory
void directory_init() {
  inode_t* rnode = get_inode(alloc_inode());
  rnode->mode = 040755; // Give read/write permissions & set as
  // a directory
}

// Return the inum of the given file (name) in the given directory inode
int directory_lookup(inode_t *dd, const char *name) {
  if (strcmp("", name) == 0) {
    return 0;
  }

  dirent_t *dir = blocks_get_block(dd->ptr[0]);
  int dirCount = dd->size / sizeof(dirent_t);

  // Iterate through all directory contents until one matches
  // the one we are looking for
  for (int i = 0; i < dirCount; i++) {
    dirent_t currDir = dir[i];

    if ((strcmp(name, currDir.name) == 0) && (currDir.used == 1)) {
      return currDir.inum;
    }
  }

  return -1;
}

// Returns the given filepath's file inum
int tree_lookup(const char *path) {
  // Get a list of all directories in the given path
  slist_t* list = s_explode(path, '/');
  slist_t* currDir = list;

  int rinum = 0;

  // Iterate through through each inode and find the one that contains the
  // file we are looking for
  while (currDir != NULL) {
    inode_t* rnode = get_inode(rinum);
    rinum = directory_lookup(rnode, currDir->data);
    if (rinum == -1) {
      s_free(list);
      return -1;
    }

    currDir = currDir->next;
  }

  s_free(list);

  return rinum;
}

// Puts a new file in the given dd with the given name and inum
int directory_put(inode_t *dd, const char *name, int inum) {
  dirent_t *dir = blocks_get_block(dd->ptr[0]);

  int added = 0;

  dirent_t newFile;

  // Create new file with given filename & inum
  strcpy(newFile.name, name);
  newFile.inum = inum;
  newFile.used = 1;

  // Add file to given directory
  int dirCount = dd->size / sizeof(dirent_t);	
  dir[dirCount] = newFile;

  for (int i = 1; i < dirCount; i++) {
    if (dir[i].used == 0) {
      dir[i] = newFile;
      added = 1;
    }
  }

  if (added == 0) {
    dir[dirCount] = newFile;
    dd->size += sizeof(dirent_t);
  }

  // Increase size of directory
  dd->size = dd->size + sizeof(dirent_t);

  return 0;
}

// Delete the file with the given filename in the given directory
int directory_delete(inode_t *dd, const char *name) {
  dirent_t *dir = blocks_get_block(dd->ptr[0]);

  int dirCount = dd->size / sizeof(dirent_t);

  // Find the file that matches the given filename and delete it
  for (int i = 0; i < dirCount; i++) {
    if (strcmp(dir[i].name, name) == 0) {
      dir[i].used = 0;
      inode_t *fileNode = get_inode(dir[i].inum);
      fileNode->refs = fileNode->refs - 1;
      if (fileNode->refs < 1) {
	free_inode(dir[i].inum);
      }

      return 0;
    }
  }

  return -ENOENT;
}

// Return a list of the given path's directory contents
slist_t *directory_list(const char *path) {
  int inum = tree_lookup(path);
  inode_t *node = get_inode(inum);

  int dirCount = node->size / sizeof(dirent_t);
  dirent_t *dir = blocks_get_block(node->ptr[0]);
  slist_t *results = NULL;

  for (int i = 0; i < dirCount; i++) {
    if (dir[i].used == 1) {
      results = s_cons(dir[i].name, results);
    }
  }

  return results;
}

// Print the items in the given directory inode
void print_directory(inode_t *dd) {
  dirent_t *dir = blocks_get_block(dd->ptr[0]);
  int dirCount = dd->size / sizeof(dirent_t);

  for (int i = 0; i < dirCount; i++) {
    printf("%s\n", dir[i].name);
  }
}
