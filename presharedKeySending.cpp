#include <iostream>
#include <string>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/secblock.h>
#include "encry_to_server.h"

using namespace std;
using namespace CryptoPP;

// Function to print byte array as hex
void printHex(const std::string& label, const SecByteBlock& data) {
    std::string encoded;
    HexEncoder encoder(new StringSink(encoded));
    encoder.Put(data, data.size());
    encoder.MessageEnd();
    std::cout << label << ": " << encoded << std::endl;
}

void generate_key_iv(SecByteBlock& key, SecByteBlock& iv) {
    AutoSeededRandomPool prng;
    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());
}

std::string aes_encrypt(const std::string& plaintext, const SecByteBlock& key, const SecByteBlock& iv) {
    std::string ciphertext;
    CBC_Mode<AES>::Encryption encryption(key, key.size(), iv);
    StringSource(plaintext, true, 
        new StreamTransformationFilter(encryption, 
            new StringSink(ciphertext), 
            StreamTransformationFilter::PKCS_PADDING)
    );
    return ciphertext;
}

// Function to encrypt the AES key and IV with a pre-shared secret
std::string encryptWithPreSharedKey(const SecByteBlock& key, const SecByteBlock& iv, const SecByteBlock& preSharedKey, const SecByteBlock& preSharedIV) {
    std::string ciphertext;
    try {
        CBC_Mode<AES>::Encryption encryption(preSharedKey, preSharedKey.size(), preSharedIV);

        std::string plaintext(reinterpret_cast<const char*>(key.BytePtr()), key.size());
        plaintext.append(reinterpret_cast<const char*>(iv.BytePtr()), iv.size());

        StringSource ss(plaintext, true,
            new StreamTransformationFilter(encryption,
                new StringSink(ciphertext)
            )
        );
    } catch (const Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    return ciphertext;
}

// Function to send encrypted data (pseudo code)
void sendEncryptedData(const std::string& encryptedData) {
    // Implementation to send the encrypted data over network
    // For example, using sockets or any network library
    std::cout << "Encrypted Data Sent: " << encryptedData << std::endl;
}

int main() {
    const int AES_KEY_LENGTH = 32; // 256-bit key
    const int AES_IV_LENGTH = 16;  // 128-bit IV

    SecByteBlock aesKey(AES_KEY_LENGTH);
    SecByteBlock aesIV(AES_IV_LENGTH);
    generate_key_iv(aesKey, aesIV);

    printHex("AES Key", aesKey);
    printHex("AES IV", aesIV);

    SecByteBlock preSharedKey(AES_KEY_LENGTH);
    SecByteBlock preSharedIV(AES_IV_LENGTH);
    generate_key_iv(preSharedKey, preSharedIV);

    printHex("Pre-Shared Key", preSharedKey);
    printHex("Pre-Shared IV", preSharedIV);

    std::string encryptedData = encryptWithPreSharedKey(aesKey, aesIV, preSharedKey, preSharedIV);
  
    std::string encoded;
    StringSource(encryptedData, true,
        new Base64Encoder(
            new StringSink(encoded)
        )
    );

    sendEncryptedData(encoded);

    return 0;
}