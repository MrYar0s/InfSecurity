#include <keccak.hpp>

#include <bit>

static_assert(std::endian::native == std::endian::little);

/// same as reset()
Keccak::Keccak(Bits bits) : m_block_size(static_cast<size_t>(200 - 2 * (bits / 8))), m_bits(bits)
{
    reset();
}

/// restart
void Keccak::reset()
{
    for (auto &hash : m_hash) {
        hash = 0;
    }

    m_num_bytes = 0;
    m_buffer_size = 0;
}

/// constants and local helper functions
namespace {

const uint32_t KECCAK_ROUNDS = 24;
const uint64_t XOR_MASKS[KECCAK_ROUNDS] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL, 0x000000000000808bULL,
    0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL, 0x0000000000000088ULL,
    0x0000000080008009ULL, 0x000000008000000aULL, 0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

/// rotate left and wrap around to the right
inline uint64_t rotateLeft(uint64_t x, uint8_t numBits)
{
    return (x << numBits) | (x >> (64 - numBits));
}

/// return x % 5 for 0 <= x <= 9
uint32_t mod5(uint32_t x)
{
    if (x < 5)
        return x;

    return x - 5;
}

}  // namespace

/// process a full block
void Keccak::processBlock(const void *data)
{
    const uint64_t *data64 = (const uint64_t *)data;
    // mix data into state
    for (uint32_t i = 0; i < m_block_size / 8; i++)
        m_hash[i] ^= data64[i];

    // re-compute state
    for (uint32_t round = 0; round < KECCAK_ROUNDS; round++) {
        // Theta
        uint64_t coefficients[5] {};
        for (uint32_t i = 0; i < 5; i++)
            coefficients[i] = m_hash[i] ^ m_hash[i + 5] ^ m_hash[i + 10] ^ m_hash[i + 15] ^ m_hash[i + 20];

        for (uint32_t i = 0; i < 5; i++) {
            uint64_t one = coefficients[mod5(i + 4)] ^ rotateLeft(coefficients[mod5(i + 1)], 1);
            m_hash[i] ^= one;
            m_hash[i + 5] ^= one;
            m_hash[i + 10] ^= one;
            m_hash[i + 15] ^= one;
            m_hash[i + 20] ^= one;
        }

        // temporary
        uint64_t one;

        // Rho Pi
        uint64_t last = m_hash[1];
        one = m_hash[10];
        m_hash[10] = rotateLeft(last, 1);
        last = one;
        one = m_hash[7];
        m_hash[7] = rotateLeft(last, 3);
        last = one;
        one = m_hash[11];
        m_hash[11] = rotateLeft(last, 6);
        last = one;
        one = m_hash[17];
        m_hash[17] = rotateLeft(last, 10);
        last = one;
        one = m_hash[18];
        m_hash[18] = rotateLeft(last, 15);
        last = one;
        one = m_hash[3];
        m_hash[3] = rotateLeft(last, 21);
        last = one;
        one = m_hash[5];
        m_hash[5] = rotateLeft(last, 28);
        last = one;
        one = m_hash[16];
        m_hash[16] = rotateLeft(last, 36);
        last = one;
        one = m_hash[8];
        m_hash[8] = rotateLeft(last, 45);
        last = one;
        one = m_hash[21];
        m_hash[21] = rotateLeft(last, 55);
        last = one;
        one = m_hash[24];
        m_hash[24] = rotateLeft(last, 2);
        last = one;
        one = m_hash[4];
        m_hash[4] = rotateLeft(last, 14);
        last = one;
        one = m_hash[15];
        m_hash[15] = rotateLeft(last, 27);
        last = one;
        one = m_hash[23];
        m_hash[23] = rotateLeft(last, 41);
        last = one;
        one = m_hash[19];
        m_hash[19] = rotateLeft(last, 56);
        last = one;
        one = m_hash[13];
        m_hash[13] = rotateLeft(last, 8);
        last = one;
        one = m_hash[12];
        m_hash[12] = rotateLeft(last, 25);
        last = one;
        one = m_hash[2];
        m_hash[2] = rotateLeft(last, 43);
        last = one;
        one = m_hash[20];
        m_hash[20] = rotateLeft(last, 62);
        last = one;
        one = m_hash[14];
        m_hash[14] = rotateLeft(last, 18);
        last = one;
        one = m_hash[22];
        m_hash[22] = rotateLeft(last, 39);
        last = one;
        one = m_hash[9];
        m_hash[9] = rotateLeft(last, 61);
        last = one;
        one = m_hash[6];
        m_hash[6] = rotateLeft(last, 20);
        last = one;
        m_hash[1] = rotateLeft(last, 44);

        // Chi
        for (size_t j = 0; j < STATE_SIZE; j += 5) {
            // temporaries
            uint64_t one_t = m_hash[j];
            uint64_t two_t = m_hash[j + 1];

            m_hash[j] ^= m_hash[j + 2] & ~two_t;
            m_hash[j + 1] ^= m_hash[j + 3] & ~m_hash[j + 2];
            m_hash[j + 2] ^= m_hash[j + 4] & ~m_hash[j + 3];
            m_hash[j + 3] ^= one_t & ~m_hash[j + 4];
            m_hash[j + 4] ^= two_t & ~one_t;
        }

        // Iota
        m_hash[0] ^= XOR_MASKS[round];
    }
}

