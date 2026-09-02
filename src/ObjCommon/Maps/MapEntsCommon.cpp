#include "MapEntsCommon.h"

#include <format>

namespace map_ents
{
    std::string GetEntsFileNameForAssetName(const std::string& assetName)
    {
        return std::format("{}.ents", assetName);
    }

    std::string GetTriggersFileNameForAssetName(const std::string& assetName)
    {
        return std::format("{}.ents.json", assetName);
    }
} // namespace map_ents

namespace addon_map_ents
{
    std::string GetEntsFileNameForAssetName(const std::string& assetName)
    {
        return assetName;
    }

    std::string GetTriggersFileNameForAssetName(const std::string& assetName)
    {
        return std::format("{}.json", assetName);
    }
} // namespace addon_map_ents
