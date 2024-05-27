#ifndef AES_CRYPT_H
#define AES_CRYPT_H

#include <string>
#include <cryptopp/secblock.h>

using namespace CryptoPP;

std::string aes_encrypt(const std::string& plaintext, const SecByteBlock& key, const SecByteBlock& iv);
void generate_key_iv(::SecByteBlock& key, SecByteBlock& iv);

#endif