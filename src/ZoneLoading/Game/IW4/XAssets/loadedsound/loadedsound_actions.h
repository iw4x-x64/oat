#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

namespace IW4
{
    class Actions_LoadedSound final : public AssetLoadingActions
    {
    public:
        explicit Actions_LoadedSound(Zone& zone);

        void SetSoundData(MssSound* sound) const;
        [[nodiscard]] ZoneStreamFillReadAccessor LoadLoadedSoundLayout(ZoneInputStream& stream, size_t count) const;
    };
} // namespace IW4
