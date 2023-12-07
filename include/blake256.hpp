#ifndef BLAKE256_HPP
#define BLAKE256_HPP

#include <cstdint>
#include <string>
#include <array>

#define U8TO32_BIG(p) \
    (((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) | ((uint32_t)((p)[2]) << 8) | ((uint32_t)((p)[3])))

#define U32TO8_BIG(p, v)           \
    (p)[0] = (uint8_t)((v) >> 24); \
    (p)[1] = (uint8_t)((v) >> 16); \
    (p)[2] = (uint8_t)((v) >> 8);  \
    (p)[3] = (uint8_t)((v));

#define U8TO64_BIG(p) (((uint64_t)U8TO32_BIG(p) << 32) | (uint64_t)U8TO32_BIG((p) + 4))

#define U64TO8_BIG(p, v)                    \
    U32TO8_BIG((p), (uint32_t)((v) >> 32)); \
    U32TO8_BIG((p) + 4, (uint32_t)((v)));

template <typename Type, uint8_t N>
Type rotate(Type val)
{
    return (val << (32 - N)) | (val >> N);
}

class BLAKE256 final {
public:
    BLAKE256();

    std::string operator()(const void *data, size_t num_bytes);

    void reset();
    void compress(const uint8_t *block);
    void update(const uint8_t *in, int inlen);
    void final(uint8_t *out);
    void getHash(uint8_t *out, const uint8_t *in, uint64_t inlen);

private:
    std::array<uint32_t, 8> h_;
    std::array<uint32_t, 4> s_;
    std::array<uint32_t, 2> t_;
    uint8_t m_[64];

    bool nullt_;
    int buflen_;

    inline void round(uint32_t v[16], uint32_t m[16], uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e,
                      uint32_t i);

    const uint8_t sigma[10][16] = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
        {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4}, {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
        {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13}, {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
        {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11}, {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
        {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5}, {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0}};

    const uint32_t u[16] = {0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344, 0xa4093822, 0x299f31d0,
                            0x082efa98, 0xec4e6c89, 0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
                            0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917};

    uint32_t ivals[8] = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
                         0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};

    const uint8_t padding[129] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

#endif  // BLAKE256_HPP