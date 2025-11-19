// Requirements: 
// sudo apt update
// sudo apt install librhash-dev

#include <iostream>
#include <string>
#include <cstdio>
#include <rhash.h>

int main() {
    // Init library
    rhash_library_init();

    std::string input;
    std::cout << "Masukkan teks: ";
    std::getline(std::cin, input);

    unsigned char digest[32]; // 256-bit

    
    if (rhash_msg(RHASH_GOST94,
                  input.data(),
                  input.size(),
                  digest) < 0) {
        std::cerr << "Error: rhash_msg gagal\n";
        return 1;
    }

    
    std::cout << "\nGOST R 34.11-94 :\n";
    for (int i = 0; i < 32; ++i) {
        std::printf("%02x", digest[i]);
        if ((i + 1) % 8 == 0) std::cout << "\n";
    }

    return 0;
}