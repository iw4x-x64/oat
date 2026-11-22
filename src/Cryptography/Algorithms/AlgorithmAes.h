#pragma once

#include "IStreamCipher.h"

#include <memory>

namespace cryptography
{
    std::unique_ptr<IStreamCipher> CreateAesCtr(const uint8_t* keyBytes, size_t keySize);
} // namespace cryptography
