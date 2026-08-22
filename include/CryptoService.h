#ifndef CRYPTOSERVICE_H
#define CRYPTOSERVICE_H

#include <string>
#include <stdexcept>

class CryptoService {
public:
    CryptoService();

    // Génération de clés et nombres aléatoires
    std::string generateRandomBytes(size_t length);

    // Hachage
    std::string hashPassword(const std::string& password);

    // Chiffrement/Déchiffrement avec clé secrète (ChaCha20-Poly1305)
    std::string encrypt(const std::string& plaintext, const std::string& key);
    std::string decrypt(const std::string& ciphertext, const std::string& key);
};

#endif