/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
// TODO: remove this
#include <stdio.h>

#include "../util/bplus-tp.h"
#include "bplus_tree.h"

#define trace_w(x) trace(x, "w")
#define trace_r(x) trace(x, "r")

#define trace(x, y) tracepoint(bplus_tree, x,  (int)std::hash<std::thread::id>{}(std::this_thread::get_id()), y)

struct stack {
  bplus_node *path[max_descend_depth];
  uint32_t    depth;
};

/**
 * @brief Functions creates a new node "left" and moves and moves all keys from root there. 
 * Also it links "left" and "right" as children of root and link them together.  
 * 
 * @param root  root node of tree
 * @param key   separation key between left and right nodes
 * @param len   len of key
 * @param right node to link as right child 
 * 
 * lock state before: x-lock on root or x-lock on whole index
 * lock state after : same as before
 */
void bplus_tree::root_split(bplus_node *root, const void *key, uint32_t len, bplus_node *right) {
  #ifdef print_functions
    std::cout << "bplus_tree_root_split" << std::endl;
  #endif

  #ifdef BLINK_STATISTIC
    hight.fetch_add(1);
  #endif
  
  trace_w(root_split_start);

  ut_a(bplus_node_is_root(root));

  int level = bplus_node_get_level(root);

  bplus_node *root_left_child = new_bplus_node(Branch, level);
  int root_left_child_id = root_left_child->pn->id;
  memmove(root_left_child->pn, root->pn, get_node_size() - get_node_offset());
  root_left_child->pn->id = root_left_child_id;

  int root_id = root->pn->id;
  memset(root->pn, 0, get_node_size() - get_node_offset());
  root->pn->id = root_id;
  root->pn->type = Root;

  root_left_child->pn->next = (node*)right;
  if (!level) {
    right->pn->first_or_prev = (node*)root_left_child;
  }
  root->pn->level = level + 1;

  bplus_node_set_first(root, root_left_child);
  ut_a(bplus_node_insert(root, key, len, (const void *)right) == 1);

  int type = level ? Branch : Leaf;
  bplus_node_set_type(root_left_child, type);
  bplus_node_set_type(right, type);

  trace_w(root_split_end);
}

/**
 * @brief Function traverses down the tree to the leaf node where provided key can be.
 * Locks all nodes on the way with S locks and leaf node with X lock.
 * @param key key we are looking place of
 * @param len len of key
 * @param stack stack to store locks
 * @return bplus_node* leaf node where key can be
 * 
 * lock state before: S-locked index
 * lock state after : S-locked index, S-locked path, returned dist_level node is X locked
 */
bplus_node* bplus_tree::descend_to_leaf_s_locking_path_x_leaf(const void *key, uint32_t len, struct stack *stack) {
  #ifdef print_functions
    std::cout << "descend_to_leaf_s_locking_path_x_leaf" << std::endl;
  #endif

  trace_w(descend_to_leaf_s_locking_start);

  bplus_node* curr = root;
  stack->depth = 0;

  w_rw_lock_s_lock(&curr->latch);
  int level = bplus_node_get_level(curr);

  ut_a(level >= 0);

  if (level == 0) {
    w_rw_lock_s_unlock(&curr->latch);
    w_rw_lock_x_lock(&curr->latch);
    level = bplus_node_get_level(curr);
    if (level == 0) {
      trace_w(descend_to_leaf_s_locking_end);
      return curr;
    }
    w_rw_lock_x_unlock(&curr->latch);
    w_rw_lock_s_lock(&curr->latch);
    level = bplus_node_get_level(curr);
  }

  stack->path[stack->depth++] = curr;

  while (true) {
    ut_a(curr);

    bplus_node *child = bplus_node_descend(curr, key, len);
    if (likely(bplus_node_get_level(child) != level)) {
      --level;
    } else {
      ut_a(0);
    }

    curr = child;
    if (level == 0) {
      break;
    }
    
    w_rw_lock_s_lock(&curr->latch);
    stack->path[stack->depth++] = curr;
  }

  w_rw_lock_x_lock(&curr->latch);
  
  ut_a(curr && bplus_node_get_level(curr) == 0);
  trace_w(descend_to_leaf_s_locking_end);
  return curr;
}

