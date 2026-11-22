#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

namespace IW4
{
    class Actions_snd_alias_list_t final : public AssetLoadingActions
    {
    public:
        explicit Actions_snd_alias_list_t(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor Loadsnd_alias_tLayout(ZoneInputStream& stream, size_t count) const;
    };
} // namespace IW4
