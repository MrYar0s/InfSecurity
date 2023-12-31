#include <cstdint>
#include <bit>

#include <sha256.hpp>

static_assert(std::endian::native == std::endian::little);

/// same as reset()
SHA256::SHA256()
{
    reset();
}

/// restart
void SHA256::reset()
{
    m_num_bytes = 0;
    m_buffer_size = 0;

    // according to RFC 1321
    // "These words were obtained by taking the first thirty-two bits of the
    //  fractional parts of the square roots of the first eight prime numbers"
    m_hash[0] = 0x6a09e667;
    m_hash[1] = 0xbb67ae85;
    m_hash[2] = 0x3c6ef372;
    m_hash[3] = 0xa54ff53a;
    m_hash[4] = 0x510e527f;
    m_hash[5] = 0x9b05688c;
    m_hash[6] = 0x1f83d9ab;
    m_hash[7] = 0x5be0cd19;
}

namespace {

uint32_t rotate(uint32_t a, uint32_t c)
{
    return (a >> c) | (a << (32 - c));
}

uint32_t swap(uint32_t x)
{
    return (x >> 24) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) | (x << 24);
}

// mix functions for processBlock()
uint32_t f1(uint32_t e, uint32_t f, uint32_t g)
{
    uint32_t term1 = rotate(e, 6) ^ rotate(e, 11) ^ rotate(e, 25);
    uint32_t term2 = (e & f) ^ (~e & g);  //(g ^ (e & (f ^ g)))
    return term1 + term2;
}

uint32_t f2(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t term1 = rotate(a, 2) ^ rotate(a, 13) ^ rotate(a, 22);
    uint32_t term2 = ((a | b) & c) | (a & b);  //(a & (b ^ c)) ^ (b & c);
    return term1 + term2;
}

}  // namespace

