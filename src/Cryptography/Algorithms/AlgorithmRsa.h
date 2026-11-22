#pragma once

#include "IPublicKeyAlgorithm.h"

#include <cstdint>
#include <memory>

namespace cryptography
{
    enum class RsaPaddingMode : std::uint8_t
    {
        RSA_PADDING_PKS1,
        RSA_PADDING_PSS,
        RSA_PADDING_OAEP,
    };

    std::unique_ptr<IPublicKeyAlgorithm> CreateRsa(HashingAlgorithm hashingAlgorithm, RsaPaddingMode paddingMode);
} // namespace cryptography
