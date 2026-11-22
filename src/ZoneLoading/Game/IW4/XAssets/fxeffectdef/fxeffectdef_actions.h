#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

#include <cstdint>
#include <vector>

namespace IW4
{
    class Actions_FxEffectDef final : public AssetLoadingActions
    {
    public:
        explicit Actions_FxEffectDef(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadFxElemDefLayout(ZoneInputStream& stream, size_t count) const;
        void LoadFxElemDefName(FxElemDef* elemDef) const;

    private:
        mutable ZoneInputStream* m_stream = nullptr;
        mutable std::vector<uint32_t> m_elem_names;
        mutable size_t m_elem_index = 0;
    };
} // namespace IW4
