#include "huff.h"
#include <string.h>

int main() {
  char *s = "Hello world";
  build_tree(s, strlen(s));

  return 0;
}