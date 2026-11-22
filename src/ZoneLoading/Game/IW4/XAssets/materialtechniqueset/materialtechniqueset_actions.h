#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

namespace IW4
{
    class Actions_MaterialTechniqueSet final : public AssetLoadingActions
    {
    public:
        explicit Actions_MaterialTechniqueSet(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadMaterialTechniqueSetLayout(ZoneInputStream& stream, size_t count) const;
        void RemapAlphaCodeConsts(MaterialPass* pass) const;
    };
} // namespace IW4
