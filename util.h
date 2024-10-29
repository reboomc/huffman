#pragma once

#define NEW_CMP(func, type, conv)                                              \
  static int func(const void *a, const void *b) {                              \
    type x = conv(a);                                                          \
    type y = conv(b);                                                          \
    if (x < y)                                                                 \
      return -1;                                                               \
    if (x == y)                                                                \
      return 0;                                                                \
    return 1;                                                                  \
  }
