/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#include <stdlib.h>
#include <string.h>
// TODO: remove this
#include <stdio.h>

#include "../util/blink-tp.h"
#include "blink_tree.h"


#define DO_WE_USE_OPTIMISTIC_INSERT 1

#define trace_w(x) trace(x, "w")
#define trace_r(x) trace(x, "r")

#define trace(x, y) tracepoint(jaluta_tree, x,  (int)std::hash<std::thread::id>{}(std::this_thread::get_id()), y)

trace_w(split_start);

struct stack {
  blink_node *path[max_descend_depth];
  uint32_t    depth;
};

/**
 * @brief traverses the tree and returns S-locked leaf where key might be
 *
 * @param key key to find
 * @param len length of key
 * @return blink_node* node which might content the key
 *
 * prestate:  none
 * poststate: S-lock on returned value
 */
blink_node* blink_tree::descend_to_leaf_s_lock_leaf(const void *key, uint32_t len)
{
  trace_r(descending_start);

  blink_node *curr = root;
  rw_lock_s_lock(&curr->latch);
  int level = blink_node_get_level(curr);

  if (!blink_covered(curr, key, len)) {
    rw_lock_s_lock(&curr->next->latch);
    rw_lock_s_unlock(&curr->latch);
    curr = curr->next;
  }

  while (level) {
    ut_a(curr);
    blink_node *child = blink_node_descend(curr, key, len);
    if (!blink_covered(child, key, len)) {
      child = child->next;
      ut_a(child);
    }

    rw_lock_s_lock(&child->latch);
    rw_lock_s_unlock(&curr->latch);
    if (likely(blink_node_get_level(child) != level)) {
      --level;
    }
    curr = child;
  }

  ut_a(curr && blink_node_get_level(curr) == 0);

  trace_r(descending_end);
  return curr;
}


/**
 * @brief traverses the tree and returns X-locked leaf where key might be
 *
 * @param key key to find
 * @param len length of key
 * @return blink_node* node which might content the key
 *
 * prestate:  none
 * poststate: X-lock on returned value
 */
blink_node* blink_tree::descend_to_leaf_x_lock(const void *key, uint32_t len)
{
  trace_w(descending_start);

  blink_node *curr = root;

  trace_w(descend_root_block_start);
  rw_lock_s_lock(&curr->latch);
  int level = blink_node_get_level(curr);

  if (!blink_covered(curr, key, len)) {
    rw_lock_s_lock(&curr->next->latch);
    rw_lock_s_unlock(&curr->latch);
    curr = curr->next;
  }

  if (level == 0) {
    rw_lock_s_unlock(&curr->latch);
    rw_lock_x_lock(&curr->latch);

    // level might change while we not hold the lock
    level = blink_node_get_level(curr);
    if (level != 0) {
      rw_lock_x_unlock(&curr->latch);
      rw_lock_s_lock(&curr->latch);
    } else {
      trace_w(descend_root_block_end);
      trace_w(descending_end);
      return curr;
    }
  }

  trace_w(descend_root_block_end);

  while (level) {
    ut_a(curr);
    blink_node *child = blink_node_descend(curr, key, len);

    // note! only root level node may change
    // this is defenetly not root
    if (likely(blink_node_get_level(child) != level)) {
      --level;
    }

    if (level == 0) {
      rw_lock_x_lock(&child->latch);
    } else {
      rw_lock_s_lock(&child->latch);
    }

    trace_w(descend_cover_block_start);
    if (!blink_covered(child, key, len)) {
      ut_a(child->next);
      if (level == 0) {
        rw_lock_x_lock(&child->next->latch);
        rw_lock_x_unlock(&child->latch);
      } else {
        rw_lock_s_lock(&child->next->latch);
        rw_lock_s_unlock(&child->latch);
      }
      child = child->next;
    }
    trace_w(descend_cover_block_end);

    rw_lock_s_unlock(&curr->latch);

    curr = child;
  }

  ut_a(curr && blink_node_get_level(curr) == 0);
  trace_w(descending_end);
  return curr;
}


