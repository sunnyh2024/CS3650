/* Complete the C version of the driver program for compare. This C code does
 * not need to compile. */

#include <stdio.h>

extern long compare(long, long);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Two arguments required\n");
    return 1;
  }
  int arg1 = atoi(argv[1]);
  int arg2 = atoi(argv[2]);
  if (arg1 < arg2) {
    printf("less\n");
  }
  else if (arg1 == arg2) {
    printf("equal\n");
  }
  else {
    printf("greater\n");
  }
  return 0;
}

