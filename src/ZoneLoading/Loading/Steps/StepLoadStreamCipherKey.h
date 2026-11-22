#pragma once

#include "Cryptography.h"
#include "Loading/ILoadingStep.h"
#include "Loading/IStreamCipherProvider.h"

#include <memory>

namespace step
{
    class IStepLoadStreamCipherKey : public ILoadingStep, public IStreamCipherProvider
    {
    };

    std::unique_ptr<IStepLoadStreamCipherKey>
        CreateStepLoadStreamCipherKey(std::unique_ptr<cryptography::IPublicKeyAlgorithm> rsa, size_t encryptedSize, size_t keySize, size_t ivSize);
} // namespace step
