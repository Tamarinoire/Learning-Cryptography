#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

// Single-file HAVAL-256/5 implementation

namespace haval {

namespace detail {

using word_t = std::uint32_t;
constexpr unsigned int version = 1; // HAVAL version 1

// padding 0x01 di depan, sisanya 0
constexpr std::uint8_t padding[128] = {
    0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

inline word_t f_1(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    return ((x1 & (x0 ^ x4)) ^ (x2 & x5) ^ (x3 & x6) ^ x0);
}

inline word_t f_2(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    return ((x2 & ((x1 & ~x3) ^ (x4 & x5) ^ x6 ^ x0)) ^ (x4 & (x1 ^ x5)) ^ (x3 & x5) ^ x0);
}

inline word_t f_3(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    return ((x3 & ((x1 & x2) ^ x6 ^ x0)) ^ (x1 & x4) ^ (x2 & x5) ^ x0);
}

inline word_t f_4(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    return ((x4 & ((x5 & ~x2) ^ (x3 & ~x6) ^ x1 ^ x6 ^ x0)) ^
            (x3 & ((x1 & x2) ^ x5 ^ x6)) ^ (x2 & x6) ^ x0);
}

inline word_t f_5(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    return ((x0 & ((x1 & x2 & x3) ^ ~x5)) ^ (x1 & x4) ^ (x2 & x5) ^ (x3 & x6));
}

// phi permutasi untuk 5-pass

template<unsigned int pass_cnt>
word_t Fphi_1(word_t, word_t, word_t, word_t, word_t, word_t, word_t) = delete;

template<>
inline word_t Fphi_1<5>(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    // phi_{5,1}: 3 4 1 0 5 2 6
    return f_1(x3, x4, x1, x0, x5, x2, x6);
}

template<unsigned int pass_cnt>
word_t Fphi_2(word_t, word_t, word_t, word_t, word_t, word_t, word_t) = delete;

template<>
inline word_t Fphi_2<5>(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    // phi_{5,2}: 6 2 1 0 3 4 5
    return f_2(x6, x2, x1, x0, x3, x4, x5);
}

template<unsigned int pass_cnt>
word_t Fphi_3(word_t, word_t, word_t, word_t, word_t, word_t, word_t) = delete;

template<>
inline word_t Fphi_3<5>(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    // phi_{5,3}: 2 6 0 4 3 1 5
    return f_3(x2, x6, x0, x4, x3, x1, x5);
}

template<unsigned int pass_cnt>
word_t Fphi_4(word_t, word_t, word_t, word_t, word_t, word_t, word_t) = delete;

template<>
inline word_t Fphi_4<5>(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    // phi_{5,4}: 1 5 3 2 0 4 6
    return f_4(x1, x5, x3, x2, x0, x4, x6);
}

template<unsigned int pass_cnt>
word_t Fphi_5(word_t, word_t, word_t, word_t, word_t, word_t, word_t) = delete;

template<>
inline word_t Fphi_5<5>(word_t x6, word_t x5, word_t x4, word_t x3, word_t x2, word_t x1, word_t x0)
{
    // phi_{5,5}: 2 5 0 6 4 3 1
    return f_5(x2, x5, x0, x6, x4, x3, x1);
}

inline word_t rotate_right(word_t x, word_t n)
{
    return ((x >> n) | (x << (32 - n)));
}

template<unsigned int pass_cnt>
inline void FF_1(word_t& x7, word_t x6, word_t x5, word_t x4,
                 word_t x3, word_t x2, word_t x1, word_t x0, word_t w)
{
    x7 = rotate_right(Fphi_1<pass_cnt>(x6, x5, x4, x3, x2, x1, x0), 7) +
         rotate_right(x7, 11) + w;
}

template<unsigned int pass_cnt>
inline void FF_2(word_t& x7, word_t x6, word_t x5, word_t x4,
                 word_t x3, word_t x2, word_t x1, word_t x0, word_t w, word_t c)
{
    x7 = rotate_right(Fphi_2<pass_cnt>(x6, x5, x4, x3, x2, x1, x0), 7) +
         rotate_right(x7, 11) + w + c;
}

template<unsigned int pass_cnt>
inline void FF_3(word_t& x7, word_t x6, word_t x5, word_t x4,
                 word_t x3, word_t x2, word_t x1, word_t x0, word_t w, word_t c)
{
    x7 = rotate_right(Fphi_3<pass_cnt>(x6, x5, x4, x3, x2, x1, x0), 7) +
         rotate_right(x7, 11) + w + c;
}

template<unsigned int pass_cnt>
inline void FF_4(word_t& x7, word_t x6, word_t x5, word_t x4,
                 word_t x3, word_t x2, word_t x1, word_t x0, word_t w, word_t c)
{
    x7 = rotate_right(Fphi_4<pass_cnt>(x6, x5, x4, x3, x2, x1, x0), 7) +
         rotate_right(x7, 11) + w + c;
}

template<unsigned int pass_cnt>
inline void FF_5(word_t& x7, word_t x6, word_t x5, word_t x4,
                 word_t x3, word_t x2, word_t x1, word_t x0, word_t w, word_t c)
{
    x7 = rotate_right(Fphi_5<pass_cnt>(x6, x5, x4, x3, x2, x1, x0), 7) +
         rotate_right(x7, 11) + w + c;
}

// string -> word32 (little endian)
inline void ch2uint(const std::uint8_t* string, word_t* word, std::size_t slen)
{
    const std::uint8_t* sp = string;
    word_t* wp = word;
    while (sp < string + slen) {
        *wp = word_t{sp[0]} |
              (word_t{sp[1]} << 8) |
              (word_t{sp[2]} << 16) |
              (word_t{sp[3]} << 24);
        wp++;
        sp += 4;
    }
}

// word32 -> string (little endian)
inline void uint2ch(const word_t* word, std::uint8_t* string, std::size_t wlen)
{
    const word_t* wp = word;
    std::uint8_t* sp = string;
    while (wp < word + wlen) {
        sp[0] = static_cast<std::uint8_t>(*wp & 0xFF);
        sp[1] = static_cast<std::uint8_t>((*wp >> 8) & 0xFF);
        sp[2] = static_cast<std::uint8_t>((*wp >> 16) & 0xFF);
        sp[3] = static_cast<std::uint8_t>((*wp >> 24) & 0xFF);
        sp += 4;
        wp++;
    }
}

struct haval_context {
    word_t fingerprint[8];
    word_t count[2];
    word_t block[32];
    std::uint8_t remainder[128];
};

// forward declaration
template<unsigned int pass_cnt, unsigned int curr_pass>
void hash_block(word_t& t0, word_t& t1, word_t& t2, word_t& t3,
                word_t& t4, word_t& t5, word_t& t6, word_t& t7,
                const word_t* w);

// PASS 1
template<>
inline void hash_block<5, 1>(word_t& t0, word_t& t1, word_t& t2, word_t& t3,
                             word_t& t4, word_t& t5, word_t& t6, word_t& t7,
                             const word_t* w)
{
    FF_1<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[0]);
    FF_1<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[1]);
    FF_1<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[2]);
    FF_1<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[3]);
    FF_1<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[4]);
    FF_1<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[5]);
    FF_1<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[6]);
    FF_1<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[7]);

    FF_1<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[8]);
    FF_1<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[9]);
    FF_1<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[10]);
    FF_1<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[11]);
    FF_1<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[12]);
    FF_1<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[13]);
    FF_1<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[14]);
    FF_1<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[15]);

    FF_1<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[16]);
    FF_1<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[17]);
    FF_1<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[18]);
    FF_1<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[19]);
    FF_1<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[20]);
    FF_1<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[21]);
    FF_1<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[22]);
    FF_1<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[23]);

    FF_1<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[24]);
    FF_1<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[25]);
    FF_1<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[26]);
    FF_1<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[27]);
    FF_1<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[28]);
    FF_1<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[29]);
    FF_1<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[30]);
    FF_1<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[31]);
}

