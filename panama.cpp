// sudo apt update
// sudo apt install libcryptopp-dev libcryptopp-tools


#include <iostream>
#include <string>

#include <cryptopp/panama.h>   // PanamaHash
#include <cryptopp/hex.h>      // HexEncoder
#include <cryptopp/filters.h>  // StringSource, HashFilter, StringSink

using namespace CryptoPP;
using std::string;
using std::cout;
using std::cin;
using std::endl;

string panama_hash(const string& msg) {
    Weak::PanamaHash<> hash;   // Panama-LE hash 256-bit
    string digest;

    StringSource ss(
        msg,
        true, // pump all data immediately
        new HashFilter(
            hash,
            new HexEncoder(
                new StringSink(digest),
                false // false = lowercase hex
            )
        )
    );

    return digest;
}

int main() {
    string input;

    cout << "Masukkan teks: ";
    std::getline(cin, input);

    string h = panama_hash(input);
    cout << "Panama: " << h << endl;

    return 0;
}
