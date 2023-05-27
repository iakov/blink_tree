
/**
 *    author:     UncP
 *    date:    2018-11-20
 *    license:    BSD-3
**/

#ifndef _bplus_node_h_
#define _bplus_node_h_

#include "node.h"
#include "sync0rw.h"

typedef node palm_node;

template<class P, class M>
size_t my_offsetof(const M P::*member)
{
    return (size_t) &( reinterpret_cast<P*>(0)->*member);
}

template<class P, class M>
P* my_container_of_impl(M* ptr, const M P::*member)
{
    return (P*)( (char*)ptr - my_offsetof(member));
}


// bplus node is basically a wrapper for palm node, but with a latch and a fence key
class bplus_node {
public:
  rw_lock_t      latch;
  // char           padding[(64 - (sizeof(latch)) % 64)]; // only for padding
  char           padding[64];
  palm_node      pn[1];

  bplus_node (uint8_t type, uint8_t level) {
    #ifdef UNIV_DEBUG
      rw_lock_create_func(&latch, SYNC_UNKNOWN, "node_lock", "", 0);
    #else
      rw_lock_create_func(&latch, "", 0);
    #endif
    ut_a(latch.pfs_psi == nullptr);
    node_init(pn, type | Bplus, level);
    ut_a(latch.pfs_psi == nullptr);
  }  
  ~bplus_node();
};


#define bplus_node_is_root(bn)   ((int)((bn)->pn->type | Root))
#define bplus_node_get_level(bn) ((int)((bn)->pn->level))
#define bplus_node_get_type(bn)  ((bn)->pn->type)
#define bplus_node_set_type(bn, type) ((bn)->pn->type = ((type) | Bplus))

// #define bplus_node_get_prev(bn)  ((bn)->pn->next ? (bplus_node *)((char *)(bn)->pn->first_or_prev - offset) : nullptr) // my_container_of((bn)->pn->first_or_prev, bplus_node, pn)

inline bplus_node* bplus_node_get_next(bplus_node* bn) {
  ut_a(bn->pn->level == 0); 
  return ((bplus_node *)((bn)->pn->next)); 
}

inline bplus_node* bplus_node_get_prev(bplus_node* bn) {
  ut_a(bn->pn->level == 0); 
  return ((bplus_node *)((bn)->pn->first_or_prev)); 
}

#define bplus_node_set_first(bn, fir) \
  ut_a(bn->pn->level > 0); \
  ((bn)->pn->first_or_prev = ((node *)fir));

#define node_to_bplus_node(n) ((node *)((char *)(n) - get_node_offset()))

bplus_node* new_bplus_node(uint8_t type, uint8_t level);
void free_bplus_node(bplus_node *bn);
void free_bplus_tree_node(bplus_node *bn);
bplus_node* bplus_node_descend(bplus_node *bn, const void *key, uint32_t len);
int bplus_node_insert(bplus_node *bn, const void *key, uint32_t len, const void *val);
void bplus_node_insert_infinity_key(bplus_node *bn);
int bplus_node_search(bplus_node *bn, const void *key, uint32_t len, void** val);
void bplus_node_split(bplus_node *old, bplus_node *new_node, char *pkey, uint32_t *plen);
int bplus_node_need_move_right(bplus_node *bn, const void *key, uint32_t len);

bool inline is_padding_clean(bplus_node* n) {
  for (int i = 0; i <64; ++i) {
    if (*(n->padding + i) != 0) {
      return false;
    }
  }
  return true;
}

bool inline is_psi_clean(bplus_node* n) {
  if (n->latch.pfs_psi == nullptr) {
    return true;
  } else {
    return false;
  }
}
#endif /* _bplus_node_h_ */