/**
 * @brief splits current node and link it to the new sibling. Note link it to the parent. Returns one of two nodes (current or new), 
 * which might contain tрe key
 *
 * @param curr SX (or X + SX if extra_x_lock) locked node, is a safe node (doesn't have unlinked sibling, etc), curr is overflowed
 * @param key key which we want to insert in the node
 * @param len length of key
 * @param extra_x_lock is there extra X-lock on the curr node
 * @return blink_node* one of two nodes produced by split which fit the key. Has same locks as curr before this function
 *
 * prestate:  curr is SX (or X + SX if extra_x_lock) latched
 * poststate: returned node is SX (or X + SX if extra_x_lock) latched
 */
blink_node* blink_tree::split(blink_node* curr, const void* key, int len, bool extra_x_lock) {
  trace_w(split_start);

  #ifdef BLINK_STATISTIC
    size.fetch_add(1);
    if (blink_node_get_level(curr) == 0) {
      leafs_amount.fetch_add(1);
    }
  #endif
  // sx to x lock
  rw_lock_x_lock(&curr->latch);

  blink_node *new_node = new_blink_node(blink_node_get_type(curr), blink_node_get_level(curr));
  rw_lock_x_lock(&new_node->latch);

  char fkey[max_key_size];
  uint32_t flen;
  new_node->next = curr->next;
  curr->next = new_node;

  char tmp_key[max_key_size];
  memcpy(tmp_key, key, len);

  blink_node_split(curr, new_node, fkey, &flen);
  if (blink_covered(curr, tmp_key, len)) {
    // converted x to sx: was sx + x, now only sx
    rw_lock_x_unlock(&curr->latch);

    rw_lock_x_unlock(&new_node->latch);
    trace_w(split_end);
    return curr;
  }
  else {
    // coverted x to sx
    rw_lock_sx_lock(&new_node->latch);
    rw_lock_x_unlock(&new_node->latch);
    if (extra_x_lock) {
      rw_lock_x_lock(&new_node->latch);
      rw_lock_x_unlock(&curr->latch);
    }
    // remove x + sx, because converted sx to x = sx + x
    rw_lock_x_unlock(&curr->latch);
    rw_lock_sx_unlock(&curr->latch);
    trace_w(split_end);
    return new_node;
  }
}

/**
 * @brief functions links unlinked child to parent and also fixes border key in parent for linked child
 *
 * @param parent parent node, which has border key and libk to linked child
 * @param linked_child child, which has unlicked right sibling, has sibling link to unlinked child
 * @param unlinked_child unlinked child
 *
 * prestate:  parent and linked_child are SX-locked, unlicked_child isn't locked
 * poststate: same as prestate
 */
void link(blink_node* parent, blink_node* linked_child, blink_node* unlinked_child) {
  trace_w(link_start);
  ut_a(unlinked_child != nullptr);

  // convert parent sx to x
  rw_lock_x_lock(&parent->latch);
  
  char k[max_key_size];
  uint32_t l;
  get_max_key(linked_child->pn, k, l);
  ut_a(blink_node_insert(parent, k, l, linked_child) == 1); 

  rw_lock_s_lock(&unlinked_child->latch);
  get_max_key(unlinked_child->pn, k, l); 
  rw_lock_s_unlock(&unlinked_child->latch);
  ut_a(blink_node_set_value(parent, k, l, unlinked_child) == 1); 

  // convert parent x back to sx
  rw_lock_x_unlock(&parent->latch);
  trace_w(link_end);
}


/**
 * @brief traverses down the tree, taking SX-lockes no more than two levels at time. Also links unlinked nodes and split overflowed ones. 
 * Returns the node corresponded to key.
 *
 * @param curr node to start traverse from
 * @param key  key to find
 * @param len  length of key
 * @return blink_node* node corresponded to the key
 *
 * prestate: SX-lock on curr
 * poststate: SX-lock on returned node
 */
