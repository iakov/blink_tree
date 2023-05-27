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


#include "../util/blink-tp.h"
#include "../palm/allocator.h"
#include "blink_tree.h"


#define trace_w(x) trace(x, "w")
#define trace_r(x) trace(x, "r")

#define trace(x, y) tracepoint(classic_blink_tree, x,  (int)std::hash<std::thread::id>{}(std::this_thread::get_id()), y)

struct stack {
  blink_node *path[max_descend_depth];
  uint32_t    depth;
};

void blink_tree::root_split(blink_node *left, const void *key, uint32_t len, blink_node *right) {
  trace_w(root_split_start);
  #ifdef BLINK_STATISTIC
    hight.fetch_add(1);
  #endif
  ut_a(blink_node_is_root(left));

  int level = blink_node_get_level(left);
  blink_node *new_root = new_blink_node(blink_node_get_type(left), level + 1);

  blink_node_insert_infinity_key(new_root);

  blink_node_set_first(new_root, left);
  ut_a(blink_node_insert(new_root, key, len, (const void *)right) == 1);

  int type = level ? Branch : Leaf;
  blink_node_set_type(left, type);
  blink_node_set_type(right, type);

  // it's ok to use `relaxed` operation, but it doesn't matter
  __atomic_store(&root, &new_root, __ATOMIC_RELEASE);
  trace_w(root_split_end);
}

blink_node* blink_tree::descend_to_leaf(const void *key, uint32_t len, struct stack *stack) {
  trace_r(descending_start);
  blink_node *curr;
  stack->depth = 0;

  // acquire the latest root, it's ok to be stale if it changes right after
  // actually it's also ok to use `relaxed` operation
  __atomic_load(&root, &curr, __ATOMIC_ACQUIRE);

  // we can read `level` without lock this node since a node's level never changes
  int level = blink_node_get_level(curr);

  while (level) {
    ut_a(curr);
    rw_lock_s_lock(&curr->latch);
    blink_node *child = blink_node_descend(curr, key, len);
    rw_lock_s_unlock(&curr->latch);
    if (likely(blink_node_get_level(child) != level)) {
      stack->path[stack->depth++] = curr;
      --level;
    }
    curr = child;
  }

  ut_a(curr && blink_node_get_level(curr) == 0);

  trace_r(descending_end);
  return curr;
}

blink_node* blink_tree::descend_to_leaf_write(const void *key, uint32_t len, struct stack *stack) {
  trace_w(descending_start);
  blink_node *curr;
  stack->depth = 0;

  // acquire the latest root, it's ok to be stale if it changes right after
  // actually it's also ok to use `relaxed` operation
  __atomic_load(&root, &curr, __ATOMIC_ACQUIRE);

  // we can read `level` without lock this node since a node's level never changes
  int level = blink_node_get_level(curr);

  while (level) {
    ut_a(curr);
    rw_lock_s_lock(&curr->latch);
    blink_node *child = blink_node_descend(curr, key, len);
    rw_lock_s_unlock(&curr->latch);
    if (likely(blink_node_get_level(child) != level)) {
      stack->path[stack->depth++] = curr;
      --level;
    }
    curr = child;
  }

  ut_a(curr && blink_node_get_level(curr) == 0);

  trace_w(descending_end);
  return curr;
}

// Reference: Efficient Locking for Concurrent Operations on B-Trees
int blink_tree::write(const void *key, uint32_t len, const void *val) {
  trace_w(write_start);
  struct stack stack;
  blink_node *curr = descend_to_leaf_write(key, len, &stack);

  rw_lock_x_lock(&curr->latch);

  char fkey[max_key_size];
  uint32_t flen;
  void *k = (void *)key;
  uint32_t l = len;
  void *v = (void *)val;

  for (;;) {
    switch (blink_node_insert(curr, k, l, v)) {
    case 0: { // key already exists
      ut_a(blink_node_get_level(curr) == 0);
      rw_lock_x_unlock(&curr->latch);
      trace_w(write_end);
      return 0;
    }
    case 1:
      // key insert succeed
      rw_lock_x_unlock(&curr->latch);
      trace_w(write_end);
      return 1;
    case -1: { // node needs to split
      // a normal split
      trace_w(split_start);
      blink_node *new_node = new_blink_node(blink_node_get_type(curr), blink_node_get_level(curr));
      #ifdef BLINK_STATISTIC
        size.fetch_add(1);
        if (blink_node_get_level(curr) == 0) {
          leafs_amount.fetch_add(1);
        }
      #endif
      blink_node_split(curr, new_node, fkey, &flen);
      trace_w(split_end);

      if (blink_node_need_move_right(curr, k, l))
        ut_a(blink_node_insert(new_node, k, l, v) == 1);
      else
        ut_a(blink_node_insert(curr, k, l, v) == 1);

      memcpy(k, fkey, flen); l = flen; v = (void *)new_node;

      // promote to parent
      if (stack.depth) {
        blink_node *parent = stack.path[--stack.depth];
        // we can unlock `curr` first, but to be safe just lock `parent` first
        rw_lock_x_lock(&parent->latch);
        rw_lock_x_unlock(&curr->latch);
        curr = parent;
      } else {

        // bug: another thread could already increased tree root and previous root node can already overflowed again
        root_split(curr, k, len, new_node);
        rw_lock_x_unlock(&curr->latch);
        trace_w(write_end);  
        return 1;
      }
      break;
    }
    case -3: {
      // need to move to right
      blink_node *next = blink_node_get_next(curr);
      rw_lock_x_lock(&next->latch);
      rw_lock_x_unlock(&curr->latch);
      curr = next;
      break;
    }
    default: ut_a(0);
    }
  }
}

// Reference: Efficient Locking for Concurrent Operations on B-Trees
int blink_tree::read(const void *key, uint32_t len, void **val) {
  trace_r(read_start);
  struct stack stack;
  blink_node *curr = descend_to_leaf(key, len, &stack);
  rw_lock_s_lock(&curr->latch);

  void *ret;
  for (;;) {
    switch ((int64_t)(ret = blink_node_search(curr, key, len))) {
    case  0: { // key not exists
      rw_lock_s_unlock(&curr->latch);
      *val = 0;
      trace_r(read_end);
      return 0;
    }
    // move to right leaf
    case -1: {
      blink_node *next = blink_node_get_next(curr);
      rw_lock_s_lock(&next->latch);
      rw_lock_s_unlock(&curr->latch);
      curr = next;
      break;
    }
    default:
      rw_lock_s_unlock(&curr->latch);
      *val = ret;
      trace_r(read_end);
      return 1;
    }
  }
}
