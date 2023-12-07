#ifndef GOST_HPP
#define GOST_HPP

#include <cstdint>
#include <cstdlib>
#include <string>

class GOST final {
public:
    GOST();

    std::string operator()(const void *data, size_t num_bytes);

    void reset();
    void compress(uint32_t *block);
    void bytes(const uint8_t *buf, size_t bits);
    void final(uint8_t *out);
    void update(const uint8_t *buf, size_t len);

private:
    void round(uint32_t key1, uint32_t key2, uint32_t *r, uint32_t *l);

    uint32_t sum_[8];
    uint32_t hash_[8];
    uint32_t len_[8];
    uint8_t partial_[32];
    size_t partial_bytes_;

    uint32_t gost_sbox_1[256];
    uint32_t gost_sbox_2[256];
    uint32_t gost_sbox_3[256];
    uint32_t gost_sbox_4[256];
};

#endif  // GOST_HPP
