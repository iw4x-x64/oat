#include "WorldCommon.h"

#include <format>

namespace world
{
    std::string GetComWorldJsonFileName(const std::string& assetName)
    {
        return std::format("comworld/{}.json", assetName);
    }

    std::string GetFxWorldJsonFileName(const std::string& assetName)
    {
        return std::format("fxworld/{}.json", assetName);
    }

    std::string GetGameWorldSpJsonFileName(const std::string& assetName)
    {
        return std::format("gameworld_sp/{}.json", assetName);
    }

    std::string GetGameWorldMpJsonFileName(const std::string& assetName)
    {
        return std::format("gameworld_mp/{}.json", assetName);
    }

    std::string GetClipMapSpJsonFileName(const std::string& assetName)
    {
        return std::format("clipmap_sp/{}.json", assetName);
    }

    std::string GetClipMapMpJsonFileName(const std::string& assetName)
    {
        return std::format("clipmap_mp/{}.json", assetName);
    }

    std::string GetGfxWorldJsonFileName(const std::string& assetName)
    {
        return std::format("gfxworld/{}.json", assetName);
    }
} // namespace world
