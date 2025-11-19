#include <iostream>
#include <iomanip>
#include <string>

#include <rhash.h>  // dari paket librhash-dev

int main() {
    // Wajib: init library RHash
    rhash_library_init();

    std::string msg;
    std::cout << "Masukkan teks: ";
    std::getline(std::cin, msg);

    unsigned char digest[32]; // Snefru-256 = 256 bit = 32 byte

    // Hitung hash Snefru-256
    if (rhash_msg(RHASH_SNEFRU256, msg.data(), msg.size(), digest) != 0) {
        std::cerr << "Error: gagal menghitung Snefru-256\n";
        return 1;
    }

    // Konversi ke hex string
    std::cout << "Snefru-256: ";
    std::cout << std::hex << std::setfill('0');
    for (int i = 0; i < 32; ++i) {
        std::cout << std::setw(2) << static_cast<unsigned int>(digest[i]);
    }
    std::cout << std::dec << std::endl;

    return 0;
}
