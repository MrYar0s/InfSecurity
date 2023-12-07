#include "blake256.hpp"
#include "hash2string.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

BLAKE256::BLAKE256()
{
    reset();
}

std::string BLAKE256::operator()(const void *data, size_t num_bytes)
{
    uint8_t out[32] = {};
    reset();
    getHash(out, reinterpret_cast<const uint8_t *>(data), num_bytes);
    return bytes2hex(out);
}

void BLAKE256::reset()
{
    for (uint32_t i = 0; i < 8; i++) {
        h_[i] = ivals[i];
    }
    t_[0] = t_[1] = nullt_ = 0;
    s_[0] = s_[1] = s_[2] = s_[3] = 0;
    for (uint32_t i = 0; i < 64; i++) {
        m_[i] = 0;
    }
}

void BLAKE256::round(uint32_t v[16], uint32_t m[16], uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint32_t i)
{
    v[a] += (m[sigma[i % 10][e]] ^ u[sigma[i % 10][e + 1]]) + v[b];
    v[d] = rotate<uint32_t, 16>(v[d] ^ v[a]);
    v[c] += v[d];
    v[b] = rotate<uint32_t, 12>(v[b] ^ v[c]);
    v[a] += (m[sigma[i % 10][e + 1]] ^ u[sigma[i % 10][e]]) + v[b];
    v[d] = rotate<uint32_t, 8>(v[d] ^ v[a]);
    v[c] += v[d];
    v[b] = rotate<uint32_t, 7>(v[b] ^ v[c]);
}

void BLAKE256::compress(const uint8_t *block)
{
    uint32_t v[16], m[16];

    for (uint32_t i = 0; i < 16; ++i) {
        m[i] = U8TO32_BIG(block + i * 4);
    }

    for (uint32_t i = 0; i < 8; ++i) {
        v[i] = h_[i];
    }

    v[8] = s_[0] ^ u[0];
    v[9] = s_[1] ^ u[1];
    v[10] = s_[2] ^ u[2];
    v[11] = s_[3] ^ u[3];
    v[12] = u[4];
    v[13] = u[5];
    v[14] = u[6];
    v[15] = u[7];

    if (!nullt_) {
        v[12] ^= t_[0];
        v[13] ^= t_[0];
        v[14] ^= t_[1];
        v[15] ^= t_[1];
    }

    for (uint32_t i = 0; i < 14; ++i) {
        round(v, m, 0, 4, 8, 12, 0, i);
        round(v, m, 1, 5, 9, 13, 2, i);
        round(v, m, 2, 6, 10, 14, 4, i);
        round(v, m, 3, 7, 11, 15, 6, i);

        round(v, m, 0, 5, 10, 15, 8, i);
        round(v, m, 1, 6, 11, 12, 10, i);
        round(v, m, 2, 7, 8, 13, 12, i);
        round(v, m, 3, 4, 9, 14, 14, i);
    }

    for (uint32_t i = 0; i < 16; ++i) {
        h_[i % 8] ^= v[i];
    }

    for (uint32_t i = 0; i < 8; ++i) {
        h_[i] ^= s_[i % 4];
    }
}

void BLAKE256::update(const uint8_t *in, int inlen)
{
    int left = buflen_;
    int to_fill = 64 - left;

    if (left && (inlen >= to_fill)) {
        memcpy(m_ + left, in, to_fill);
        t_[0] += 512;

        if (t_[0] == 0) {
            t_[1]++;
        }

        compress(m_);
        in += to_fill;
        inlen -= to_fill;
        left = 0;
    }
    while (inlen >= 64) {
        t_[0] += 512;

        if (t_[0] == 0) {
            t_[1]++;
        }

        compress(in);
        in += 64;
        inlen -= 64;
    }

    if (inlen > 0) {
        memcpy(m_ + left, in, inlen);
        buflen_ = left + inlen;
    } else {
        buflen_ = 0;
    }
}

void BLAKE256::final(uint8_t *out)
{
    uint8_t msglen[8], zo = 0x01, oo = 0x81;
    int lo = t_[0] + (buflen_ << 3), hi = t_[1];

    if (lo < (buflen_ << 3)) {
        hi++;
    }

    U32TO8_BIG(msglen + 0, hi);
    U32TO8_BIG(msglen + 4, lo);

    if (buflen_ == 55) {
        t_[0] -= 8;
        update(&oo, 1);
    } else {
        if (buflen_ < 55) {
            if (!buflen_) {
                nullt_ = 1;
            }

            t_[0] -= 440 - (buflen_ << 3);
            update(padding, 55 - buflen_);
        } else {
            t_[0] -= 512 - (buflen_ << 3);
            update(padding, 64 - buflen_);
            t_[0] -= 440;
            update(padding + 1, 55);
            nullt_ = 1;
        }

        update(&zo, 1);
        t_[0] -= 8;
    }

    t_[0] -= 64;
    update(msglen, 8);
    U32TO8_BIG(out + 0, h_[0]);
    U32TO8_BIG(out + 4, h_[1]);
    U32TO8_BIG(out + 8, h_[2]);
    U32TO8_BIG(out + 12, h_[3]);
    U32TO8_BIG(out + 16, h_[4]);
    U32TO8_BIG(out + 20, h_[5]);
    U32TO8_BIG(out + 24, h_[6]);
    U32TO8_BIG(out + 28, h_[7]);
}

void BLAKE256::getHash(uint8_t *out, const uint8_t *in, uint64_t inlen)
{
    update(in, inlen);
    final(out);
}

void blake256_test()
{
    int i, v;
    uint8_t in[72], out[32];
    uint8_t test1[] = {0x0c, 0xe8, 0xd4, 0xef, 0x4d, 0xd7, 0xcd, 0x8d, 0x62, 0xdf, 0xde, 0xd9, 0xd4, 0xed, 0xb0, 0xa7,
                       0x74, 0xae, 0x6a, 0x41, 0x92, 0x9a, 0x74, 0xda, 0x23, 0x10, 0x9e, 0x8f, 0x11, 0x13, 0x9c, 0x87};
    uint8_t test2[] = {0xd4, 0x19, 0xba, 0xd3, 0x2d, 0x50, 0x4f, 0xb7, 0xd4, 0x4d, 0x46, 0x0c, 0x42, 0xc5, 0x59, 0x3f,
                       0xe5, 0x44, 0xfa, 0x4c, 0x13, 0x5d, 0xec, 0x31, 0xe2, 0x1b, 0xd9, 0xab, 0xdc, 0xc2, 0x2d, 0x41};
    memset(in, 0, 72);
    BLAKE256 blake;
    blake.getHash(out, in, 1);
    v = 0;

    for (i = 0; i < 32; ++i) {
        if (out[i] != test1[i])
            v = 1;
    }

    if (v)
        printf("test 1 error\n");

    blake.reset();
    blake.getHash(out, in, 72);
    v = 0;

    for (i = 0; i < 32; ++i) {
        if (out[i] != test2[i])
            v = 1;
    }

    if (v)
        printf("test 2 error\n");
}
