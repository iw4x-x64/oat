#pragma once

#include "Dumping/AbstractAssetDumper.h"
#include "Game/IW4MS/IW4MS.h"

namespace map_ents
{
    class JsonDumperIW4MS final : public AbstractAssetDumper<IW4MS::AssetMapEnts>
    {
    protected:
        void DumpAsset(AssetDumpingContext& context, const XAssetInfo<IW4MS::AssetMapEnts::Type>& asset) override;
    };
} // namespace map_ents