/// process 64 bytes
void SHA256::processBlock(const void *data)
{
    // get last hash
    uint32_t a = m_hash[0];
    uint32_t b = m_hash[1];
    uint32_t c = m_hash[2];
    uint32_t d = m_hash[3];
    uint32_t e = m_hash[4];
    uint32_t f = m_hash[5];
    uint32_t g = m_hash[6];
    uint32_t h = m_hash[7];

    // data represented as 16x 32-bit words
    const uint32_t *input = reinterpret_cast<const uint32_t *>(data);
    // convert to big endian
    uint32_t words[64];
    size_t i;
    for (i = 0; i < 16; i++)
        words[i] = swap(input[i]);

    uint32_t x, y;  // temporaries

    // first round
    x = h + f1(e, f, g) + 0x428a2f98 + words[0];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0x71374491 + words[1];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0xb5c0fbcf + words[2];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0xe9b5dba5 + words[3];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0x3956c25b + words[4];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0x59f111f1 + words[5];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0x923f82a4 + words[6];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0xab1c5ed5 + words[7];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // secound round
    x = h + f1(e, f, g) + 0xd807aa98 + words[8];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0x12835b01 + words[9];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0x243185be + words[10];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0x550c7dc3 + words[11];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0x72be5d74 + words[12];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0x80deb1fe + words[13];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0x9bdc06a7 + words[14];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0xc19bf174 + words[15];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // extend to 24 words
    for (; i < 24; i++)
        words[i] = words[i - 16] + (rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^ (words[i - 15] >> 3)) +
                   words[i - 7] + (rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^ (words[i - 2] >> 10));

    // third round
    x = h + f1(e, f, g) + 0xe49b69c1 + words[16];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0xefbe4786 + words[17];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0x0fc19dc6 + words[18];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0x240ca1cc + words[19];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0x2de92c6f + words[20];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0x4a7484aa + words[21];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0x5cb0a9dc + words[22];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0x76f988da + words[23];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // extend to 32 words
    for (; i < 32; i++)
        words[i] = words[i - 16] + (rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^ (words[i - 15] >> 3)) +
                   words[i - 7] + (rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^ (words[i - 2] >> 10));

    // fourth round
    x = h + f1(e, f, g) + 0x983e5152 + words[24];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0xa831c66d + words[25];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0xb00327c8 + words[26];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0xbf597fc7 + words[27];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0xc6e00bf3 + words[28];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0xd5a79147 + words[29];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0x06ca6351 + words[30];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0x14292967 + words[31];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // extend to 40 words
    for (; i < 40; i++)
        words[i] = words[i - 16] + (rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^ (words[i - 15] >> 3)) +
                   words[i - 7] + (rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^ (words[i - 2] >> 10));

    // fifth round
    x = h + f1(e, f, g) + 0x27b70a85 + words[32];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0x2e1b2138 + words[33];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0x4d2c6dfc + words[34];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0x53380d13 + words[35];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0x650a7354 + words[36];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0x766a0abb + words[37];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0x81c2c92e + words[38];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0x92722c85 + words[39];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // extend to 48 words
    for (; i < 48; i++)
        words[i] = words[i - 16] + (rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^ (words[i - 15] >> 3)) +
                   words[i - 7] + (rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^ (words[i - 2] >> 10));

    // sixth round
    x = h + f1(e, f, g) + 0xa2bfe8a1 + words[40];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0xa81a664b + words[41];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0xc24b8b70 + words[42];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0xc76c51a3 + words[43];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0xd192e819 + words[44];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0xd6990624 + words[45];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0xf40e3585 + words[46];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0x106aa070 + words[47];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // extend to 56 words
    for (; i < 56; i++)
        words[i] = words[i - 16] + (rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^ (words[i - 15] >> 3)) +
                   words[i - 7] + (rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^ (words[i - 2] >> 10));

    // seventh round
    x = h + f1(e, f, g) + 0x19a4c116 + words[48];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0x1e376c08 + words[49];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0x2748774c + words[50];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0x34b0bcb5 + words[51];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0x391c0cb3 + words[52];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0x4ed8aa4a + words[53];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0x5b9cca4f + words[54];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0x682e6ff3 + words[55];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // extend to 64 words
    for (; i < 64; i++)
        words[i] = words[i - 16] + (rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^ (words[i - 15] >> 3)) +
                   words[i - 7] + (rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^ (words[i - 2] >> 10));

    // eigth round
    x = h + f1(e, f, g) + 0x748f82ee + words[56];
    y = f2(a, b, c);
    d += x;
    h = x + y;
    x = g + f1(d, e, f) + 0x78a5636f + words[57];
    y = f2(h, a, b);
    c += x;
    g = x + y;
    x = f + f1(c, d, e) + 0x84c87814 + words[58];
    y = f2(g, h, a);
    b += x;
    f = x + y;
    x = e + f1(b, c, d) + 0x8cc70208 + words[59];
    y = f2(f, g, h);
    a += x;
    e = x + y;
    x = d + f1(a, b, c) + 0x90befffa + words[60];
    y = f2(e, f, g);
    h += x;
    d = x + y;
    x = c + f1(h, a, b) + 0xa4506ceb + words[61];
    y = f2(d, e, f);
    g += x;
    c = x + y;
    x = b + f1(g, h, a) + 0xbef9a3f7 + words[62];
    y = f2(c, d, e);
    f += x;
    b = x + y;
    x = a + f1(f, g, h) + 0xc67178f2 + words[63];
    y = f2(b, c, d);
    e += x;
    a = x + y;

    // update hash
    m_hash[0] += a;
    m_hash[1] += b;
    m_hash[2] += c;
    m_hash[3] += d;
    m_hash[4] += e;
    m_hash[5] += f;
    m_hash[6] += g;
    m_hash[7] += h;
}

/// add arbitrary number of bytes
void SHA256::add(const void *data, size_t num_bytes)
{
    const uint8_t *current = (const uint8_t *)data;

    if (m_buffer_size > 0) {
        while (num_bytes > 0 && m_buffer_size < BLOCK_SIZE) {
            m_buffer[m_buffer_size++] = *current++;
            num_bytes--;
        }
    }

    // full buffer
    if (m_buffer_size == BLOCK_SIZE) {
        processBlock(m_buffer);
        m_num_bytes += BLOCK_SIZE;
        m_buffer_size = 0;
    }

    // no more data ?
    if (num_bytes == 0)
        return;

    // process full blocks
    while (num_bytes >= BLOCK_SIZE) {
        processBlock(current);
        current += BLOCK_SIZE;
        m_num_bytes += BLOCK_SIZE;
        num_bytes -= BLOCK_SIZE;
    }

    // keep remaining bytes in buffer
    while (num_bytes > 0) {
        m_buffer[m_buffer_size++] = *current++;
        num_bytes--;
    }
}

