#pragma once

#include <string>

namespace map_ents
{
    std::string GetEntsFileNameForAssetName(const std::string& assetName);

    std::string GetTriggersFileNameForAssetName(const std::string& assetName);
} // namespace map_ents

namespace addon_map_ents
{
    std::string GetEntsFileNameForAssetName(const std::string& assetName);
    std::string GetTriggersFileNameForAssetName(const std::string& assetName);
} // namespace addon_map_ents