// PASS 2
template<>
inline void hash_block<5, 2>(word_t& t0, word_t& t1, word_t& t2, word_t& t3,
                             word_t& t4, word_t& t5, word_t& t6, word_t& t7,
                             const word_t* w)
{
    hash_block<5, 1>(t0, t1, t2, t3, t4, t5, t6, t7, w);

#define C(x) static_cast<word_t>(x##u)

    FF_2<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[5],  C(0x452821E6));
    FF_2<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[14], C(0x38D01377));
    FF_2<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[26], C(0xBE5466CF));
    FF_2<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[18], C(0x34E90C6C));
    FF_2<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[11], C(0xC0AC29B7));
    FF_2<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[28], C(0xC97C50DD));
    FF_2<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[7],  C(0x3F84D5B5));
    FF_2<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[16], C(0xB5470917));

    FF_2<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[0],  C(0x9216D5D9));
    FF_2<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[23], C(0x8979FB1B));
    FF_2<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[20], C(0xD1310BA6));
    FF_2<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[22], C(0x98DFB5AC));
    FF_2<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[1],  C(0x2FFD72DB));
    FF_2<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[10], C(0xD01ADFB7));
    FF_2<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[4],  C(0xB8E1AFED));
    FF_2<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[8],  C(0x6A267E96));

    FF_2<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[30], C(0xBA7C9045));
    FF_2<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[3],  C(0xF12C7F99));
    FF_2<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[21], C(0x24A19947));
    FF_2<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[9],  C(0xB3916CF7));
    FF_2<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[17], C(0x0801F2E2));
    FF_2<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[24], C(0x858EFC16));
    FF_2<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[29], C(0x636920D8));
    FF_2<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[6],  C(0x71574E69));

    FF_2<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[19], C(0xA458FEA3));
    FF_2<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[12], C(0xF4933D7E));
    FF_2<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[15], C(0x0D95748F));
    FF_2<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[13], C(0x728EB658));
    FF_2<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[2],  C(0x718BCD58));
    FF_2<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[25], C(0x82154AEE));
    FF_2<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[31], C(0x7B54A41D));
    FF_2<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[27], C(0xC25A59B5));
}

