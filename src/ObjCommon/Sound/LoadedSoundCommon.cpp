#include "LoadedSoundCommon.h"

#include <format>

namespace sound
{
    std::string GetLoadedSoundFileNameForAssetName(const std::string& assetName)
    {
        return std::format("sound/{}", assetName);
    }
} // namespace sound
