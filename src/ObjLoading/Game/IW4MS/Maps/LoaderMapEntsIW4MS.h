#pragma once

#include "Asset/IAssetCreator.h"
#include "Game/IW4MS/IW4MS.h"
#include "SearchPath/ISearchPath.h"
#include "Utils/MemoryManager.h"

#include <memory>

namespace map_ents
{
    std::unique_ptr<AssetCreator<IW4MS::AssetMapEnts>> CreateLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath);
} // namespace map_ents

namespace addon_map_ents
{
    std::unique_ptr<AssetCreator<IW4MS::AssetAddonMapEnts>> CreateLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath);
} // namespace addon_map_ents