blink_node* blink_tree::update_traverse(blink_node *curr, const void *key, uint32_t len) {
  trace_w(update_traverse_start);
  for (int level = blink_node_get_level(curr); level > 0; --level) {
    ut_a(curr);

    if (level == 1) {
      // convert lock to x to prevent deadlock with optimistic insert
       rw_lock_x_lock(&curr->latch);
    }

    char* hkey;
    uint32_t hlen;

    blink_node *child = blink_node_descend_get_corresponding_key(curr, key, len, hkey, hlen);
    rw_lock_sx_lock(&child->latch);
    if (!is_it_high_key(child->pn, hkey, hlen)) {
      // TODO if key has different length should 
      // take into account here delta in key size
      if (is_overflowed(curr->pn, len)) {
        curr = split(curr, hkey, hlen, level == 1);
      }
      link(curr, child, child->next);
    }
    if (blink_covered(child, key, len)) {
      if (level == 1) {
        rw_lock_x_unlock(&curr->latch);
      }
      rw_lock_sx_unlock(&curr->latch);
      curr = child;
    } else {
      ut_a(child->next != nullptr);
      rw_lock_sx_lock(&child->next->latch);
      if (level == 1) {
        rw_lock_x_unlock(&curr->latch);
      }
      rw_lock_sx_unlock(&curr->latch);
      rw_lock_sx_unlock(&child->latch);
      curr = child->next;
    }
    ut_a(blink_node_get_level(child) == level - 1);
  }

  ut_a(curr && blink_node_get_level(curr) == 0);
  trace_w(update_traverse_end);
  return curr;
}

std::ostream&  operator<<(std::ostream& stream, const blink_tree* blink_tree) {
  stream << "Blink jaluta tree" << std::endl;
  stream << blink_tree->root;
  stream << std::endl;
  return stream;
}


std::ostream&  operator<<(std::ostream& stream, const blink_node* bn) {

  blink_node* cur = const_cast<blink_node*>(bn);
  int level = blink_node_get_level(bn);
  while (level) {
    blink_node* child = (blink_node*)cur->pn->first;
    while (cur->has_next()) {
      stream << (node*)cur->pn;
      cur = cur->next;
    }
    stream << (node*)cur->pn;
    cur = child;
    --level;
  }
  while (cur->has_next()) {
    stream << (node*)cur->pn;
    cur = cur->next;
  }
  stream << (node*)cur->pn;
  stream << std::endl;
  return stream;
}


/**
 * @brief inserts value with minimal locking if there is enough space in target node.
 *
 * @param key key to insert
 * @param len length of key
 * @param val value to insert
 * @return int
 * -1 if not enough space in target node
 * 0 if value exists
 * 1 if insert
 *
 * prestate: node
 * poststate: node
 */
int blink_tree::optimistic_write(const void *key, uint32_t len, const void *val) {
  trace_w(optimistic_write_start);
  blink_node *curr = descend_to_leaf_x_lock(key, len);
  if (is_overflowed(curr->pn, len)) {
    rw_lock_x_unlock(&curr->latch);
    trace_w(optimistic_write_end);    
    return -1;
  } 

  int insert_result = -1;
  if (blink_covered(curr, key, len)) {
    insert_result = blink_node_insert(curr, key, len, val);
  } 
  rw_lock_x_unlock(&curr->latch);

  trace_w(optimistic_write_end);
  return insert_result;
}


/**
 * @brief inserts value into the tree. First tries to insert optimistically. If it fails then inserts pessimistic.
 * Pessimistic inserts also may produce unlinked children or fix previous unlinked children.
 *
 * @param key key to insert
 * @param len length of key
 * @param val value to insert
 * @return int
 * -1 if not enough space in target node
 * 0 if value exists
 * 1 if insert
 *
 * prestate: node
 * poststate: node
 */
