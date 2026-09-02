#include "FxCommon.h"

#include <format>

namespace
{
    constexpr auto DEFAULT_ASSET_NAME = "default";
}

namespace fx
{
    std::string GetEfxFileNameForAssetName(const std::string& assetName)
    {
        return std::format("fx/{}.efx", assetName);
    }

    std::string GetImpactFxJsonFileNameForAssetName(const std::string& assetName)
    {
        return std::format("impactfx/{}.json", assetName.empty() ? DEFAULT_ASSET_NAME : assetName);
    }
} // namespace fx