// PASS 3
template<>
inline void hash_block<5, 3>(word_t& t0, word_t& t1, word_t& t2, word_t& t3,
                             word_t& t4, word_t& t5, word_t& t6, word_t& t7,
                             const word_t* w)
{
    hash_block<5, 2>(t0, t1, t2, t3, t4, t5, t6, t7, w);

    FF_3<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[19], C(0x9C30D539));
    FF_3<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[9],  C(0x2AF26013));
    FF_3<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[4],  C(0xC5D1B023));
    FF_3<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[20], C(0x286085F0));
    FF_3<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[28], C(0xCA417918));
    FF_3<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[17], C(0xB8DB38EF));
    FF_3<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[8],  C(0x8E79DCB0));
    FF_3<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[22], C(0x603A180E));

    FF_3<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[29], C(0x6C9E0E8B));
    FF_3<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[14], C(0xB01E8A3E));
    FF_3<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[25], C(0xD71577C1));
    FF_3<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[12], C(0xBD314B27));
    FF_3<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[24], C(0x78AF2FDA));
    FF_3<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[30], C(0x55605C60));
    FF_3<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[16], C(0xE65525F3));
    FF_3<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[26], C(0xAA55AB94));

    FF_3<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[31], C(0x57489862));
    FF_3<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[15], C(0x63E81440));
    FF_3<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[7],  C(0x55CA396A));
    FF_3<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[3],  C(0x2AAB10B6));
    FF_3<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[1],  C(0xB4CC5C34));
    FF_3<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[0],  C(0x1141E8CE));
    FF_3<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[18], C(0xA15486AF));
    FF_3<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[27], C(0x7C72E993));

    FF_3<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[13], C(0xB3EE1411));
    FF_3<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[6],  C(0x636FBC2A));
    FF_3<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[21], C(0x2BA9C55D));
    FF_3<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[10], C(0x741831F6));
    FF_3<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[23], C(0xCE5C3E16));
    FF_3<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[11], C(0x9B87931E));
    FF_3<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[5],  C(0xAFD6BA33));
    FF_3<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[2],  C(0x6C24CF5C));
}

