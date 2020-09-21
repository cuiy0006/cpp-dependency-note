#include <stdio.h>

void xyz() {
  printf("foo-xyz\n");
}

void foo() {
  printf("foo\n");
  xyz();
}