#pragma once

#include "Asset/IAssetCreator.h"
#include "Game/IW4MS/IW4MS.h"
#include "SearchPath/ISearchPath.h"
#include "Utils/MemoryManager.h"

#include <memory>

namespace world
{
    std::unique_ptr<AssetCreator<IW4MS::AssetClipMapSp>> CreateClipMapSpLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath);
    std::unique_ptr<AssetCreator<IW4MS::AssetClipMapMp>> CreateClipMapMpLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath);
} // namespace world
