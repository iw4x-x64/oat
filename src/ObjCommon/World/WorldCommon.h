#pragma once

#include <string>

namespace world
{
    std::string GetComWorldJsonFileName(const std::string& assetName);
    std::string GetFxWorldJsonFileName(const std::string& assetName);
    std::string GetGameWorldSpJsonFileName(const std::string& assetName);
    std::string GetGameWorldMpJsonFileName(const std::string& assetName);
    std::string GetClipMapSpJsonFileName(const std::string& assetName);
    std::string GetClipMapMpJsonFileName(const std::string& assetName);
    std::string GetGfxWorldJsonFileName(const std::string& assetName);
} // namespace world
