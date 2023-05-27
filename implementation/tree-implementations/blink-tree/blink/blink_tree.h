/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#ifndef _blink_tree_h_
#define _blink_tree_h_

#include <pthread.h>

#include "node.h"
#include "mapping_array.h"
#include "../../../util/abstract_tree.h"

class blink_tree : public abstract_tree {
public:
  int write(const void *key, uint32_t len, const void *val);
  int read(const void *key, uint32_t len, void **val);

  blink_tree() : blink_tree((uint32_t(1)) << 12){}
  blink_tree(uint32_t node_size) {
    set_node_size(node_size);
    root = new_blink_node(Root, 0);
    uint32_t offset = (char *)(&(root->pn)) - (char *)(&(root->latch));
    set_node_offset(offset);
    blink_node_insert_infinity_key(root);

    #ifdef BLINK_STATISTIC
      hight.store(1);
      size.store(1);
    #endif 
  }

private:
  blink_node *root;

  void root_split(blink_node *left, const void *key, uint32_t len, blink_node *right);
  blink_node* descend_to_leaf(const void *key, uint32_t len, struct stack *stack);
  blink_node* descend_to_leaf_write(const void *key, uint32_t len, struct stack *stack);
};


#endif /* _blink_tree_h_ */