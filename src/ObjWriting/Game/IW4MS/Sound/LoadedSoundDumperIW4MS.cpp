#include "LoadedSoundDumperIW4MS.h"

#include "Sound/LoadedSoundCommon.h"
#include "Sound/WavTypes.h"
#include "Sound/WavWriter.h"
#include "Utils/Logging/Log.h"

#include <format>

using namespace IW4MS;

namespace
{
    void DumpWavPcm(const LoadedSound* asset, std::ostream& stream)
    {
        const WavWriter writer(stream);

        const WavMetaData metaData{.channelCount = static_cast<unsigned>(asset->sound.nChannels),
                                   .samplesPerSec = static_cast<unsigned>(asset->sound.nSamplesPerSec),
                                   .bitsPerSample = static_cast<unsigned>(asset->sound.wBitsPerSample)};

        writer.WritePcmHeader(metaData, asset->sound.data_len);
        writer.WritePcmData(asset->sound.data, asset->sound.data_len);
    }
} // namespace

namespace sound
{
    void LoadedSoundDumperIW4MS::DumpAsset(AssetDumpingContext& context, const XAssetInfo<AssetLoadedSound::Type>& asset)
    {
        const auto* loadedSound = asset.Asset();
        const auto assetFile = context.OpenAssetFile(sound::GetLoadedSoundFileNameForAssetName(asset.m_name));

        if (!assetFile)
            return;

        auto& stream = *assetFile;
        switch (static_cast<WavFormat>(loadedSound->sound.wFormatTag))
        {
        case WavFormat::PCM:
            DumpWavPcm(loadedSound, stream);
            break;

        default:
            con::error("Unknown format {} for loaded sound: {}", loadedSound->sound.wFormatTag, loadedSound->name);
            break;
        }
    }
} // namespace sound
