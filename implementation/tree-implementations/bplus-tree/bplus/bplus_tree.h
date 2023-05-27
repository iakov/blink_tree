/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#ifndef _bplus_tree_h_
#define _bplus_tree_h_

#include "bplus_node.h"
#include "../../../util/abstract_tree.h"

#ifndef WITHOUT_LOCKS
  #define w_rw_lock_s_lock rw_lock_s_lock
  #define w_rw_lock_s_unlock rw_lock_s_unlock
  #define w_rw_lock_x_lock rw_lock_x_lock
  #define w_rw_lock_x_unlock rw_lock_x_unlock
  #define w_rw_lock_sx_lock rw_lock_sx_lock
  #define w_rw_lock_sx_unlock rw_lock_sx_unlock
#else 
  #define w_rw_lock_s_lock fake_lock
  #define w_rw_lock_s_unlock fake_lock
  #define w_rw_lock_x_lock fake_lock
  #define w_rw_lock_x_unlock fake_lock
  #define w_rw_lock_sx_lock fake_lock
  #define w_rw_lock_sx_unlock fake_lock
#endif


inline void fake_lock(rw_lock_t*) {
  return;
}

class bplus_tree : public abstract_tree {
public:
  int write(const void *key, uint32_t len, const void *val);
  int read(const void *key, uint32_t len, void **val);
  bplus_tree() : bplus_tree(((uint32)1) << 12) {}
  bplus_tree(uint32 node_size) {
    #ifdef UNIV_DEBUG
      rw_lock_create_func(&lock, SYNC_UNKNOWN, "index_lock", "", 0);
    #else
      rw_lock_create_func(&lock, "", 0);
    #endif
    set_node_size(node_size);
    
    root = new_bplus_node(Root, 0);
    uint32_t offset = (char *)(&(root->pn)) - (char *)(&(root->latch)); // offsetof(bplus_node, pn);
    set_node_offset(offset);
    bplus_node_insert_infinity_key(root);

    #ifdef BLINK_STATISTIC
      hight.store(1);
      size.store(1);
    #endif 
  }

private: 
  bplus_node *root;
  rw_lock_t lock;

  void root_split(bplus_node *root, const void *key, uint32_t len, bplus_node *right);
  bplus_node* descend_with_parent_x_lock(const void *key, uint32_t len, bplus_node*& parent);
  bplus_node* descend_to_leaf_s_locking_path_s_leaf(const void *key, uint32_t len, struct stack *stack);
  bplus_node* descend(const void *key, uint32_t len, struct stack *stack);

  int optimistic_write(const void *key, uint32_t len, const void *val);
  bplus_node* descend_to_leaf_s_locking_path_x_leaf(const void *key, uint32_t len, struct stack *stack);
  int index_lock_write(const void *key, uint32_t len, const void *val);
  friend std::ostream&  operator<<(std::ostream& stream, const bplus_tree* n);
};


enum my_lock_mode {
  S,
  SX,
  X
};

class my_lock_guard
{
private:
  enum my_lock_mode mode;
  rw_lock_t* lock;
public:
  my_lock_guard(rw_lock_t* lock_to_lock, my_lock_mode lock_mode) : mode(lock_mode), lock(lock_to_lock) {
    switch (mode)
    {
      case S:
        rw_lock_s_lock(lock);
      break;
      case SX:
        rw_lock_sx_lock(lock);
      break;
      case X:
        rw_lock_x_lock(lock);
      break;   
    }
  }

  ~my_lock_guard() {
    switch (mode)
    {
      case S:
        rw_lock_s_unlock(lock);
      break;
      case SX:
        rw_lock_sx_unlock(lock);
      break;
      case X:
        rw_lock_x_unlock(lock);
      break;   
    }
  }
};


#endif /* _bplus_tree_h_ */