/// add arbitrary number of bytes
void Keccak::add(const void *data, size_t num_bytes)
{
    const uint8_t *current = (const uint8_t *)data;

    if (m_buffer_size > 0) {
        while (num_bytes > 0 && m_buffer_size < m_block_size) {
            m_buffer[m_buffer_size++] = *current++;
            num_bytes--;
        }
    }

    // full buffer
    if (m_buffer_size == m_block_size) {
        processBlock((void *)m_buffer);
        m_num_bytes += m_block_size;
        m_buffer_size = 0;
    }

    // no more data ?
    if (num_bytes == 0)
        return;

    // process full blocks
    while (num_bytes >= m_block_size) {
        processBlock(current);
        current += m_block_size;
        m_num_bytes += m_block_size;
        num_bytes -= m_block_size;
    }

    // keep remaining bytes in buffer
    while (num_bytes > 0) {
        m_buffer[m_buffer_size++] = *current++;
        num_bytes--;
    }
}

/// process everything left in the internal buffer
void Keccak::processBuffer()
{
    size_t blockSize = static_cast<size_t>(200 - 2 * (m_bits / 8));

    // add padding
    size_t offset = m_buffer_size;
    // add a "1" byte
    m_buffer[offset++] = 1;
    // fill with zeros
    while (offset < blockSize)
        m_buffer[offset++] = 0;

    // and add a single set bit
    m_buffer[blockSize - 1] |= 0x80;

    processBlock(m_buffer);
}

/// return latest hash as 16 hex characters
std::string Keccak::getHash()
{
    // save hash state
    uint64_t old_hash[STATE_SIZE] {};
    for (uint32_t i = 0; i < STATE_SIZE; i++)
        old_hash[i] = m_hash[i];

    // process remaining bytes
    processBuffer();

    // convert hash to string
    const char *dec2hex = "0123456789abcdef";

    // number of significant elements in hash (uint64_t)
    uint32_t hash_length = static_cast<uint32_t>(m_bits / 64);

    std::string result {};
    for (uint32_t i = 0; i < hash_length; i++)
        for (uint32_t j = 0; j < 8; j++)  // 64 bits => 8 bytes
        {
            // convert a byte to hex
            uint8_t oneByte = (uint8_t)(m_hash[i] >> (8 * j));
            result += dec2hex[oneByte >> 4];
            result += dec2hex[oneByte & 15];
        }

    // Keccak224's last entry in m_hash provides only 32 bits instead of 64 bits
    uint32_t remainder = static_cast<unsigned>(m_bits) - hash_length * 64;
    uint32_t processed = 0;
    while (processed < remainder) {
        // convert a byte to hex
        uint8_t oneByte = (uint8_t)(m_hash[hash_length] >> processed);
        result += dec2hex[oneByte >> 4];
        result += dec2hex[oneByte & 15];

        processed += 8;
    }

    // restore state
    for (uint32_t i = 0; i < STATE_SIZE; i++)
        m_hash[i] = old_hash[i];

    return result;
}

/// compute Keccak hash of a memory block
std::string Keccak::operator()(const void *data, size_t num_bytes)
{
    reset();
    add(data, num_bytes);
    return getHash();
}
