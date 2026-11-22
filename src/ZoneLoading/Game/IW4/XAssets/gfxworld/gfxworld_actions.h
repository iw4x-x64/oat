#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

#include <cstdint>

namespace IW4
{
    class Actions_GfxWorld final : public AssetLoadingActions
    {
    public:
        explicit Actions_GfxWorld(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadGfxWorldLayout(ZoneInputStream& stream, size_t count) const;
        void LoadSunflareMaterials(sunflare_t* sun) const;

    private:
        mutable ZoneInputStream* m_stream = nullptr;
        mutable uint32_t m_sun_materials[2]{};
    };
} // namespace IW4