/**
 * @brief Function traverses down the tree to the leaf node where provided key can be.
 * Locks all nodes on the way with S locks S lock leaf node.
 * @param key key we are looking place of
 * @param len len of key
 * @param stack stack to store path of travered path
 * @return bplus_node* leaf node where key can be
 * 
 * lock state before: S-locked index
 * lock state after : S-locked index, S-locked path, returned node is S locked
 */
bplus_node* bplus_tree::descend_to_leaf_s_locking_path_s_leaf(const void *key, uint32_t len, struct stack *stack) {
  #ifdef print_functions
    std::cout << "descend_to_leaf_s_locking_path_s_leaf" << std::endl;
  #endif

  trace_r(descending_start);

  bplus_node *curr = root;
  stack->depth = 0;

  w_rw_lock_s_lock(&curr->latch);
  int level = bplus_node_get_level(curr);

  ut_a(level >= 0);

  if (level == 0) {
    trace_r(descending_end);
    return curr;
  }

  stack->path[stack->depth++] = curr;

  while (true) {
    ut_a(curr);

    bplus_node *child = bplus_node_descend(curr, key, len);
    if (likely(bplus_node_get_level(child) != level)) {
      --level;
    } else {
      ut_a(0);
    }

    curr = child;
    if (level == 0) {
      break;
    }
    
    w_rw_lock_s_lock(&curr->latch);
    stack->path[stack->depth++] = curr;
  }

  w_rw_lock_s_lock(&curr->latch);
  ut_a(curr && bplus_node_get_level(curr) == 0);
  trace_r(descending_end);
  return curr;
}

/**
 * @brief Function traverses down the tree to the leaf node where provided key can be.
 * Doesn't lock any nodes, because X-locked on index is taken
 * @param key key we are looking place of
 * @param len len of key
 * @param stack stack to store path of travered path
 * @return bplus_node* leaf node where key can be
 * 
 * lock state before: X-locked index
 * lock state after : X-locked index
 */
bplus_node* bplus_tree::descend(const void *key, uint32_t len, struct stack *stack) {
  #ifdef print_functions
    std::cout << "descend" << std::endl;
  #endif

  trace_w(descending_no_locks_start);

  bplus_node *curr = root;
  stack->depth = 0;
  int level = bplus_node_get_level(curr);

  ut_a(level >= 0);

  if (level == 0) {
    trace_w(descending_no_locks_end);
    return curr;
  }

  stack->path[stack->depth++] = curr;

  while (true) {
    ut_a(curr);

    bplus_node *child = bplus_node_descend(curr, key, len);
    if (likely(bplus_node_get_level(child) != level)) {
      --level;
    } else {
      ut_a(0);
    }

    curr = child;
    if (level == 0) {
      break;
    }
    
    stack->path[stack->depth++] = curr;
  }

  ut_a(curr && bplus_node_get_level(curr) == 0);
  trace_w(descending_no_locks_end);
  return curr;
}


/**
 * @brief Function traverses down the tree to the leaf node where provided key can be.
 * Locks only parent of searched leaf. If leaf is root then doesn't lock anything
 * @param key key we are looking place of
 * @param len len of key
 * @param parent parent on which we took lock
 * @return bplus_node* leaf node where key can be
 * 
 * lock state before: SX-locked or index
 * lock state after : SX-locked index and X-lock on parent
 */
bplus_node* bplus_tree::descend_with_parent_x_lock(const void *key, uint32_t len, bplus_node*& parent) {
  #ifdef print_functions
    std::cout << "descend_with_parent_x_lock" << std::endl;
  #endif

  trace_w(descending_start);

  bplus_node *curr = root;
  int level = bplus_node_get_level(curr);

  if (level == 0) {
    parent = nullptr;
    trace_w(descending_end);
    return curr;
  }

  if (level == 1) {
    w_rw_lock_x_lock(&curr->latch);
    parent = curr;
  } else {
    w_rw_lock_s_lock(&curr->latch);
  }

  while (level != 0) {
    --level;

    bplus_node *child = bplus_node_descend(curr, key, len);
    if (level > 1) {
      w_rw_lock_s_lock(&child->latch);
      w_rw_lock_s_unlock(&curr->latch);
    }

    if (level == 1) {
      w_rw_lock_x_lock(&child->latch);
      w_rw_lock_s_unlock(&curr->latch);
      parent = child;
    } 
    
    curr = child;
  }

  ut_a(curr && bplus_node_get_level(curr) == 0);
  ut_a(!parent || bplus_node_get_level(parent) == 1);

  trace_w(descending_end);
  return curr;
}


