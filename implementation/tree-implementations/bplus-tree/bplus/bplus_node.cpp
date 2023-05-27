/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "bplus_node.h"


#define my_container_of(ptr, type, member) \
     my_container_of_impl (ptr, &type::member)


#define get_ptr(n, off) ((char *)n->data + off)
// get the length of the key
#define get_len(n, off) ((uint32_t)(*(len_t *)get_ptr(n, off)))
#define get_key(n, off) (get_ptr(n, off) + key_byte)
#define get_val(n, off) ((void *)(*(val_t *)(get_key(n, off) + get_len(n, off))))
#define node_index(n)   ((index_t *)((char *)n + (get_node_size() - get_node_offset() - (n->keys * index_byte))))


bplus_node *new_bplus_node(uint8_t type, uint8_t level)
{
  void *memory_for_bn = (bplus_node *)malloc(get_node_size());
  memset(memory_for_bn, 0, get_node_size());
  bplus_node* bn = new(memory_for_bn) bplus_node(type, level);
  return bn;
}

void free_bplus_node(bplus_node *bn)
{
  free((void *)bn);
}

void free_bplus_tree_node(bplus_node *bn)
{
  (void)bn;
  // TODO
}


bplus_node* bplus_node_descend(bplus_node *bn, const void *key, uint32_t len)
{
  return (bplus_node *)node_descend(bn->pn, key, len);
}

int bplus_node_insert(bplus_node *bn, const void *key, uint32_t len, const void *val)
{
  return node_insert(bn->pn, key, len, val);
}

int bplus_node_search(bplus_node *bn, const void *key, uint32_t len, void** val)
{
  return node_search(bn->pn, key, len, val);
}

bplus_node::~bplus_node(){
  node* n = this->pn;
  if (n->level) {
    // delete( n->first_or_prev );
    assert(n->keys);
    index_t *index = node_index(n);
    for (uint32_t i = 0; i < n->keys; ++i) {
      node *child = (node *)get_val(n, index[i]);
      delete(node_to_bplus_node(child));
    }
  }   
}

void bplus_node_split(bplus_node *old, bplus_node *new_node, char *pkey, uint32_t *plen)
{
  node_split(old->pn, new_node->pn, pkey, plen);
  // node_insert_fence(old->pn, new_node->pn, (void *)new_node, pkey, plen);
}

int bplus_node_need_move_right(bplus_node *bn, const void *key, uint32_t len)
{
  return node_need_move_right(bn->pn, key, len);
}

void bplus_node_insert_infinity_key(bplus_node *bn)
{
  char key[max_key_size];
  memset(key, 0xff, max_key_size);
  bplus_node_insert(bn, key, max_key_size, 0);
}


