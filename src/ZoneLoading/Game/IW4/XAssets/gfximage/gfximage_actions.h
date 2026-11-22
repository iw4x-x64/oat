#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

namespace IW4
{
    class Actions_GfxImage final : public AssetLoadingActions
    {
    public:
        explicit Actions_GfxImage(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadGfxImageLayout(ZoneInputStream& stream, size_t count) const;
        [[nodiscard]] block_t LoadGfxImageLoadDefBlock(block_t block) const;
        void OnImageLoaded(GfxImage* image) const;
        void LoadImageData(GfxImageLoadDef* loadDef, GfxImage* image) const;
    };
} // namespace IW4