/// process final block, less than 64 bytes
void SHA256::processBuffer()
{
    // the input bytes are considered as bits strings, where the first bit is the
    // most significant bit of the byte

    // - append "1" bit to message
    // - append "0" bits until message length in bit mod 512 is 448
    // - append length as 64 bit integer

    // number of bits
    size_t padded_length = m_buffer_size * 8;

    // plus one bit set to 1 (always appended)
    padded_length++;

    // number of bits must be (num_bits % 512) = 448
    size_t lower11bits = padded_length & 511;
    if (lower11bits <= 448)
        padded_length += 448 - lower11bits;
    else
        padded_length += 512 + 448 - lower11bits;
    // convert from bits to bytes
    padded_length /= 8;

    // only needed if additional data flows over into a second block
    uint8_t extra[BLOCK_SIZE];

    // append a "1" bit, 128 => binary 10000000
    if (m_buffer_size < BLOCK_SIZE)
        m_buffer[m_buffer_size] = 128;
    else
        extra[0] = 128;

    size_t i;
    for (i = m_buffer_size + 1; i < BLOCK_SIZE; i++)
        m_buffer[i] = 0;
    for (; i < padded_length; i++)
        extra[i - BLOCK_SIZE] = 0;

    // add message length in bits as 64 bit number
    uint64_t msg_bits = 8 * (m_num_bytes + m_buffer_size);
    // find right position
    uint8_t *add_len = nullptr;
    if (padded_length < BLOCK_SIZE)
        add_len = m_buffer + padded_length;
    else
        add_len = extra + padded_length - BLOCK_SIZE;

    // must be big endian
    *add_len++ = static_cast<uint8_t>((msg_bits >> 48) & 0xFF);
    *add_len++ = static_cast<uint8_t>((msg_bits >> 56) & 0xFF);
    *add_len++ = static_cast<uint8_t>((msg_bits >> 40) & 0xFF);
    *add_len++ = static_cast<uint8_t>((msg_bits >> 32) & 0xFF);
    *add_len++ = static_cast<uint8_t>((msg_bits >> 24) & 0xFF);
    *add_len++ = static_cast<uint8_t>((msg_bits >> 16) & 0xFF);
    *add_len++ = static_cast<uint8_t>((msg_bits >> 8) & 0xFF);
    *add_len = static_cast<uint8_t>(msg_bits & 0xFF);

    // process blocks
    processBlock(m_buffer);
    // flowed over into a second block ?
    if (padded_length > BLOCK_SIZE)
        processBlock(extra);
}

/// return latest hash as 64 hex characters
std::string SHA256::getHash()
{
    // compute hash (as raw bytes)
    uint8_t raw_hash[HASH_SIZE];
    getHash(raw_hash);

    // convert to hex string
    std::string result;
    result.reserve(2 * HASH_SIZE);
    for (size_t i = 0; i < HASH_SIZE; i++) {
        static constexpr std::string_view dec2hex = "0123456789abcdef";
        result += dec2hex[(raw_hash[i] >> 4) & 0xf];
        result += dec2hex[raw_hash[i] & 0xf];
    }

    return result;
}

/// return latest hash as bytes
void SHA256::getHash(uint8_t buffer[SHA256::HASH_SIZE])
{
    // save old hash if buffer is partially filled
    uint32_t old_hash[HASH_ARR_LEN];
    for (size_t i = 0; i < HASH_ARR_LEN; i++)
        old_hash[i] = m_hash[i];

    // process remaining bytes
    processBuffer();

    uint8_t *current = buffer;
    for (size_t i = 0; i < HASH_ARR_LEN; i++) {
        *current++ = (m_hash[i] >> 24) & 0xFF;
        *current++ = (m_hash[i] >> 16) & 0xFF;
        *current++ = (m_hash[i] >> 8) & 0xFF;
        *current++ = m_hash[i] & 0xFF;

        // restore old hash
        m_hash[i] = old_hash[i];
    }
}

/// compute SHA256 of a memory block
std::string SHA256::operator()(const void *data, size_t num_bytes)
{
    reset();
    add(data, num_bytes);
    return getHash();
}
