#pragma once

#include <stdint.h>

typedef struct _node {
  // 数量
  uint16_t weight;
  // 字符
  char sym;
  // 从根节点开始的路径数
  uint8_t tot;
  // 从根节点开始的各个路径值，即最终的编码
  uint8_t output[16];
  // 父节点
  struct _node *parent;
  // 左子节点
  struct _node *left;
  // 右子节点
  struct _node *right;
} hf_node_t;

void build_tree(const char *input, uint32_t len);