#include "WavReader.h"

#include <cstdint>

WavReader::WavReader(std::istream& stream)
    : m_stream(stream)
{
}

bool WavReader::ReadChunkHeader(WavChunkHeader& chunkHeader) const
{
    m_stream.read(reinterpret_cast<char*>(&chunkHeader), sizeof(chunkHeader));

    return static_cast<bool>(m_stream);
}

bool WavReader::ReadPcmHeader(WavMetaData& metaData, size_t& dataLength) const
{
    WavChunkHeader riffHeader{};
    if (!ReadChunkHeader(riffHeader) || riffHeader.chunkID != WAV_CHUNK_ID_RIFF)
        return false;

    uint32_t waveId = 0u;
    m_stream.read(reinterpret_cast<char*>(&waveId), sizeof(waveId));
    if (!m_stream || waveId != WAV_WAVE_ID)
        return false;

    auto foundFormat = false;
    WavChunkHeader chunkHeader{};
    while (ReadChunkHeader(chunkHeader))
    {
        if (chunkHeader.chunkID == WAV_CHUNK_ID_FMT)
        {
            if (chunkHeader.chunkSize < sizeof(WavFormatChunkPcm))
                return false;

            WavFormatChunkPcm formatChunk{};
            m_stream.read(reinterpret_cast<char*>(&formatChunk), sizeof(formatChunk));
            if (!m_stream || formatChunk.wFormatTag != WavFormat::PCM)
                return false;

            metaData.channelCount = formatChunk.nChannels;
            metaData.samplesPerSec = formatChunk.nSamplesPerSec;
            metaData.bitsPerSample = formatChunk.wBitsPerSample;
            foundFormat = true;

            m_stream.seekg(chunkHeader.chunkSize - sizeof(formatChunk), std::ios::cur);
        }
        else if (chunkHeader.chunkID == WAV_CHUNK_ID_DATA)
        {
            if (!foundFormat)
                return false;

            dataLength = chunkHeader.chunkSize;
            return true;
        }
        else
        {
            m_stream.seekg(chunkHeader.chunkSize + (chunkHeader.chunkSize & 1u), std::ios::cur);
        }

        if (!m_stream)
            return false;
    }

    return false;
}

bool WavReader::ReadData(void* buffer, const size_t dataLength) const
{
    m_stream.read(static_cast<char*>(buffer), static_cast<std::streamsize>(dataLength));

    return static_cast<bool>(m_stream);
}
