#ifndef KECCAK_HPP
#define KECCAK_HPP

#include <cstdint>
#include <string>

struct Keccak final {
    /// algorithm variants
    enum Bits { Keccak224 = 224, Keccak256 = 256, Keccak384 = 384, Keccak512 = 512 };

private:
    /// 1600 bits, stored as 25x64 bit, BlockSize is no more than 1152 bits
    /// (Keccak224)
    static constexpr size_t STATE_SIZE = 1600 / (8 * 8);
    static constexpr size_t MAX_BLOCK_SIZE = 200 - 2 * (224 / 8);

    /// hash
    uint64_t m_hash[STATE_SIZE] {};
    /// size of processed data in bytes
    uint64_t m_num_bytes = 0;
    /// block size (less or equal to MAX_BLOCK_SIZE)
    size_t m_block_size = 0;
    /// valid bytes in m_buffer
    size_t m_buffer_size = 0;
    /// bytes not processed yet
    uint8_t m_buffer[MAX_BLOCK_SIZE] {};
    /// variant
    Bits m_bits {};

public:
    explicit Keccak(Bits bits = Keccak256);

    /// compute hash of a memory block
    std::string operator()(const void *data, size_t num_bytes);

    /// add arbitrary number of bytes
    void add(const void *data, size_t num_bytes);

    /// return latest hash as hex characters
    std::string getHash();

    /// restart
    void reset();

private:
    /// process a full block
    void processBlock(const void *data);
    /// process everything left in the internal buffer
    void processBuffer();
};

#endif // KECCAK_HPP
