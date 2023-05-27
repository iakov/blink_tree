#include <cstdint>
#include <atomic>

class abstract_tree {
public:
    #ifdef BLINK_STATISTIC
        std::atomic_long size;
        std::atomic_long leafs_amount;
        std::atomic_int hight;
    #endif
    virtual int write(const void *key, uint32_t len, const void *val) = 0;
    virtual int read(const void *key, uint32_t len, void **val) = 0;
};