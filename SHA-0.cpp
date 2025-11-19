#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdint>

class SHA0 {
public:
    SHA0() { reset(); }

    void reset() {
        h0 = 0x67452301;
        h1 = 0xEFCDAB89;
        h2 = 0x98BADCFE;
        h3 = 0x10325476;
        h4 = 0xC3D2E1F0;
        message_len = 0;
        buffer_len = 0;
    }

    void update(const uint8_t* data, size_t len) {
        message_len += len * 8;

        while (len > 0) {
            size_t space = 64 - buffer_len;
            size_t copy_len = (len < space ? len : space);

            memcpy(buffer + buffer_len, data, copy_len);
            buffer_len += copy_len;
            data += copy_len;
            len -= copy_len;

            if (buffer_len == 64) {
                process_block(buffer);
                buffer_len = 0;
            }
        }
    }

    void update(const std::string& s) {
        update(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    }

    std::string final() {
        uint8_t pad[64] = {0x80};
        uint8_t length_bytes[8];

        for (int i = 0; i < 8; i++)
            length_bytes[7 - i] = (message_len >> (i * 8)) & 0xFF;

        size_t pad_len = (buffer_len < 56 ? 56 - buffer_len : 120 - buffer_len);
        update(pad, pad_len);
        update(length_bytes, 8);

        uint32_t digest[5] = {h0, h1, h2, h3, h4};

        std::ostringstream out;
        out << std::hex << std::setfill('0');
        for (uint32_t v : digest)
            out << std::setw(8) << v;

        return out.str();
    }

private:
    uint32_t h0, h1, h2, h3, h4;
    uint64_t message_len;
    uint8_t buffer[64];
    size_t buffer_len;

    static uint32_t rol(uint32_t x, uint32_t n) {
        return (x << n) | (x >> (32 - n));
    }

    static uint32_t f(uint32_t t, uint32_t b, uint32_t c, uint32_t d) {
        if (t < 20) return (b & c) | (~b & d);
        if (t < 40) return b ^ c ^ d;
        if (t < 60) return (b & c) | (b & d) | (c & d);
        return b ^ c ^ d;
    }

    void process_block(const uint8_t block[64]) {
        uint32_t w[80];

        for (int i = 0; i < 16; i++) {
            w[i] = (block[i*4] << 24) |
                   (block[i*4 + 1] << 16) |
                   (block[i*4 + 2] << 8) |
                   (block[i*4 + 3]);
        }

        for (int t = 16; t < 80; t++) {
            // SHA-0 uses NO left-rotate here (unlike SHA-1)
            w[t] = w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16];
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        for (int t = 0; t < 80; t++) {
            uint32_t temp = rol(a, 5) + f(t, b, c, d) + e + w[t] +
                (t < 20 ? 0x5A827999 :
                 t < 40 ? 0x6ED9EBA1 :
                 t < 60 ? 0x8F1BBCDC :
                          0xCA62C1D6);

            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }
};

std::string sha0_file(const std::string& path) {
    SHA0 sha;
    std::ifstream f(path, std::ios::binary);

    if (!f.good()) return "";

    uint8_t buf[1024];
    while (!f.eof()) {
        f.read((char*)buf, sizeof(buf));
        sha.update(buf, f.gcount());
    }

    return sha.final();
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // hash stdin
        SHA0 sha;
        uint8_t buf[1024];
        while (!std::cin.eof()) {
            std::cin.read((char*)buf, sizeof(buf));
            sha.update(buf, std::cin.gcount());
        }
        std::cout << sha.final() << std::endl;
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-m") {
            if (i+1 >= argc) {
                std::cerr << "-m but no string" << std::endl;
                return 1;
            }
            SHA0 sha;
            sha.update(argv[++i]);
            std::cout << sha.final() << std::endl;
        } else {
            std::string hash = sha0_file(arg);
            if (hash == "") std::cout << "Cannot open: " << arg << std::endl;
            else std::cout << "SHA0(" << arg << ") = " << hash << std::endl;
        }
    }
    return 0;
}
