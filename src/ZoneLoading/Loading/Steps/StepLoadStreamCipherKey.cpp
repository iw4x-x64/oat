#include "StepLoadStreamCipherKey.h"

#include "Loading/Exception/UnexpectedEndOfFileException.h"

#include <cassert>
#include <stdexcept>
#include <vector>

namespace
{
    class StepLoadStreamCipherKey final : public step::IStepLoadStreamCipherKey
    {
    public:
        StepLoadStreamCipherKey(std::unique_ptr<cryptography::IPublicKeyAlgorithm> rsa, const size_t encryptedSize, const size_t keySize, const size_t ivSize)
            : m_rsa(std::move(rsa)),
              m_encrypted_size(encryptedSize),
              m_key_size(keySize),
              m_iv_size(ivSize)
        {
        }

        void PerformStep(ZoneLoader& zoneLoader, ILoadingStream& stream) override
        {
            std::vector<uint8_t> encrypted(m_encrypted_size);
            if (stream.Load(encrypted.data(), m_encrypted_size) != m_encrypted_size)
                throw UnexpectedEndOfFileException();

            std::vector<uint8_t> decrypted(m_encrypted_size);
            auto decryptedSize = decrypted.size();
            if (!m_rsa->Decrypt(encrypted.data(), encrypted.size(), decrypted.data(), decryptedSize) || decryptedSize < m_key_size + m_iv_size)
                throw std::runtime_error("Could not decrypt the zone key");

            m_cipher = cryptography::CreateAesCtr(decrypted.data(), m_key_size);
            m_cipher->SetIv(&decrypted[m_key_size], m_iv_size);
        }

        cryptography::IStreamCipher& GetStreamCipher() override
        {
            assert(m_cipher);

            return *m_cipher;
        }

    private:
        std::unique_ptr<cryptography::IPublicKeyAlgorithm> m_rsa;
        size_t m_encrypted_size;
        size_t m_key_size;
        size_t m_iv_size;
        std::unique_ptr<cryptography::IStreamCipher> m_cipher;
    };
} // namespace

namespace step
{
    std::unique_ptr<IStepLoadStreamCipherKey>
        CreateStepLoadStreamCipherKey(std::unique_ptr<cryptography::IPublicKeyAlgorithm> rsa, const size_t encryptedSize, const size_t keySize, const size_t ivSize)
    {
        return std::make_unique<StepLoadStreamCipherKey>(std::move(rsa), encryptedSize, keySize, ivSize);
    }
} // namespace step
