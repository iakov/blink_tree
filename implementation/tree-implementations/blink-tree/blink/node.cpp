/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "../palm/allocator.h"
#include "node.h"


blink_node *new_blink_node(uint8_t type, uint8_t level)
{
  blink_node *bn = (blink_node *)malloc(get_node_size());
  memset(bn, 0, get_node_size());
  rw_lock_create_func(&bn->latch, "", 0);
  node_init(bn->pn, type | Blink, level);

  return bn;
}

void free_blink_node(blink_node *bn)
{
  free((void *)bn);
}

void free_blink_tree_node(blink_node *bn)
{
  (void)bn;
  // TODO
}


blink_node* blink_node_descend(blink_node *bn, const void *key, uint32_t len)
{
  return (blink_node *)node_descend(bn->pn, key, len);
}

int blink_node_insert(blink_node *bn, const void *key, uint32_t len, const void *val)
{
  return node_insert(bn->pn, key, len, val);
}

void* blink_node_search(blink_node *bn, const void *key, uint32_t len)
{
  return node_search(bn->pn, key, len);
}

void blink_node_split(blink_node *old, blink_node *new_node, char *pkey, uint32_t *plen)
{
  node_split(old->pn, new_node->pn, pkey, plen);
  node_insert_fence(old->pn, new_node->pn, (void *)new_node, pkey, plen);
}

int blink_node_need_move_right(blink_node *bn, const void *key, uint32_t len)
{
  return node_need_move_right(bn->pn, key, len);
}

void blink_node_insert_infinity_key(blink_node *bn)
{
  char key[max_key_size];
  memset(key, 0xff, max_key_size);
  blink_node_insert(bn, key, max_key_size, 0);
}

#ifdef Test

void blink_node_print(blink_node *bn, int detail)
{
  node_print(bn->pn, detail);
}

#endif /* Test */
