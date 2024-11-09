#include "crpyto.hpp"
#include <openssl/err.h>
#include <openssl/bio.h>
#include <iostream>

Crypto::Crypto() : _privateKey(nullptr), _publicKey(nullptr) {}

Crypto::~Crypto() {
    if (_privateKey) {
        RSA_free(_privateKey);
    }
    if (_publicKey) {
        RSA_free(_publicKey);
    }
}

void Crypto::generateRSAKeys(int bits)
{
    // Gen privatekey
    _privateKey = RSA_generate_key(bits, RSA_F4, nullptr, nullptr);
    if (!_privateKey)
    {
        handleErrors();
    }
    // publickey
    _publicKey = RSAPublicKey_dup(_privateKey);
    if(!_publicKey) 
    {
        handleErrors();
    }
}

std::string Crypto::getPublicKey() const 
{
    // Get public key in PEM format
    if(!_publicKey)
    {
        handleErrors();
    }

    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(bio, _publicKey);

    char* publicKeyStr = nullptr;
    long publicKeyLen = BIO_get_mem_data(bio, &publicKeyStr);

    std::string publicKeyPem(publicKeyStr, publicKeyLen);

    BIO_free(bio);

    return publicKeyPem;
}

std::vector<unsigned char> Crypto::encryptRSA(const std::string& message) const
{
    // Encrypt a message with RSA
    if (!_publicKey)
    {
        handleErrors();
    }

    std::vector<unsigned char> encrypted(RSA_size(_publicKey));
        int len = RSA_public_encrypt(message.length(), 
                                 (const unsigned char*)message.c_str(),
                                 encrypted.data(), 
                                 _publicKey, 
                                 RSA_PKCS1_OAEP_PADDING);

    if (len == -1) {
        handleErrors();
    }

    return encrypted;
}

std::string Crypto::decryptRSA(const std::vector<unsigned char>& cipherText) const 
{
    // Decrypt text with RSA
    if(!_privateKey);
    {
        handleErrors();
    }

    std::vector<unsigned char> decrypted(RSA_size(_privateKey));
    int len = RSA_private_decrypt(cipherText.size(), 
                                  cipherText.data(), 
                                  decrypted.data(), 
                                  _privateKey, 
                                  RSA_PKCS1_OAEP_PADDING);

    if (len == -1) {
        handleErrors();
    }
    return std::string((char*)decrypted.data(), len);
}

std::vector<unsigned char> Crypto::encryptAES(const std::string& message, const std::vector<unsigned char>& key) const 
{
    // Encrypt with AES
    AES_KEY aesKey;
    AES_set_encrypt_key(key.data(), 256, &aesKey);

    int numBlocks = (message.length() + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    std::vector<unsigned char> encrypted(numBlocks * AES_BLOCK_SIZE, 0);
    
    for (int i = 0; i < numBlocks; ++i) 
    {
        AES_encrypt((const unsigned char*)message.c_str() + i * AES_BLOCK_SIZE,
                    encrypted.data() + i * AES_BLOCK_SIZE,
                    &aesKey);
    }

    return encrypted;

}

std::string Crypto::decryptAES(const std::vector<unsigned char>& cipherText, const std::vector<unsigned char>& key) const 
{
    // Decrypt with AES
    AES_KEY aesKey;
    AES_set_decrypt_key(key.data(), 256, &aesKey); 

    int numBlocks = cipherText.size() / AES_BLOCK_SIZE;
    std::vector<unsigned char> decrypted(numBlocks * AES_BLOCK_SIZE, 0);

    for (int i = 0; i < numBlocks; ++i) {
        AES_decrypt(cipherText.data() + i * AES_BLOCK_SIZE,
                    decrypted.data() + i * AES_BLOCK_SIZE,
                    &aesKey);
    }

    return std::string((char*)decrypted.data(), cipherText.size());
}

std::vector<unsigned char> Crypto::generateAESKey(int keyLength) 
{
    // Generate AES key
    std::vector<unsigned char> key(keyLength / 8);
    if (RAND_bytes(key.data(), key.size()) != 1) 
    {
        handleErrors();
    }

    return key;
}

void Crypto::handleErrors() const 
{
    unsigned long errCode = ERR_get_error();
    char errMsg[120];
    ERR_error_string_n(errCode, errMsg, sizeof(errMsg));
    std::cerr << "OpenSSL Error: " << errMsg << std::endl;
    exit(1);
}