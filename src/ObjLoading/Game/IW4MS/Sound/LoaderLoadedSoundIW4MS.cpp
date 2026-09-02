#include "LoaderLoadedSoundIW4MS.h"

#include "Sound/LoadedSoundCommon.h"
#include "Sound/WavReader.h"
#include "Utils/Logging/Log.h"

#include <cstdint>

using namespace IW4MS;

namespace
{
    class LoadedSoundLoader final : public AssetCreator<AssetLoadedSound>
    {
    public:
        LoadedSoundLoader(MemoryManager& memory, ISearchPath& searchPath)
            : m_memory(memory),
              m_search_path(searchPath)
        {
        }

        AssetCreationResult CreateAsset(const std::string& assetName, AssetCreationContext& context) override
        {
            const auto file = m_search_path.Open(sound::GetLoadedSoundFileNameForAssetName(assetName));
            if (!file.IsOpen())
                return AssetCreationResult::NoAction();

            WavMetaData metaData{};
            size_t dataLength = 0u;

            const WavReader reader(*file.m_stream);
            if (!reader.ReadPcmHeader(metaData, dataLength))
            {
                con::error("Loaded sound \"{}\" is not a pcm wav", assetName);
                return AssetCreationResult::Failure();
            }

            auto* loadedSound = m_memory.Alloc<LoadedSound>();
            loadedSound->name = m_memory.Dup(assetName.c_str());

            auto* data = m_memory.Alloc<char>(dataLength);
            if (!reader.ReadData(data, dataLength))
            {
                con::error("Loaded sound \"{}\" ends before its data chunk does", assetName);
                return AssetCreationResult::Failure();
            }

            const auto blockAlign = static_cast<uint16_t>(metaData.channelCount * metaData.bitsPerSample / 8u);

            auto& sound = loadedSound->sound;
            sound.wFormatTag = static_cast<uint16_t>(WavFormat::PCM);
            sound.nChannels = static_cast<uint16_t>(metaData.channelCount);
            sound.nSamplesPerSec = metaData.samplesPerSec;
            sound.nAvgBytesPerSec = metaData.samplesPerSec * blockAlign;
            sound.nBlockAlign = blockAlign;
            sound.wBitsPerSample = static_cast<uint16_t>(metaData.bitsPerSample);

            sound.cbSize = 0u;

            sound.data_len = static_cast<unsigned>(dataLength);
            sound.data = data;

            return AssetCreationResult::Success(context.AddAsset<AssetLoadedSound>(assetName, loadedSound));
        }

    private:
        MemoryManager& m_memory;
        ISearchPath& m_search_path;
    };
} // namespace

namespace sound
{
    std::unique_ptr<AssetCreator<AssetLoadedSound>> CreateLoadedSoundLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath)
    {
        return std::make_unique<LoadedSoundLoader>(memory, searchPath);
    }
} // namespace sound