// PASS 4
template<>
inline void hash_block<5, 4>(word_t& t0, word_t& t1, word_t& t2, word_t& t3,
                             word_t& t4, word_t& t5, word_t& t6, word_t& t7,
                             const word_t* w)
{
    hash_block<5, 3>(t0, t1, t2, t3, t4, t5, t6, t7, w);

    FF_4<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[24], C(0x7A325381));
    FF_4<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[4],  C(0x28958677));
    FF_4<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[0],  C(0x3B8F4898));
    FF_4<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[14], C(0x6B4BB9AF));
    FF_4<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[2],  C(0xC4BFE81B));
    FF_4<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[7],  C(0x66282193));
    FF_4<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[28], C(0x61D809CC));
    FF_4<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[23], C(0xFB21A991));

    FF_4<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[26], C(0x487CAC60));
    FF_4<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[6],  C(0x5DEC8032));
    FF_4<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[30], C(0xEF845D5D));
    FF_4<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[20], C(0xE98575B1));
    FF_4<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[18], C(0xDC262302));
    FF_4<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[25], C(0xEB651B88));
    FF_4<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[19], C(0x23893E81));
    FF_4<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[3],  C(0xD396ACC5));

    FF_4<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[22], C(0x0F6D6FF3));
    FF_4<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[11], C(0x83F44239));
    FF_4<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[31], C(0x2E0B4482));
    FF_4<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[21], C(0xA4842004));
    FF_4<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[8],  C(0x69C8F04A));
    FF_4<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[27], C(0x9E1F9B5E));
    FF_4<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[12], C(0x21C66842));
    FF_4<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[9],  C(0xF6E96C9A));

    FF_4<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[1],  C(0x670C9C61));
    FF_4<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[29], C(0xABD388F0));
    FF_4<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[5],  C(0x6A51A0D2));
    FF_4<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[15], C(0xD8542F68));
    FF_4<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[17], C(0x960FA728));
    FF_4<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[10], C(0xAB5133A3));
    FF_4<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[16], C(0x6EEF0B6C));
    FF_4<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[13], C(0x137A3BE4));
}

// PASS 5
template<>
inline void hash_block<5, 5>(word_t& t0, word_t& t1, word_t& t2, word_t& t3,
                             word_t& t4, word_t& t5, word_t& t6, word_t& t7,
                             const word_t* w)
{
    hash_block<5, 4>(t0, t1, t2, t3, t4, t5, t6, t7, w);

    FF_5<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[27], C(0xBA3BF050));
    FF_5<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[3],  C(0x7EFB2A98));
    FF_5<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[21], C(0xA1F1651D));
    FF_5<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[26], C(0x39AF0176));
    FF_5<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[17], C(0x66CA593E));
    FF_5<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[11], C(0x82430E88));
    FF_5<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[20], C(0x8CEE8619));
    FF_5<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[29], C(0x456F9FB4));

    FF_5<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[19], C(0x7D84A5C3));
    FF_5<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[0],  C(0x3B8B5EBE));
    FF_5<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[12], C(0xE06F75D8));
    FF_5<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[7],  C(0x85C12073));
    FF_5<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[13], C(0x401A449F));
    FF_5<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[8],  C(0x56C16AA6));
    FF_5<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[31], C(0x4ED3AA62));
    FF_5<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[10], C(0x363F7706));

    FF_5<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[5],  C(0x1BFEDF72));
    FF_5<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[9],  C(0x429B023D));
    FF_5<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[14], C(0x37D0D724));
    FF_5<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[30], C(0xD00A1248));
    FF_5<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[18], C(0xDB0FEAD3));
    FF_5<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[6],  C(0x49F1C09B));
    FF_5<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[28], C(0x075372C9));
    FF_5<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[24], C(0x80991B7B));

    FF_5<5>(t7, t6, t5, t4, t3, t2, t1, t0, w[2],  C(0x25D479D8));
    FF_5<5>(t6, t5, t4, t3, t2, t1, t0, t7, w[23], C(0xF6E8DEF7));
    FF_5<5>(t5, t4, t3, t2, t1, t0, t7, t6, w[16], C(0xE3FE501A));
    FF_5<5>(t4, t3, t2, t1, t0, t7, t6, t5, w[22], C(0xB6794C3B));
    FF_5<5>(t3, t2, t1, t0, t7, t6, t5, t4, w[4],  C(0x976CE0BD));
    FF_5<5>(t2, t1, t0, t7, t6, t5, t4, t3, w[1],  C(0x04C006BA));
    FF_5<5>(t1, t0, t7, t6, t5, t4, t3, t2, w[25], C(0xC1A94FB6));
    FF_5<5>(t0, t7, t6, t5, t4, t3, t2, t1, w[15], C(0x409F60C4));

#undef C
}

// tailor 256-bit (no-op)
inline void tailor_256(haval_context&) {}

} // namespace detail

// Template utama (kita pakai spesifik 5,256)
template<unsigned int pass_cnt, unsigned int fpt_len>
class haval
{
public:
    using size_type = std::size_t;
    static constexpr size_type result_size = fpt_len / 8;

