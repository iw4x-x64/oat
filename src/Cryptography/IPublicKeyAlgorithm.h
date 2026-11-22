#pragma once

#include <cstdint>
#include <cstdlib>

namespace cryptography
{
    enum class HashingAlgorithm : std::uint8_t
    {
        RSA_HASH_SHA256,
        RSA_HASH_SHA512
    };

    class IPublicKeyAlgorithm
    {
    public:
        IPublicKeyAlgorithm() = default;
        virtual ~IPublicKeyAlgorithm() = default;
        IPublicKeyAlgorithm(const IPublicKeyAlgorithm& other) = default;
        IPublicKeyAlgorithm(IPublicKeyAlgorithm&& other) noexcept = default;
        IPublicKeyAlgorithm& operator=(const IPublicKeyAlgorithm& other) = default;
        IPublicKeyAlgorithm& operator=(IPublicKeyAlgorithm&& other) noexcept = default;

        virtual bool SetKey(const uint8_t* keyData, size_t keySize) = 0;

        // If needed, add a signing method

        virtual bool Verify(const uint8_t* signedData, size_t signedDataSize, const uint8_t* signature, size_t signatureSize) = 0;

        virtual bool Decrypt(const uint8_t* cipherText, size_t cipherTextSize, uint8_t* plainText, size_t& plainTextSize) = 0;
    };
} // namespace cryptography
