#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

namespace IW4
{
    class Actions_Material final : public AssetLoadingActions
    {
    public:
        explicit Actions_Material(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadMaterialLayout(ZoneInputStream& stream, size_t count) const;
        void FixDistortionSortKey(Material* material) const;
    };
} // namespace IW4
