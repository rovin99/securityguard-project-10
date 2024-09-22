#pragma once
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include "FileStructure.h"

class Crypto {
public:
    Crypto(const std::string& token);
    ~Crypto();

    std::vector<uint8_t> deriveKey(const std::vector<uint8_t>& salt) const;
    std::vector<uint8_t> encrypt(const std::string& plaintext, std::vector<uint8_t>& iv, std::vector<uint8_t>& tag) const;
    std::string decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& iv, const std::vector<uint8_t>& tag) const;

private:
    std::string token;
    static constexpr int KEY_SIZE = 32; // 256 bits
    static constexpr int ITERATION_COUNT = 10000; // Adjust based on your security/performance requirements
};
