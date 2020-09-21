#include <stdio.h>

void xyz() {
  printf("bar-xyz\n");
}

void bar() {
  printf("bar\n");
  xyz();
}