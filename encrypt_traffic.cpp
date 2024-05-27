#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h> 
#include <cryptopp/hex.h>
#include "encry_to_server.h"
#include "decry_from_cli.h"


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

std::string aes_decrypt(const std::string& ciphertext, const SecByteBlock& key, const SecByteBlock& iv) {
    std::string decrypted;
    CBC_Mode<AES>::Decryption decryption(key, key.size(), iv);
    StringSource(ciphertext, true, new StreamTransformationFilter(decryption, new StringSink(decrypted), StreamTransformationFilter::PKCS_PADDING));
    return decrypted;
}


int main2() {
    string plaintext = "Hello, World!";
    //key/iv gen
    AutoSeededRandomPool prng;
    SecByteBlock key(AES::DEFAULT_KEYLENGTH);
    SecByteBlock iv(AES::BLOCKSIZE);
    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());
    // string encrypted_text = aes_encrypt(plaintext, key, iv);
    //decry
    // string decrypted_text = aes_decrypt(encrypted_text, key, iv);

    // encdoe
    string encoded_key, encoded_iv;
    StringSource(key, key.size(), true, new HexEncoder(new StringSink(encoded_key)));
    StringSource(iv, iv.size(), true, new HexEncoder(new StringSink(encoded_iv)));

    cout << "Plaintext: " << plaintext << endl;
    // cout << "Encrypted text: " << encrypted_text << endl;
    // cout << "Decrypted text: " << decrypted_text << endl;
    cout << "Key (hex): " << encoded_key << endl;
    cout << "IV (hex): " << encoded_iv << endl;

    return 0;
}
