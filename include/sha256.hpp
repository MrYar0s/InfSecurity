#ifndef SHA256_HPP
#define SHA256_HPP

#include <cstdint>
#include <string>

struct SHA256 final {
    // split into 64 byte blocks (=> 512 bits)
    static constexpr size_t BLOCK_SIZE = 512 / 8;
    // hash is 32 bytes long
    static constexpr size_t HASH_SIZE = 32;

private:
    // size of processed data in bytes
    uint64_t m_num_bytes = 0;
    // valid bytes in m_buffer
    size_t m_buffer_size = 0;
    // bytes not processed yet
    uint8_t m_buffer[BLOCK_SIZE] = {};

    static constexpr size_t HASH_ARR_LEN = HASH_SIZE / sizeof(uint32_t);
    // hash, stored as integers
    uint32_t m_hash[HASH_ARR_LEN] = {};

public:
    // same as reset()
    SHA256();

    // compute SHA256 of a memory block
    std::string operator()(const void *data, size_t num_bytes);

    // add arbitrary number of bytes
    void add(const void *data, size_t num_bytes);

    // return latest hash as 64 hex characters
    std::string getHash();
    // return latest hash as bytes
    void getHash(uint8_t buffer[HASH_SIZE]);

    // restart
    void reset();

private:
    // process 64 bytes
    void processBlock(const void *data);
    // process everything left in the internal buffer
    void processBuffer();
};

#endif  // SHA256_HPP
