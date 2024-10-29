#include "huff.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 排序节点
typedef struct _sort {
  hf_node_t *node;
  // 便于插入新节点，避免拷贝
  struct _sort *prev;
  struct _sort *next;
} sort_t;

static void _traverse(hf_node_t *n, const hf_node_t *root) {
  if (!n || !root)
    return;

  const hf_node_t *cur = n;
  n->tot = 0;
  uint8_t filo[16];

  while (1) {
    uint8_t path = cur->parent->left == cur ? 0 : 1;
    filo[n->tot++] = path;

    if (cur->parent == root)
      break;
    cur = cur->parent;
  }

  for (uint8_t i = 0; i < n->tot; i++) {
    n->output[i] = filo[n->tot - i - 1];
  }
}

static uint16_t _n2u(const void *v) { return ((const hf_node_t *)v)->weight; }

NEW_CMP(_sort_nodes, uint16_t, _n2u);

// 节点转换为排序表
static sort_t *_node2sort(hf_node_t *nodes, uint8_t len) {
  sort_t *sts = (sort_t *)malloc(sizeof(sort_t) * len);

  for (uint8_t i = 0; i < len; i++) {
    sort_t *cur = sts + i;

    cur->prev = 0 == i ? NULL : sts + i - 1;
    cur->next = len - 1 == i ? NULL : sts + i + 1;

    cur->node = nodes + i;
  }
  return sts;
}

/*
抛弃俩节点，并且加入新节点
@param head_sts: 首节点

@return 新的首节点
 */
static sort_t *_replace_sort(sort_t *head_sts, const hf_node_t *new,
                             hf_node_t **root, sort_t **out_insert) {
  // 插入节点
  sort_t *insert = (sort_t *)malloc(sizeof(*insert));
  *out_insert = insert;

  hf_node_t *tgt = (hf_node_t *)malloc(sizeof(hf_node_t));
  memcpy(tgt, new, sizeof(*tgt));

  insert->node = tgt;
  tgt->left->parent = tgt;
  tgt->right->parent = tgt;
  tgt->parent = NULL;

  *root = tgt;

  /*
  既然排序表可保证有序
  那么队列头的2个节点即可直接删除
  */
  sort_t *fix_head = head_sts->next->next;
  if (!fix_head) {
    // 排序列表已结束
    return NULL;
  }

#define CMP_CN(cur, op, try) ((cur)->node->weight op(try)->node->weight)

  sort_t *next = fix_head;
  while (1) {
    sort_t *cur = next;
    if (!cur)
      break;

    if (CMP_CN(insert, <=, cur)) {
      cur->prev = insert;
      insert->next = cur;
      insert->prev = NULL;
      fix_head = insert;

      break;
    } else if (CMP_CN(cur, <=, insert) &&
               (!cur->next || (cur->next && CMP_CN(insert, <=, cur->next)))) {
      insert->prev = cur;
      insert->next = cur->next;

      if (cur->next) {
        cur->next->prev = insert;
      }
      cur->next = insert;
      break;
    }

    next = cur->next;
  }

  return fix_head;
}

static hf_node_t *_2min_nodes(const sort_t *head_sts, hf_node_t **two) {
  *two = (head_sts->next)->node;
  return head_sts->node;
}

static void _print_enc(const hf_node_t *n) {
  printf("sym: %c, ", n->sym);
  for (uint8_t i = 0; i < n->tot; i++) {
    printf("%d", *(n->output + i));
  }
  printf("\n");
}

/*
遍历叶子节点，构建根对象

@param leaf_nodes: 所有叶子节点
    要求传入前已排序
@param len: 数量
 */
static void _build(hf_node_t *leaf_nodes, uint8_t len) {
  if (!leaf_nodes || len < 2)
    return;

  uint8_t tot = len;
  // 构建排序表
  sort_t *sts = _node2sort(leaf_nodes, len);
  // 用于销毁排序表
  sort_t *orig_sts = sts;
  /*
  储存插入排序过程中生成的父节点
  最终统一释放
   */
  uint8_t fly_cnt = 0;
  sort_t **ont_fly_sorts = (sort_t **)malloc(sizeof(sort_t *) * (tot - 1));

  hf_node_t *root = NULL;
  while (1) {
    // 每次选择队首最小的俩值作为计算目标
    hf_node_t *two;
    hf_node_t *one = _2min_nodes(sts, &two);

    hf_node_t tn;
    tn.weight = one->weight + two->weight;
    tn.left = one;
    tn.right = two;
    tn.sym = 0x00;

    // 过程中创建的临时根节点
    sort_t *tsr;
    sts = _replace_sort(sts, &tn, &root, &tsr);
    if (tsr) {
      *(ont_fly_sorts + fly_cnt++) = tsr;
    }

    if (1 == --len) {
      /*
      在完成插入排序后，节点数减1
      减到1之后，说明已是根节点
       */
      break;
    }
  }

  for (uint8_t i = 0; i < tot; i++) {
    _traverse(leaf_nodes + i, root);
    _print_enc(leaf_nodes + i);
  }

  for (uint8_t i = 0; i < fly_cnt; i++) {
    sort_t *cur = *(ont_fly_sorts + i);
    free(cur->node);
    free(cur);
  }
  free(ont_fly_sorts);
  free(orig_sts);
}

void build_tree(const char *input, uint32_t len) {
  // {'H': 1, 'e': 1, 'l': 3, 'o': 2, ' ': 1, 'w': 1, 'r': 1, 'd': 1}
  hf_node_t leaf_nodes[] = {
      {1, ' ', 0, {0}, NULL, NULL, NULL}, {1, 'r', 0, {0}, NULL, NULL, NULL},
      {1, 'e', 0, {0}, NULL, NULL, NULL}, {1, 'd', 0, {0}, NULL, NULL, NULL},
      {1, 'w', 0, {0}, NULL, NULL, NULL}, {1, 'H', 0, {0}, NULL, NULL, NULL},
      {2, 'o', 0, {0}, NULL, NULL, NULL}, {3, 'l', 0, {0}, NULL, NULL, NULL},
  };

  size_t count = sizeof(leaf_nodes) / sizeof(hf_node_t);
  qsort(leaf_nodes, count, sizeof(hf_node_t), _sort_nodes);

  _build(leaf_nodes, count);
}