#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h> 
#include <cryptopp/hex.h>
#include "encry_to_server.h"

using namespace std;
using namespace CryptoPP;

void generate_key_iv(SecByteBlock& key, SecByteBlock& iv) {
    AutoSeededRandomPool prng;
    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());
}

std::string aes_encrypt(const std::string& plaintext, const SecByteBlock& key, const SecByteBlock& iv) {
    std::string ciphertext;
    CBC_Mode<AES>::Encryption encryption(key, key.size(), iv);
    StringSource(plaintext, true, new StreamTransformationFilter(encryption, new StringSink(ciphertext), StreamTransformationFilter::PKCS_PADDING));
    return ciphertext;
}
