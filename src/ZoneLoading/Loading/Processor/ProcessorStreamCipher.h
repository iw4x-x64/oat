#pragma once

#include "Cryptography.h"
#include "Loading/IStreamCipherProvider.h"
#include "Loading/StreamProcessor.h"

#include <memory>

namespace processor
{
    std::unique_ptr<StreamProcessor> CreateProcessorStreamCipher(std::unique_ptr<cryptography::IStreamCipher> cipher);
    std::unique_ptr<StreamProcessor> CreateProcessorStreamCipher(IStreamCipherProvider* cipherProvider);
} // namespace processor
