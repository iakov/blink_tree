#include <memory>

namespace random_util {
    class random_key_generator {
    public:
        random_key_generator(int _key_lenght, int _seed);
        random_key_generator(int _key_lenght);
        virtual uint64_t* next_write() = 0;
        virtual uint64_t* next_read();
        virtual ~random_key_generator();
    protected:
        std::vector<unsigned char>* data;
    private:
        int key_lenght;
        int seed;
    };

    std::unique_ptr<random_key_generator> get_key_generator(int key_size, int seed);
}