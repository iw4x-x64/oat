#pragma once

#include "Sound/WavTypes.h"

#include <cstddef>
#include <istream>

class WavReader
{
public:
    explicit WavReader(std::istream& stream);

    [[nodiscard]] bool ReadPcmHeader(WavMetaData& metaData, size_t& dataLength) const;

    [[nodiscard]] bool ReadData(void* buffer, size_t dataLength) const;

private:
    [[nodiscard]] bool ReadChunkHeader(WavChunkHeader& chunkHeader) const;

    std::istream& m_stream;
};
