    #ifndef CRYPTO_HPP
    #define CRYPTO_HPP

    #include <openssl/rsa.h>
    #include <openssl/pem.h>
    #include <openssl/aes.h>
    #include <openssl/rand.h>
    #include <string>
    #include <vector>

    class Crypto
    {
    public:
        // Constructor / Destrucotor
        Crypto();
        ~Crypto();

        // Methdos

        // Generates RSA key pair
        void generateRSAKeys(int bits = 2048);

        // Get public key
        std::string getPublicKey() const;

        // Encrypt with RSA
        std::vector<unsigned char> Crypto::encryptRSA(const std::string& message) const;

        // Decrypt with RSA
        std::string decryptRSA(const std::vector<unsigned char>& cipherText) const;

        // AES encrpytion
        std::vector<unsigned char> encryptAES(const std::string& message, const std::vector<unsigned char>& key) const;

        // AES decryption
        std::string decryptAES(const std::vector<unsigned char>& cipherText, const std::vector<unsigned char>& key) const;

        // Generate a random AES key
        std::vector<unsigned char> generateAESKey(int keyLength = 256);

        // Load the public key
        void Crypto::loadPublicKey(const std::string& publicKeyPem);
    private:
        RSA* _privateKey;
        RSA* _publicKey;

        // Error handling
        void handleErrors() const;
    };

    #endif // CRYPTO_HPP