static void s_unlock_path(struct stack *stack) {
  for (uint i = 0; i < stack->depth; ++i) {
    w_rw_lock_s_unlock(&stack->path[i]->latch);
  }
}

/**
 * @brief inserts key into tree if target node has enough space for it
 * 
 * @param key key to insert 
 * @param len len of key
 * @param val value to insert
 * @return int 
 * 0 - key already exists
 * 1 - key inserted
 * -1 - not enough space
 * prestate:  s-locked index
 * poststate: s-locked index
 */
int bplus_tree::optimistic_write(const void *key, uint32_t len, const void *val) {
  #ifdef print_functions
    std::cout << "bplus_tree_optimistic_write" << std::endl;
  #endif
  trace_w(optimistic_write_start);

  struct stack stack;
  bplus_node *curr = descend_to_leaf_s_locking_path_x_leaf(key, len, &stack);
  int result = bplus_node_insert(curr, key, len, val);

  s_unlock_path(&stack);
  w_rw_lock_x_unlock(&curr->latch);
  trace_w(optimistic_write_end);
  return result;
}



std::ostream& operator<<(std::ostream& stream, const bplus_tree* bt) {
  stream << "blink tree" << std::endl;
  stream << bt->root->pn << std::endl;
  
  return stream;
}

/**
 * @brief inserts key into tree if target node and its parent has not enough space for keys
 * leaf node has less space then len and its parent has less space then max key size.
 *  
 * @param key key to insert 
 * @param len len of key
 * @param val value to insert
 * @return int 
 * 0 - key already exists
 * 1 - key inserted
 *
 * prestate:  none
 * poststate: node
 */
int bplus_tree::index_lock_write(const void *key, uint32_t len, const void *val) {
  trace_w(index_lock_write_start);
  my_lock_guard lock_g(&lock, X);

  char k[max_key_size];
  uint32_t l = len;
  const void* v = val;
  memcpy(k, key, l);

  struct stack path; 
  bplus_node* node = descend(key, len, &path);

  while (true) {
    int insert_result = bplus_node_insert(node, k, l, v);
    if (insert_result == 1 or insert_result == 0) {
      trace_w(index_lock_write_end);
      return insert_result;
    }

    char new_parent_key[max_key_size];
    uint parent_key_len;
    bplus_node *new_node = new_bplus_node(bplus_node_get_type(node), bplus_node_get_level(node));

    trace_w(split_start);
    #ifdef BLINK_STATISTIC
      size.fetch_add(1);
      if (bplus_node_get_level(node) == 0) {
        leafs_amount.fetch_add(1);
      }
    #endif
    bplus_node_split(node, new_node, new_parent_key, &parent_key_len);
    trace_w(split_end);
    
    if (compare_key(new_parent_key, parent_key_len, k, l) <= 0) {
      ut_a(bplus_node_insert(new_node, k, l, v) == 1);  
    } else {
      ut_a(bplus_node_insert(node, k, l, v) == 1);
    }

    if (!path.depth) {
      root_split(node, new_parent_key, parent_key_len, new_node);
      trace_w(index_lock_write_end);
      return 1;
    } else {
      memcpy(k, new_parent_key, parent_key_len); l = parent_key_len; v = (void*)new_node;
      node = path.path[--path.depth];
    }
  }
}


/**
 * @brief inserts key into tree
 *  
 * @param key key to insert 
 * @param len len of key
 * @param val value to insert
 * @return int 
 * 0 - key already exists
 * 1 - key inserted
 *
 * prestate:  none
 * poststate: node
 */
