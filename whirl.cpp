#include <iostream>
#include <string>
#include <cryptopp/whrlpool.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>

// Fungsi untuk mencetak hash secara vertikal
void printVertical(const std::string& hex, size_t width = 32) {
    for (size_t i = 0; i < hex.size(); i += width) {
        std::cout << hex.substr(i, width) << std::endl;
    }
}

int main() {
    using namespace CryptoPP;

    std::string input;
    std::string hash_hex;

    std::cout << "Masukkan teks: ";
    std::getline(std::cin, input);

    Whirlpool whirl;

    StringSource(
        input,
        true,
        new HashFilter(
            whirl,
            new HexEncoder(
                new StringSink(hash_hex),
                false  // lowercase hex
            )
        )
    );

    std::cout << "\nWhirlpool:" << std::endl;
    printVertical(hash_hex);

    return 0;
}