    void start()
    {
        m_context.count[0] = 0;
        m_context.count[1] = 0;
        m_context.fingerprint[0] = 0x243F6A88u;
        m_context.fingerprint[1] = 0x85A308D3u;
        m_context.fingerprint[2] = 0x13198A2Eu;
        m_context.fingerprint[3] = 0x03707344u;
        m_context.fingerprint[4] = 0xA4093822u;
        m_context.fingerprint[5] = 0x299F31D0u;
        m_context.fingerprint[6] = 0x082EFA98u;
        m_context.fingerprint[7] = 0xEC4E6C89u;
    }

    void update(const void* vdata, size_type data_len)
    {
        const auto* data = static_cast<const std::uint8_t*>(vdata);
        size_type rmd_len = (m_context.count[0] >> 3) & 0x7F;
        size_type fill_len = 128 - rmd_len;

        m_context.count[0] += static_cast<detail::word_t>(data_len) << 3;
        if (m_context.count[0] < (static_cast<detail::word_t>(data_len) << 3)) {
            m_context.count[1]++;
        }
        m_context.count[1] += static_cast<detail::word_t>(data_len) >> 29;

        size_type i = 0;

        if (rmd_len + data_len >= 128) {
            std::memcpy(&m_context.remainder[rmd_len], data, fill_len);
            detail::ch2uint(m_context.remainder, m_context.block, 128);
            hash_block();
            for (i = fill_len; i + 127 < data_len; i += 128) {
                std::memcpy(m_context.remainder, data + i, 128);
                detail::ch2uint(m_context.remainder, m_context.block, 128);
                hash_block();
            }
            rmd_len = 0;
        }
        std::memcpy(&m_context.remainder[rmd_len], data + i, data_len - i);
    }

    void end_to(void* out)
    {
        std::uint8_t tail[10];
        tail[0] = static_cast<std::uint8_t>(((fpt_len & 0x3) << 6) |
                                            ((pass_cnt & 0x7) << 3) |
                                            (detail::version & 0x7));
        tail[1] = static_cast<std::uint8_t>((fpt_len >> 2) & 0xFF);
        detail::uint2ch(m_context.count, &tail[2], 2);

        size_type rmd_len = (m_context.count[0] >> 3) & 0x7F;
        size_type pad_len = (rmd_len < 118) ? (118 - rmd_len) : (246 - rmd_len);

        update(detail::padding, pad_len);
        update(tail, 10);

        detail::tailor_256(m_context);

        detail::uint2ch(m_context.fingerprint,
                        static_cast<std::uint8_t*>(out),
                        fpt_len >> 5);

        std::memset(&m_context, 0, sizeof(m_context));
    }

    std::string end()
    {
        std::string result(result_size, '\0');
        end_to(&result[0]);
        return result;
    }

    static std::string hash(const void* data, size_type len)
    {
        haval ctx;
        ctx.start();
        ctx.update(data, len);
        return ctx.end();
    }

    static std::string hash(const std::string& s)
    {
        return hash(s.data(), s.size());
    }

private:
    void hash_block()
    {
        auto t0 = m_context.fingerprint[0];
        auto t1 = m_context.fingerprint[1];
        auto t2 = m_context.fingerprint[2];
        auto t3 = m_context.fingerprint[3];
        auto t4 = m_context.fingerprint[4];
        auto t5 = m_context.fingerprint[5];
        auto t6 = m_context.fingerprint[6];
        auto t7 = m_context.fingerprint[7];

        detail::hash_block<pass_cnt, pass_cnt>(
            t0, t1, t2, t3, t4, t5, t6, t7, m_context.block);

        m_context.fingerprint[0] += t0;
        m_context.fingerprint[1] += t1;
        m_context.fingerprint[2] += t2;
        m_context.fingerprint[3] += t3;
        m_context.fingerprint[4] += t4;
        m_context.fingerprint[5] += t5;
        m_context.fingerprint[6] += t6;
        m_context.fingerprint[7] += t7;
    }

    detail::haval_context m_context{};
};

using haval256_5 = haval<5, 256>;

} // namespace haval

// helper: binary string -> hex lowercase
static std::string to_hex(const std::string& bin)
{
    std::ostringstream os;
    os << std::hex << std::nouppercase << std::setfill('0');
    for (unsigned char c : bin) {
        os << std::setw(2) << static_cast<int>(c);
    }
    return os.str();
}

int main()
{
    std::string s;
    std::getline(std::cin, s);
    auto digest = haval::haval256_5::hash(s);
    std::cout << to_hex(digest) << std::endl;
    return 0;
}
