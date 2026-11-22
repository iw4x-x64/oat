#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

namespace IW4
{
    class Actions_PhysPreset final : public AssetLoadingActions
    {
    public:
        explicit Actions_PhysPreset(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadPhysPresetLayout(ZoneInputStream& stream, size_t count) const;
    };
} // namespace IW4
