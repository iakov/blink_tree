#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <atomic>
#include <mutex>
#include <cstring>
#include <iostream>

#include "random_key_generator.h"
#define key_type ulong 

using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;

constexpr bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

static key_type byteswap(key_type x) {
        key_type tmp = 0;
        for (uint i = 0; i < sizeof(key_type); ++i) {
                (reinterpret_cast<char*>(&tmp))[i] = (reinterpret_cast<char*>(&x))[sizeof(key_type) - i - 1];
        }
        return tmp;
}

namespace random_util {
    static std::atomic<key_type>* thread_sync_value = nullptr;
    static std::mutex sync_value_init_m;

    random_key_generator::random_key_generator(int _key_lenght, int _seed): key_lenght(_key_lenght), seed(_seed) {
        data = new std::vector<unsigned char>(_key_lenght, 0);
    };
    random_key_generator::random_key_generator(int _key_lenght): key_lenght(_key_lenght), seed(0){};
    random_key_generator::~random_key_generator(){
        delete(data);
    };  
    uint64_t* random_key_generator::next_read(){ return next_write(); };

    class uniform_key_generator : public random_key_generator {
    public:
        uniform_key_generator(int _key_lenght, int _seed) : random_key_generator(_key_lenght, _seed) {
            rbe.seed(_seed);
        }
        
        uint64_t* next_write() {
            std::generate(begin(*data), end(*data), std::ref(rbe));
            return reinterpret_cast<uint64_t *>(&data->at(0));
        }
    private:
        random_bytes_engine rbe;
    };

    class autoincrement_generator : public random_key_generator {
    private:
        const bool is_big_endianness; 
    public:
        autoincrement_generator(int _key_lenght, int _seed) : random_key_generator(_key_lenght, _seed), is_big_endianness(is_big_endian()) {
            std::lock_guard<std::mutex> lg(sync_value_init_m);
            if (thread_sync_value == nullptr) {
                thread_sync_value = new std::atomic<key_type>(1);
            }
            srand((unsigned) time(nullptr));
        }

        uint64_t* next_write() {
            key_type value = thread_sync_value->fetch_add(1);
            // std::cout << "value: " << value << std::endl;
            if (!is_big_endianness) {
                value = byteswap(value);
            }
            memcpy(reinterpret_cast<uint64_t *>(&data->at(0)), &value, sizeof(key_type));
            return reinterpret_cast<uint64_t *>(&data->at(0));
        }

        uint64_t* next_read() {
            key_type value = rand() % thread_sync_value->load();
            if (!is_big_endianness) {
                value = byteswap(value);
            }
            memcpy(reinterpret_cast<uint64_t *>(&data->at(0)), &value, sizeof(key_type));
            return reinterpret_cast<uint64_t *>(&data->at(0));
        }
    };

    class shared_autoincrement_generator : public random_key_generator {
        int value;
        const bool is_big_endianness;
        int initial_value;
    public:
        shared_autoincrement_generator(int _key_lenght, int _seed, long key_gap) : random_key_generator(_key_lenght, _seed), is_big_endianness(is_big_endian()) {
            std::lock_guard<std::mutex> lg(sync_value_init_m);
            if (thread_sync_value == nullptr) {
                thread_sync_value = new std::atomic<key_type>(0);
            }
            initial_value = key_gap;
            value = thread_sync_value->fetch_add(key_gap);
        }
        shared_autoincrement_generator(int _key_lenght, int _seed) : shared_autoincrement_generator(_key_lenght, _seed, 100000) {}

        uint64_t* next_write() {
            key_type new_key;
            if (!is_big_endianness) {
                new_key = byteswap(value);
            } else {
                new_key = value;
            }
            memcpy(reinterpret_cast<uint64_t *>(&data->at(0)), &new_key, sizeof(key_type));
            ++value;
            return reinterpret_cast<uint64_t *>(&data->at(0));
        }

        uint64_t* next_read() {
            int random_value = thread_sync_value->load() == initial_value ? initial_value
            		: rand() % (thread_sync_value->load() - initial_value) + initial_value;
            if (!is_big_endianness) {
                random_value = byteswap(random_value);
            }
            memcpy(reinterpret_cast<uint64_t *>(&data->at(0)), &random_value, sizeof(key_type));
            return reinterpret_cast<uint64_t *>(&data->at(0));
        }
    };

    std::unique_ptr<random_key_generator> get_key_generator(int key_size, int seed) {
        const char* rand_type = std::getenv("RAND_TYPE");

        if (!rand_type) {
            // printf("Using UNIFORM key generator\n");
            return std::make_unique<uniform_key_generator>(key_size, seed);
        }

        if (strcmp(rand_type, "AUTOINCREAMENT") == 0) {
            // printf("Using AUTOINCREAMENT key generator\n");
            return std::make_unique<autoincrement_generator>(key_size, seed);
        }

        if (strcmp(rand_type, "SHAREDAUTOINCREAMENT") == 0) {
            // printf("Using SHARED_AUTOINCREAMENT key generator\n");
            if (std::getenv("KEY_GAP")) {
                return std::make_unique<shared_autoincrement_generator>(key_size, seed, std::stol(std::getenv("KEY_GAP")));
            }
            return std::make_unique<shared_autoincrement_generator>(key_size, seed);
        }

        if (strcmp(rand_type, "UNIFORM") == 0) {
            // printf("Using UNIFORM key generator\n");
            return std::make_unique<uniform_key_generator>(key_size, seed);
        }

        // printf("Using UNIFORM key generator\n");
        return std::make_unique<uniform_key_generator>(key_size, seed);
    }

}
