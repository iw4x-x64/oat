#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

#include <array>
#include <cstdint>

namespace IW4
{
    class Actions_GameWorldMp final : public AssetLoadingActions
    {
    public:
        explicit Actions_GameWorldMp(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadGameWorldMpLayout(ZoneInputStream& stream, size_t count) const;
        void LoadAlphaPathData(GameWorldMp* world) const;

    private:
        uint8_t* LoadBytes(unsigned align, size_t size) const;
        void LoadPathNodeTrees(size_t count) const;

        mutable ZoneInputStream* m_stream = nullptr;
        mutable std::array<uint8_t, 68> m_path{};
    };
} // namespace IW4
