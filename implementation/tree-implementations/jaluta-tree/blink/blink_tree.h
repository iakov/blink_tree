/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#ifndef _blink_tree_h_
#define _blink_tree_h_

#include <pthread.h>

#include "blink_node.h"
#include "../../../util/abstract_tree.h"

class blink_tree : public abstract_tree {
public:
  blink_node *root;
  rw_lock_t storage_map_lock;

  blink_tree() : blink_tree((uint32_t(1)) << 12){}
  blink_tree(uint32_t node_size) {
    rw_lock_create_func(&storage_map_lock, "", 0);
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

  int write(const void *key, uint32_t len, const void *val);
  int read(const void *key, uint32_t len, void **val);

private:
  blink_node* increase_tree_hight(const void* key, int len);
  blink_node* descend_to_leaf_s_lock_leaf(const void *key, uint32_t len);
  blink_node* descend_to_leaf_x_lock(const void *key, uint32_t len);
  blink_node* split(blink_node* curr, const void* k, int l, bool extra_x_lock);
  blink_node* update_traverse(blink_node *curr, const void *key, uint32_t len);
  int optimistic_write(const void *key, uint32_t len, const void *val);

  friend std::ostream& operator<< (std::ostream& stream, const blink_tree* blink_tree);
};

#endif /* _blink_tree_h_ */