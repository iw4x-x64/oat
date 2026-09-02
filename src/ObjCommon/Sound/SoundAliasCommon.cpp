#include "SoundAliasCommon.h"

#include <format>

namespace sound
{
    std::string GetSoundAliasCsvFileNameForAssetName(const std::string& assetName)
    {
        return std::format("soundaliases/{}.csv", assetName);
    }
} // namespace sound
