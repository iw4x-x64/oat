#pragma once

#include "IStreamCipher.h"

class IStreamCipherProvider
{
public:
    IStreamCipherProvider() = default;
    virtual ~IStreamCipherProvider() = default;
    IStreamCipherProvider(const IStreamCipherProvider& other) = default;
    IStreamCipherProvider(IStreamCipherProvider&& other) noexcept = default;
    IStreamCipherProvider& operator=(const IStreamCipherProvider& other) = default;
    IStreamCipherProvider& operator=(IStreamCipherProvider&& other) noexcept = default;

    virtual cryptography::IStreamCipher& GetStreamCipher() = 0;
};
