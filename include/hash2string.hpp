#ifndef HASH2STRING_HPP
#define HASH2STRING_HPP

#include <sstream>
#include <string>

constexpr size_t HASH_BYTE_SIZE = 256 / 8;

inline std::string bytes2hex(const uint8_t *hash)
{
    const char *four_bit_2_hex = "0123456789abcdef";

    std::ostringstream out;
    for (size_t i = 0; i != HASH_BYTE_SIZE; ++i) {
        out << four_bit_2_hex[hash[i] >> 4];
        out << four_bit_2_hex[hash[i] & 0xf];
    }

    return out.str();
}

#endif  // HASH2STRING_HPP
