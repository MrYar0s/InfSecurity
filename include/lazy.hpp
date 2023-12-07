#ifndef DUMP_HPP
#define DUMP_HPP

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <string>

inline std::string lazy_hash(const void *data, size_t num_bytes)
{
    static constexpr size_t BUF_BYTE_SIZE = 32;
    static constexpr size_t BUF_DWORD_SIZE = BUF_BYTE_SIZE / sizeof(uint64_t);

    uint64_t buf1[BUF_DWORD_SIZE] = {0xbd687521ef86239aULL, 0x4c35837f9ee91669ULL, 0xe61603164b5190baULL,
                                     0x48abc3e4a562c2ad};

    auto *buf_byte_ptr = reinterpret_cast<uint8_t *>(buf1);
    auto *data_byte_ptr = static_cast<const uint8_t *>(data);

    for (size_t i = 0, end = std::max(num_bytes, BUF_BYTE_SIZE); i != end; ++i) {
        size_t buf_idx = i % BUF_BYTE_SIZE;
        size_t data_idx = i % num_bytes;

        buf_byte_ptr[buf_idx] += data_byte_ptr[data_idx] + num_bytes - i;
    }

    static constexpr size_t SHIFT_SIZE = 11;
    uint64_t buf2[BUF_DWORD_SIZE] {};

    for (size_t i = 0; i != BUF_DWORD_SIZE; ++i) {
        buf2[i] = buf1[i] << SHIFT_SIZE;
    }

    for (size_t i = 0; i != BUF_DWORD_SIZE; ++i) {
        buf2[i] |= buf1[(i - 1) % BUF_DWORD_SIZE] >> (sizeof(uint64_t) * CHAR_BIT - SHIFT_SIZE);
    }

    for (size_t i = 0; i != BUF_DWORD_SIZE; ++i) {
        buf1[i] ^= buf2[i];
    }

    std::string result;
    result.reserve(BUF_BYTE_SIZE * 2);
    for (size_t i = 0; i != BUF_BYTE_SIZE; ++i) {
        static constexpr std::string_view dec2hex = "0123456789abcdef";
        result += dec2hex[(buf_byte_ptr[i] >> 4) & 0xf];
        result += dec2hex[buf_byte_ptr[i] & 0xf];
    }

    return result;
}

#endif  // DUMP_HPP
