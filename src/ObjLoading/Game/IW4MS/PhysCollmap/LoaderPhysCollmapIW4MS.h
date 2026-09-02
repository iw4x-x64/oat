#pragma once

#include "Asset/IAssetCreator.h"
#include "Game/IW4MS/IW4MS.h"
#include "SearchPath/ISearchPath.h"
#include "Utils/MemoryManager.h"

#include <memory>

namespace phys_collmap
{
    std::unique_ptr<AssetCreator<IW4MS::AssetPhysCollMap>> CreateLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath);
} // namespace phys_collmap
