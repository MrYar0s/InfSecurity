#include "gost.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include "hash2string.hpp"

GOST::GOST()
{
    uint32_t ax, bx, cx, dx;

    uint32_t sbox[8][16] = {
        {4, 10, 9, 2, 13, 8, 0, 14, 6, 11, 1, 12, 7, 15, 5, 3}, {14, 11, 4, 12, 6, 13, 15, 10, 2, 3, 8, 1, 0, 7, 5, 9},
        {5, 8, 1, 13, 10, 3, 4, 2, 14, 15, 12, 7, 6, 0, 9, 11}, {7, 13, 10, 1, 0, 8, 9, 15, 14, 4, 6, 12, 11, 2, 5, 3},
        {6, 12, 7, 1, 5, 15, 13, 8, 4, 10, 9, 14, 0, 3, 11, 2}, {4, 11, 10, 0, 7, 2, 1, 13, 3, 6, 8, 5, 9, 12, 15, 14},
        {13, 11, 4, 1, 3, 15, 5, 9, 0, 10, 14, 7, 6, 8, 2, 12}, {1, 15, 13, 0, 5, 7, 10, 4, 9, 2, 3, 14, 6, 11, 8, 12}};

    int i = 0;
    for (int a = 0; a < 16; a++) {
        ax = sbox[1][a] << 15;
        bx = sbox[3][a] << 23;
        cx = sbox[5][a];
        cx = (cx >> 1) | (cx << 31);
        dx = sbox[7][a] << 7;

        for (int b = 0; b < 16; b++) {
            gost_sbox_1[i] = ax | (sbox[0][b] << 11);
            gost_sbox_2[i] = bx | (sbox[2][b] << 19);
            gost_sbox_3[i] = cx | (sbox[4][b] << 27);
            gost_sbox_4[i++] = dx | (sbox[6][b] << 3);
        }
    }
}

std::string GOST::operator()(const void *data, size_t num_bytes)
{
    uint8_t out[32] = {};
    reset();
    update(reinterpret_cast<const uint8_t *>(data), num_bytes);
    final(out);
    return bytes2hex(out);
}

void GOST::round(uint32_t key1, uint32_t key2, uint32_t *r, uint32_t *l)
{
    uint32_t tmp = key1 + *r;
    *l ^= gost_sbox_1[tmp & 0xff] ^ gost_sbox_2[(tmp >> 8) & 0xff] ^ gost_sbox_3[(tmp >> 16) & 0xff] ^
          gost_sbox_4[tmp >> 24];
    tmp = key2 + *l;
    *r ^= gost_sbox_1[tmp & 0xff] ^ gost_sbox_2[(tmp >> 8) & 0xff] ^ gost_sbox_3[(tmp >> 16) & 0xff] ^
          gost_sbox_4[tmp >> 24];
}

