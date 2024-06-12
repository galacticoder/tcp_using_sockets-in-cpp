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

std::string aes_decrypt(const std::string& ciphertext, const SecByteBlock& key, const SecByteBlock& iv) {
    std::string plaintext;
    CBC_Mode<AES>::Decryption decryption(key, key.size(), iv);
    StringSource(ciphertext, true, 
        new StreamTransformationFilter(decryption, 
            new StringSink(plaintext), 
            StreamTransformationFilter::PKCS_PADDING)
    );
    return plaintext;
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

// Function to decrypt the AES key and IV with a pre-shared secret
void decryptWithPreSharedKey(const std::string& encryptedData, SecByteBlock& key, SecByteBlock& iv, const SecByteBlock& preSharedKey, const SecByteBlock& preSharedIV) {
    try {
        CBC_Mode<AES>::Decryption decryption(preSharedKey, preSharedKey.size(), preSharedIV);

        std::string plaintext;
        StringSource ss(encryptedData, true,
            new StreamTransformationFilter(decryption,
                new StringSink(plaintext)
            )
        );

        // Extract the decrypted key and IV from the plaintext
        std::string decryptedKey = plaintext.substr(0, key.size());
        std::string decryptedIV = plaintext.substr(key.size());

        // Copy the decrypted key and IV to the output parameters
        memcpy(key.BytePtr(), decryptedKey.data(), key.size());
        memcpy(iv.BytePtr(), decryptedIV.data(), iv.size());
    } catch (const Exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Define AES key and IV lengths
    const int AES_KEY_LENGTH = 32; // 256-bit key
    const int AES_IV_LENGTH = 16;  // 128-bit IV

    // Generate AES key and IV
    SecByteBlock aesKey(AES_KEY_LENGTH);
    SecByteBlock aesIV(AES_IV_LENGTH);
    generate_key_iv(aesKey, aesIV);

    // Print key and IV
    printHex("AES Key", aesKey);
    printHex("AES IV", aesIV);

    // Define a pre-shared secret key and IV for encryption
    SecByteBlock preSharedKey(AES_KEY_LENGTH);
    SecByteBlock preSharedIV(AES_IV_LENGTH);
    generate_key_iv(preSharedKey, preSharedIV);

    // Print pre-shared key and IV
    printHex("Pre-Shared Key", preSharedKey);
    printHex("Pre-Shared IV", preSharedIV);

    // Encrypt the AES key and IV with the pre-shared secret
    std::string encryptedData = encryptWithPreSharedKey(aesKey, aesIV, preSharedKey, preSharedIV);

    // Send the encrypted data to the server
    // (In this example, we'll just simulate sending by printing the encrypted data)
    cout << "Encrypted Data Sent to Server: " << encryptedData << endl;

    // Decrypt the received data from the server
    SecByteBlock receivedKey(AES_KEY_LENGTH);
    SecByteBlock receivedIV(AES_IV_LENGTH);
    decryptWithPreSharedKey(encryptedData, receivedKey, receivedIV, preSharedKey, preSharedIV);

    // Print the decrypted key and IV
    printHex("Received AES Key", receivedKey);
    printHex("Received AES IV", receivedIV);

    // Here you can use the received key and IV to decrypt the message sent by the user to the server
    // Decrypt the message using aes_decrypt function

    return 0;
}