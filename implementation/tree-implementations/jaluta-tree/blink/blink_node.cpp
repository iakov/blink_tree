/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "blink_node.h"

blink_node *new_blink_node(uint8_t type, uint8_t level)
{
  void *memory_for_bn = (blink_node *)malloc(get_node_size());
  memset(memory_for_bn, 0, get_node_size());
  blink_node* bn = new(memory_for_bn) blink_node(type, level);
  return bn;
}

void free_blink_node(blink_node *bn)
{
#ifdef Allocator
  allocator_free((void *)bn);
#else
  free((void *)bn);
#endif
}

void free_blink_tree_node(blink_node *bn)
{
  (void)bn;
  // TODO
}


blink_node* blink_node_descend_get_corresponding_key(blink_node *bn, const void *key, uint32_t len, 
    char*& hkey, uint32_t& hlen) {
  return (blink_node *)node_descend_get_corresponding_key(bn->pn, key, len, hkey, hlen);
}

bool blink_covered(blink_node *bn, const void *key, uint32_t len) {
  return covered(bn->pn, key, len);
}

void blink_node_change_last_key(blink_node *bn, const void *key, uint32_t len) {
  node_change_last_key(bn->pn, key, len);
}

int blink_node_set_value(blink_node *bn, const void *key, uint32_t len, const void *val) {
  return node_set_value(bn->pn, key, len, val);
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

void blink_node_split(blink_node *old, blink_node *new_node, void *pkey, uint32_t *plen)
{
  node_split(old->pn, new_node->pn, (char*)pkey, plen);
  // node_insert_fence(old->pn, new_node->pn, (void *)new_node, (char*)pkey, plen);
}

void blink_node_insert_infinity_key(blink_node *bn)
{
  char key[max_key_size];
  memset(key, 0xff, max_key_size);
  ut_a(blink_node_insert(bn, key, max_key_size, 0) == 1);
}

#ifdef Test

void blink_node_print(blink_node *bn, int detail)
{
  node_print(bn->pn, detail);
}

#endif /* Test */