void GOST::compress(uint32_t *block)
{
    uint32_t l, r, t, key[8], u[8], v[8], w[8], s[8];

    memcpy(u, hash_, sizeof(u));
    memcpy(v, block, sizeof(u));

    for (int i = 0; i < 8; i += 2) {
        w[0] = u[0] ^ v[0]; /* w = u xor v */
        w[1] = u[1] ^ v[1];
        w[2] = u[2] ^ v[2];
        w[3] = u[3] ^ v[3];
        w[4] = u[4] ^ v[4];
        w[5] = u[5] ^ v[5];
        w[6] = u[6] ^ v[6];
        w[7] = u[7] ^ v[7];

        key[0] = (w[0] & 0x000000ff) | ((w[2] & 0x000000ff) << 8) | ((w[4] & 0x000000ff) << 16) |
                 ((w[6] & 0x000000ff) << 24);
        key[1] =
            ((w[0] & 0x0000ff00) >> 8) | (w[2] & 0x0000ff00) | ((w[4] & 0x0000ff00) << 8) | ((w[6] & 0x0000ff00) << 16);
        key[2] =
            ((w[0] & 0x00ff0000) >> 16) | ((w[2] & 0x00ff0000) >> 8) | (w[4] & 0x00ff0000) | ((w[6] & 0x00ff0000) << 8);
        key[3] = ((w[0] & 0xff000000) >> 24) | ((w[2] & 0xff000000) >> 16) | ((w[4] & 0xff000000) >> 8) |
                 (w[6] & 0xff000000);
        key[4] = (w[1] & 0x000000ff) | ((w[3] & 0x000000ff) << 8) | ((w[5] & 0x000000ff) << 16) |
                 ((w[7] & 0x000000ff) << 24);
        key[5] =
            ((w[1] & 0x0000ff00) >> 8) | (w[3] & 0x0000ff00) | ((w[5] & 0x0000ff00) << 8) | ((w[7] & 0x0000ff00) << 16);
        key[6] =
            ((w[1] & 0x00ff0000) >> 16) | ((w[3] & 0x00ff0000) >> 8) | (w[5] & 0x00ff0000) | ((w[7] & 0x00ff0000) << 8);
        key[7] = ((w[1] & 0xff000000) >> 24) | ((w[3] & 0xff000000) >> 16) | ((w[5] & 0xff000000) >> 8) |
                 (w[7] & 0xff000000);

        r = hash_[i];
        l = hash_[i + 1];
        round(key[0], key[1], &r, &l);
        round(key[2], key[3], &r, &l);
        round(key[4], key[5], &r, &l);
        round(key[6], key[7], &r, &l);
        round(key[0], key[1], &r, &l);
        round(key[2], key[3], &r, &l);
        round(key[4], key[5], &r, &l);
        round(key[6], key[7], &r, &l);
        round(key[0], key[1], &r, &l);
        round(key[2], key[3], &r, &l);
        round(key[4], key[5], &r, &l);
        round(key[6], key[7], &r, &l);
        round(key[7], key[6], &r, &l);
        round(key[5], key[4], &r, &l);
        round(key[3], key[2], &r, &l);
        round(key[1], key[0], &r, &l);
        t = r;
        r = l;
        l = t;

        s[i] = r;
        s[i + 1] = l;

        if (i == 6)
            break;

        l = u[0] ^ u[2];
        r = u[1] ^ u[3];
        u[0] = u[2];
        u[1] = u[3];
        u[2] = u[4];
        u[3] = u[5];
        u[4] = u[6];
        u[5] = u[7];
        u[6] = l;
        u[7] = r;

        if (i == 2) {
            u[0] ^= 0xff00ff00;
            u[1] ^= 0xff00ff00;
            u[2] ^= 0x00ff00ff;
            u[3] ^= 0x00ff00ff;
            u[4] ^= 0x00ffff00;
            u[5] ^= 0xff0000ff;
            u[6] ^= 0x000000ff;
            u[7] ^= 0xff00ffff;
        }

        l = v[0];
        r = v[2];
        v[0] = v[4];
        v[2] = v[6];
        v[4] = l ^ r;
        v[6] = v[0] ^ r;
        l = v[1];
        r = v[3];
        v[1] = v[5];
        v[3] = v[7];
        v[5] = l ^ r;
        v[7] = v[1] ^ r;
    }

    u[0] = block[0] ^ s[6];
    u[1] = block[1] ^ s[7];
    u[2] = block[2] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff) ^ (s[1] & 0xffff) ^ (s[1] >> 16) ^ (s[2] << 16) ^
           s[6] ^ (s[6] << 16) ^ (s[7] & 0xffff0000) ^ (s[7] >> 16);
    u[3] = block[3] ^ (s[0] & 0xffff) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^ (s[1] << 16) ^ (s[1] >> 16) ^ (s[2] << 16) ^
           (s[2] >> 16) ^ (s[3] << 16) ^ s[6] ^ (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^ (s[7] << 16) ^
           (s[7] >> 16);
    u[4] = block[4] ^ (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[1] & 0xffff0000) ^ (s[1] >> 16) ^
           (s[2] << 16) ^ (s[2] >> 16) ^ (s[3] << 16) ^ (s[3] >> 16) ^ (s[4] << 16) ^ (s[6] << 16) ^ (s[6] >> 16) ^
           (s[7] & 0xffff) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[5] = block[5] ^ (s[0] << 16) ^ (s[0] >> 16) ^ (s[0] & 0xffff0000) ^ (s[1] & 0xffff) ^ s[2] ^ (s[2] >> 16) ^
           (s[3] << 16) ^ (s[3] >> 16) ^ (s[4] << 16) ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[6] << 16) ^ (s[6] >> 16) ^
           (s[7] & 0xffff0000) ^ (s[7] << 16) ^ (s[7] >> 16);
    u[6] = block[6] ^ s[0] ^ (s[1] >> 16) ^ (s[2] << 16) ^ s[3] ^ (s[3] >> 16) ^ (s[4] << 16) ^ (s[4] >> 16) ^
           (s[5] << 16) ^ (s[5] >> 16) ^ s[6] ^ (s[6] << 16) ^ (s[6] >> 16) ^ (s[7] << 16);
    u[7] = block[7] ^ (s[0] & 0xffff0000) ^ (s[0] << 16) ^ (s[1] & 0xffff) ^ (s[1] << 16) ^ (s[2] >> 16) ^
           (s[3] << 16) ^ s[4] ^ (s[4] >> 16) ^ (s[5] << 16) ^ (s[5] >> 16) ^ (s[6] >> 16) ^ (s[7] & 0xffff) ^
           (s[7] << 16) ^ (s[7] >> 16);

    v[0] = hash_[0] ^ (u[1] << 16) ^ (u[0] >> 16);
    v[1] = hash_[1] ^ (u[2] << 16) ^ (u[1] >> 16);
    v[2] = hash_[2] ^ (u[3] << 16) ^ (u[2] >> 16);
    v[3] = hash_[3] ^ (u[4] << 16) ^ (u[3] >> 16);
    v[4] = hash_[4] ^ (u[5] << 16) ^ (u[4] >> 16);
    v[5] = hash_[5] ^ (u[6] << 16) ^ (u[5] >> 16);
    v[6] = hash_[6] ^ (u[7] << 16) ^ (u[6] >> 16);
    v[7] = hash_[7] ^ (u[0] & 0xffff0000) ^ (u[0] << 16) ^ (u[7] >> 16) ^ (u[1] & 0xffff0000) ^ (u[1] << 16) ^
           (u[6] << 16) ^ (u[7] & 0xffff0000);

    hash_[0] = (v[0] & 0xffff0000) ^ (v[0] << 16) ^ (v[0] >> 16) ^ (v[1] >> 16) ^ (v[1] & 0xffff0000) ^ (v[2] << 16) ^
               (v[3] >> 16) ^ (v[4] << 16) ^ (v[5] >> 16) ^ v[5] ^ (v[6] >> 16) ^ (v[7] << 16) ^ (v[7] >> 16) ^
               (v[7] & 0xffff);
    hash_[1] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^ (v[1] & 0xffff) ^ v[2] ^ (v[2] >> 16) ^
               (v[3] << 16) ^ (v[4] >> 16) ^ (v[5] << 16) ^ (v[6] << 16) ^ v[6] ^ (v[7] & 0xffff0000) ^ (v[7] >> 16);
    hash_[2] = (v[0] & 0xffff) ^ (v[0] << 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^ (v[1] & 0xffff0000) ^ (v[2] << 16) ^
               (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ (v[5] >> 16) ^ v[6] ^ (v[6] >> 16) ^ (v[7] & 0xffff) ^
               (v[7] << 16) ^ (v[7] >> 16);
    hash_[3] = (v[0] << 16) ^ (v[0] >> 16) ^ (v[0] & 0xffff0000) ^ (v[1] & 0xffff0000) ^ (v[1] >> 16) ^ (v[2] << 16) ^
               (v[2] >> 16) ^ v[2] ^ (v[3] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
               (v[7] & 0xffff) ^ (v[7] >> 16);
    hash_[4] = (v[0] >> 16) ^ (v[1] << 16) ^ v[1] ^ (v[2] >> 16) ^ v[2] ^ (v[3] << 16) ^ (v[3] >> 16) ^ v[3] ^
               (v[4] << 16) ^ (v[5] >> 16) ^ v[5] ^ (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16);
    hash_[5] = (v[0] << 16) ^ (v[0] & 0xffff0000) ^ (v[1] << 16) ^ (v[1] >> 16) ^ (v[1] & 0xffff0000) ^ (v[2] << 16) ^
               v[2] ^ (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^ (v[4] >> 16) ^ v[4] ^ (v[5] << 16) ^ (v[6] << 16) ^
               (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ (v[7] >> 16) ^ (v[7] & 0xffff0000);
    hash_[6] = v[0] ^ v[2] ^ (v[2] >> 16) ^ v[3] ^ (v[3] << 16) ^ v[4] ^ (v[4] >> 16) ^ (v[5] << 16) ^ (v[5] >> 16) ^
               v[5] ^ (v[6] << 16) ^ (v[6] >> 16) ^ v[6] ^ (v[7] << 16) ^ v[7];
    hash_[7] = v[0] ^ (v[0] >> 16) ^ (v[1] << 16) ^ (v[1] >> 16) ^ (v[2] << 16) ^ (v[3] >> 16) ^ v[3] ^ (v[4] << 16) ^
               v[4] ^ (v[5] >> 16) ^ v[5] ^ (v[6] << 16) ^ (v[6] >> 16) ^ (v[7] << 16) ^ v[7];
}

void GOST::reset()
{
    memset(sum_, 0, 32);
    memset(hash_, 0, 32);
    memset(len_, 0, 32);
    memset(partial_, 0, 32);
    partial_bytes_ = 0;
}

void GOST::bytes(const uint8_t *buf, size_t bits)
{
    uint32_t a, b, c, m[8];

    int j = 0;
    c = 0;
    for (int i = 0; i < 8; i++) {
        a = ((uint32_t)buf[j]) | (((uint32_t)buf[j + 1]) << 8) | (((uint32_t)buf[j + 2]) << 16) |
            (((uint32_t)buf[j + 3]) << 24);
        j += 4;
        m[i] = a;
        b = sum_[i];
        c = a + c + sum_[i];
        sum_[i] = c;
        c = ((c < a) || (c < b)) ? 1 : 0;
    }

    compress(m);

    len_[0] += bits;
    if (len_[0] < bits) {
        len_[1]++;
    }
}

void GOST::update(const uint8_t *buf, size_t len)
{
    size_t i, j;

    i = partial_bytes_;
    j = 0;
    while (i < 32 && j < len) {
        partial_[i++] = buf[j++];
    }

    if (i < 32) {
        partial_bytes_ = i;
        return;
    }
    bytes(partial_, 256);

    while ((j + 32) < len) {
        bytes(&buf[j], 256);
        j += 32;
    }

    i = 0;
    while (j < len) {
        partial_[i++] = buf[j++];
    }
    partial_bytes_ = i;
}

void GOST::final(uint8_t *out)
{
    if (partial_bytes_ > 0) {
        memset(&partial_[partial_bytes_], 0, 32 - partial_bytes_);
        bytes(partial_, partial_bytes_ << 3);
    }

    compress(len_);
    compress(sum_);

    int j = 0;
    for (int i = 0; i < 8; i++) {
        uint32_t a = hash_[i];
        out[j] = (uint8_t)a;
        out[j + 1] = (uint8_t)(a >> 8);
        out[j + 2] = (uint8_t)(a >> 16);
        out[j + 3] = (uint8_t)(a >> 24);
        j += 4;
    }
}