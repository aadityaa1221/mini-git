#ifndef SHA1_HPP
#define SHA1_HPP

#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>

class SHA1 {
public:
    SHA1() { reset(); }

    void update(const std::string &s) {
        std::istringstream is(s);
        update(is);
    }

    void update(std::istream &is) {
        std::string restOfBuffer;
        read(is, restOfBuffer);
        update(restOfBuffer.c_str(), restOfBuffer.size());
    }

    std::string final() {
        uint8_t digest[20];
        finalize(digest);
        std::ostringstream buf;
        for (int i = 0; i < 20; ++i) {
            buf << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
        }
        return buf.str();
    }

private:
    uint32_t digest[5];
    std::string buffer;
    uint64_t transforms;

    void reset() {
        digest[0] = 0x67452301;
        digest[1] = 0xefcdab89;
        digest[2] = 0x98badcfe;
        digest[3] = 0x10325476;
        digest[4] = 0xc3d2e1f0;
        transforms = 0;
        buffer = "";
    }

    void read(std::istream &is, std::string &s) {
        char sbuf[256];
        while (is.good()) {
            is.read(sbuf, 256);
            s += std::string(sbuf, is.gcount());
        }
    }

    void transform(uint32_t block[80]) {
        uint32_t a = digest[0];
        uint32_t b = digest[1];
        uint32_t c = digest[2];
        uint32_t d = digest[3];
        uint32_t e = digest[4];

        for (uint8_t i = 0; i < 80; ++i) {
            uint32_t f = 0, k = 0;
            if (i < 20) {
                f = (b & c) | (~b & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + block[i];
            e = d;
            d = c;
            c = (b << 30) | (b >> 2);
            b = a;
            a = temp;
        }

        digest[0] += a;
        digest[1] += b;
        digest[2] += c;
        digest[3] += d;
        digest[4] += e;
        transforms++;
    }

    void update(const char* data, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            buffer += data[i];
            if (buffer.size() == 64) {
                uint32_t block[80];
                for (size_t j = 0; j < 16; ++j) {
                    block[j] = (buffer[j * 4 + 3] & 0xff)
                             | (buffer[j * 4 + 2] & 0xff) << 8
                             | (buffer[j * 4 + 1] & 0xff) << 16
                             | (buffer[j * 4 + 0] & 0xff) << 24;
                }
                for (size_t j = 16; j < 80; ++j) {
                    uint32_t val = block[j - 3] ^ block[j - 8] ^ block[j - 14] ^ block[j - 16];
                    block[j] = (val << 1) | (val >> 31);
                }
                transform(block);
                buffer.clear();
            }
        }
    }

    void finalize(uint8_t digest_out[20]) {
        uint64_t total_bits = (transforms * 64 + buffer.size()) * 8;
        buffer += (char)0x80;
        size_t orig_size = buffer.size();
        while (buffer.size() < 64) { buffer += (char)0x00; }

        uint32_t block[80];
        for (size_t j = 0; j < 16; ++j) {
            block[j] = (buffer[j * 4 + 3] & 0xff)
                     | (buffer[j * 4 + 2] & 0xff) << 8
                     | (buffer[j * 4 + 1] & 0xff) << 16
                     | (buffer[j * 4 + 0] & 0xff) << 24;
        }

        if (orig_size > 56) {
            for (size_t j = 16; j < 80; ++j) {
                uint32_t val = block[j - 3] ^ block[j - 8] ^ block[j - 14] ^ block[j - 16];
                block[j] = (val << 1) | (val >> 31);
            }
            transform(block);
            for (size_t j = 0; j < 16; ++j) { block[j] = 0; }
        }

        block[14] = total_bits >> 32;
        block[15] = total_bits & 0xffffffff;

        for (size_t j = 16; j < 80; ++j) {
            uint32_t val = block[j - 3] ^ block[j - 8] ^ block[j - 14] ^ block[j - 16];
            block[j] = (val << 1) | (val >> 31);
        }
        transform(block);

        for (size_t i = 0; i < 20; ++i) {
            digest_out[i] = (this->digest[i >> 2] >> (((3 - i) & 3) * 8)) & 0xff;
        }
    }
};

#endif