int bplus_tree::write(const void *key, uint32_t len, const void *val) {
  #ifdef print_functions
    std::cout << "bplus_tree_write" << std::endl;
  #endif

  trace_w(write_start);
  w_rw_lock_s_lock(&lock);
  int optimistic_result = optimistic_write(key, len, val);
  w_rw_lock_s_unlock(&lock);

  if (optimistic_result >= 0) { 
    trace_w(write_end);
    return optimistic_result;
  }

  ut_a(optimistic_result == -1);

  trace_w(pessimistic_write_start);
  my_lock_guard lock_g(&lock, SX); 

  optimistic_result = optimistic_write(key, len, val);
  
  if (optimistic_result >= 0) {
    trace_w(pessimistic_write_end);  
    trace_w(write_end);
    return optimistic_result;
  }

  ut_a(optimistic_result == -1);

  bplus_node* parent = nullptr;
  bplus_node* curr = descend_with_parent_x_lock(key, len, parent);

  if (!parent or has_place(parent->pn, max_key_size)) {

    ut_a(bplus_node_get_level(curr) == 0);

    // x lock node and siblings
    bplus_node* left_simbling  = bplus_node_get_prev(curr);
    bplus_node* right_simbling = bplus_node_get_next(curr);
    if (left_simbling) {
      w_rw_lock_x_lock(&left_simbling->latch);
    }
    w_rw_lock_x_lock(&curr->latch);

    if (right_simbling) {
      w_rw_lock_x_lock(&right_simbling->latch);
    }

    char new_parent_key[max_key_size];
    uint parent_key_len;
    bplus_node *new_node = new_bplus_node(bplus_node_get_type(curr), bplus_node_get_level(curr));

    trace_w(split_start);
    #ifdef BLINK_STATISTIC
      size.fetch_add(1);
      if (bplus_node_get_level(curr) == 0) {
        leafs_amount.fetch_add(1);
      }
    #endif
    bplus_node_split(curr, new_node, new_parent_key, &parent_key_len);
    trace_w(split_end);
    
    if (compare_key(new_parent_key, parent_key_len, key, len) <= 0) {
      ut_a(bplus_node_insert(new_node, key, len, val) == 1);  
    } else {
      ut_a(bplus_node_insert(curr, key, len, val) == 1);
    }

    // unlock simblings
    if (left_simbling) {
      w_rw_lock_x_unlock(&left_simbling->latch);
    }

    if (right_simbling) {
      w_rw_lock_x_unlock(&right_simbling->latch);
    }
  
    if (!parent) {
      // new parent key goes in new root
      root_split(curr, new_parent_key, parent_key_len, new_node);
      w_rw_lock_x_unlock(&curr->latch);
      trace_w(pessimistic_write_end); 
      trace_w(write_end);
      return 1;
    } 

    w_rw_lock_x_unlock(&curr->latch);
    ut_a(bplus_node_get_level(parent) == 1);
    ut_a(bplus_node_insert(parent, new_parent_key, parent_key_len, (void*)new_node) == 1); 
    w_rw_lock_x_unlock(&parent->latch);

    trace_w(pessimistic_write_end); 
    trace_w(write_end);
    return 1;
  }

  if (parent) {
    w_rw_lock_x_unlock(&parent->latch);
  }

  int res = index_lock_write(key, len, val);

  trace_w(pessimistic_write_end);
  trace_w(write_end);
  return res;
}


/**
 * @brief reads value by key, if key exists
 *  
 * @param key key 
 * @param len len of key
 * @param val if key exists, result is put here
 * @return int 
 * 0 - no such key
 * 1 - key exists, result in val
 *
 * prestate:  none
 * poststate: node
 */
int bplus_tree::read(const void *key, uint32_t len, void **val) {
  trace_r(read_start);
  struct stack stack;
  my_lock_guard guard_l(&lock, S);

  bplus_node *curr = descend_to_leaf_s_locking_path_s_leaf(key, len, &stack);
 
  int result = bplus_node_search(curr, key, len, val);

  w_rw_lock_s_unlock(&curr->latch);
  s_unlock_path(&stack);
  
  trace_r(read_end);  
  return result;
}
