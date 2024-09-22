#include "headers/Crypto.h"
#include <stdexcept>
#include <openssl/rand.h>

Crypto::Crypto(const std::string& token) : token(token) {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

Crypto::~Crypto() {
    EVP_cleanup();
    ERR_free_strings();
}

std::vector<uint8_t> Crypto::deriveKey(const std::vector<uint8_t>& salt) const {
    std::vector<uint8_t> key(KEY_SIZE);

    if (PKCS5_PBKDF2_HMAC(
        token.c_str(), token.length(),
        salt.data(), salt.size(),
        ITERATION_COUNT,
        EVP_sha256(),
        KEY_SIZE, key.data()) != 1) {
        throw std::runtime_error("Key derivation failed");
    }

    return key;
}

std::vector<uint8_t> Crypto::encrypt(const std::string& plaintext, std::vector<uint8_t>& iv, std::vector<uint8_t>& tag) const {
    // Generate a random salt
    std::vector<uint8_t> salt(FileHeader::SALT_SIZE);
    if (RAND_bytes(salt.data(), FileHeader::SALT_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }

    // Derive the key
    std::vector<uint8_t> key = deriveKey(salt);

    // Generate a random IV
    iv.resize(FileHeader::IV_SIZE);
    if (RAND_bytes(iv.data(), FileHeader::IV_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random IV");
    }

    // Prepare the encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }

    // Initialize the encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    // Encrypt the plaintext
    std::vector<uint8_t> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
    int len = 0, ciphertext_len = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                          reinterpret_cast<const uint8_t*>(plaintext.data()), plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }
    ciphertext_len = len;

    // Finalize the encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize encryption");
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    // Get the tag
    tag.resize(FileHeader::AUTH_TAG_SIZE);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, FileHeader::AUTH_TAG_SIZE, tag.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to get authentication tag");
    }

    EVP_CIPHER_CTX_free(ctx);

    // Prepend the salt to the ciphertext
    ciphertext.insert(ciphertext.begin(), salt.begin(), salt.end());

    return ciphertext;
}

std::string Crypto::decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& iv, const std::vector<uint8_t>& tag) const {
    // Extract the salt from the ciphertext
    std::vector<uint8_t> salt(ciphertext.begin(), ciphertext.begin() + FileHeader::SALT_SIZE);
    std::vector<uint8_t> actualCiphertext(ciphertext.begin() + FileHeader::SALT_SIZE, ciphertext.end());

    // Derive the key
    std::vector<uint8_t> key = deriveKey(salt);

    // Prepare the decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }

    // Initialize the decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    // Decrypt the ciphertext
    std::vector<uint8_t> plaintext(actualCiphertext.size());
    int len = 0, plaintext_len = 0;
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, actualCiphertext.data(), actualCiphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }
    plaintext_len = len;

    // Set the expected tag value
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, FileHeader::AUTH_TAG_SIZE, const_cast<uint8_t*>(tag.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set authentication tag");
    }

    // Finalize the decryption
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed (authentication tag mismatch?)");
    }
    plaintext_len += len;
    plaintext.resize(plaintext_len);

    EVP_CIPHER_CTX_free(ctx);

    return std::string(plaintext.begin(), plaintext.end());
}