int blink_tree::write(const void *key, uint32_t len, const void *val) {
  trace_w(write_start);   
  if (DO_WE_USE_OPTIMISTIC_INSERT) {
    if (int result = optimistic_write(key, len, val) >= 0) {
      trace_w(write_end);   
      return result;
    }
  }

  trace_w(pessimistic_write_start);   
  // std::cout << bt << std::endl;
  blink_node *curr = root;

  trace_w(pessimistic_root_wait_start); 
  rw_lock_sx_lock(&curr->latch);
  trace_w(pessimistic_root_wait_end); 
  
  if (curr->has_next()) {
    rw_lock_sx_lock(&curr->next->latch);
    curr = increase_tree_hight(key, len);
  }

  curr = update_traverse(curr, key, len);
  if (is_overflowed(curr->pn, len)) {
    curr = split(curr, key, len, false);
  }

  // upgrade sx lock to x mode
  rw_lock_x_lock(&curr->latch);
  int insert_result = blink_node_insert(curr, key, len, val); 
  
  // remove sx + x lock
  rw_lock_x_unlock(&curr->latch);
  rw_lock_sx_unlock(&curr->latch);


  trace_w(pessimistic_write_end);
  trace_w(write_end);     
  return insert_result;
}




/**
 * @brief increases tree high by making root right sibling the right child of root and moves content of root
 * to the left child. Also links them correctly. Returns the one of two root children, which correspond to key.
 *
 * @param key key to pick the root child to return
 * @param len length of key
 * @return blink_node* sx locked
 *
 * prestate: root node and it simbling are SX-locked
 * postate:  returned node is SX-locked
 */
blink_node* blink_tree::increase_tree_hight(const void* key, int len) {

  trace_w(root_split_start);  
  #ifdef BLINK_STATISTIC
    hight.fetch_add(1);
  #endif 
  ut_a(root->next != nullptr);

  // sx to x
  rw_lock_x_lock(&root->latch);

  blink_node* root_right_simbling = root->next;
  ut_a(root_right_simbling != nullptr);

  int level = blink_node_get_level(root);

  // rw_lock_x_lock(&bt->storage_map_lock);

  blink_node *root_left_child = new_blink_node(blink_node_get_type(root), level);
  rw_lock_x_lock(&root_left_child->latch);

  // move all content. Restore ids.
  int root_left_child_id = root_left_child->pn->id;
  memmove(root_left_child->pn, root->pn, get_node_size() - get_node_offset());
  root_left_child->pn->id = root_left_child_id;

  int root_id = root->pn->id;
  memset(root->pn, 0, get_node_size() - get_node_offset());
  root->pn->id = root_id;
  root->pn->type = Root;

  root_left_child->next = root_right_simbling;
  root->next = nullptr;
  root->pn->level = level + 1;

  blink_node_set_first(root, root_left_child);

  char fkey[max_key_size];
  uint32_t flen;
  get_max_key(root_left_child->pn, fkey, flen);

  ut_a(blink_node_insert(root, fkey, flen, (const void *)root_right_simbling) == 1);
  blink_node_insert_infinity_key(root);

  int type = level ? Branch : Leaf;
  blink_node_set_type(root_left_child, type);
  blink_node_set_type(root_right_simbling, type);

  // rw_lock_x_unlock(&bt->storage_map_lock);

  rw_lock_x_unlock(&root->latch);
  rw_lock_sx_unlock(&root->latch);

  if (blink_covered(root_left_child, key, len)) {
    rw_lock_sx_unlock(&root_right_simbling->latch);

    // x to sx
    rw_lock_sx_lock(&root_left_child->latch);
    rw_lock_x_unlock(&root_left_child->latch);
    trace_w(root_split_end);
    return root_left_child;
  } else {
    rw_lock_x_unlock(&root_left_child->latch);
    trace_w(root_split_end);     
    return root_right_simbling;
  }
    
}


/**
 * @brief traverses the tree and looks up for key. If it exists, then put
 * corresponding value into val.
 *
 * @param key key to find
 * @param len length of key
 * @param val to put value in, if it is found
 * @return int
 * 0 - no such key
*  1 - key found, value is in val
*
*  prestate: none
*  poststate: none
 */
int blink_tree::read(const void *key, uint32_t len, void **val) {
  trace_r(read_start);  
  blink_node *curr = descend_to_leaf_s_lock_leaf(key, len);

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
      blink_node *next = curr->next;
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


            
