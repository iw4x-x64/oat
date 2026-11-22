#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

#include <cstdint>
#include <type_traits>

namespace IW4
{
    class Actions_FxImpactTable final : public AssetLoadingActions
    {
    public:
        explicit Actions_FxImpactTable(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadFxImpactEntryLayout(ZoneInputStream& stream, size_t count) const;
        void LoadAlphaImpactEntry(FxImpactTable* impactTable) const;

    private:
        mutable ZoneInputStream* m_stream = nullptr;
        mutable uint32_t m_alpha_entry[std::extent_v<decltype(FxImpactEntry::nonflesh)> + std::extent_v<decltype(FxImpactEntry::flesh)>]{};
    };
} // namespace IW4
