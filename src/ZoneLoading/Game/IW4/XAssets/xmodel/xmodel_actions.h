#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

#include <cstdint>

namespace IW4
{
    class Actions_XModel final : public AssetLoadingActions
    {
    public:
        explicit Actions_XModel(Zone& zone);

        void SetModelSurfs(XModelLodInfo* lodInfo, XModelSurfs* modelSurfs) const;

        [[nodiscard]] ZoneStreamFillReadAccessor LoadXModelLayout(ZoneInputStream& stream, size_t count) const;
        [[nodiscard]] ZoneStreamFillReadAccessor LoadXModelSurfsLayout(ZoneInputStream& stream, size_t count) const;
        [[nodiscard]] ZoneStreamFillReadAccessor LoadXSurfaceLayout(ZoneInputStream& stream, size_t count) const;
        void LoadXModelLodInfoName(XModelLodInfo* lodInfo) const;

    private:
        mutable ZoneInputStream* m_stream = nullptr;
        mutable uint32_t m_lod_names[4]{};
        mutable unsigned m_lod_index = 0;
    };
} // namespace IW4
