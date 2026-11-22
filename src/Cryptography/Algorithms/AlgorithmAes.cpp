#include "AlgorithmAes.h"

#include "Internal/CryptoLibrary.h"
#include "tomcrypt.h"

#include <cassert>
#include <stdexcept>

using namespace cryptography;

namespace
{
    class AlgorithmAesCtr final : public IStreamCipher
    {
    public:
        AlgorithmAesCtr(const uint8_t* keyBytes, const size_t keySize)
        {
            internal::CryptoLibrary::Init();

            const auto cipher = register_cipher(&aes_desc);
            const uint8_t iv[16]{};

            if (ctr_start(cipher, iv, keyBytes, static_cast<int>(keySize), 0, CTR_COUNTER_LITTLE_ENDIAN, &m_ctr) != CRYPT_OK)
                throw std::invalid_argument("Invalid AES key");
        }

        ~AlgorithmAesCtr() override
        {
            ctr_done(&m_ctr);
        }

        AlgorithmAesCtr(const AlgorithmAesCtr& other) = delete;
        AlgorithmAesCtr(AlgorithmAesCtr&& other) noexcept = delete;
        AlgorithmAesCtr& operator=(const AlgorithmAesCtr& other) = delete;
        AlgorithmAesCtr& operator=(AlgorithmAesCtr&& other) noexcept = delete;

        void SetIv(const uint8_t* iv, const size_t ivSize) override
        {
            assert(ivSize == 16);

            if (ivSize != 16 || ctr_setiv(iv, static_cast<unsigned long>(ivSize), &m_ctr) != CRYPT_OK)
                throw std::invalid_argument("AES IV size must be 16");
        }

        void Process(const void* plainText, void* cipherText, const size_t amount) override
        {
            ctr_encrypt(static_cast<const unsigned char*>(plainText), static_cast<unsigned char*>(cipherText), static_cast<unsigned long>(amount), &m_ctr);
        }

    private:
        symmetric_CTR m_ctr{};
    };
} // namespace

namespace cryptography
{
    std::unique_ptr<IStreamCipher> CreateAesCtr(const uint8_t* keyBytes, const size_t keySize)
    {
        return std::make_unique<AlgorithmAesCtr>(keyBytes, keySize);
    }
} // namespace cryptography
