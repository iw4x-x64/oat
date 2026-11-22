#include "fximpacttable_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/XAssets/fxeffectdef/fxeffectdef_iw4_load_db.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>
#include <stdexcept>

using namespace IW4;

namespace
{
    constexpr size_t IMPACT_ENTRY_SIZE = 140u;
    constexpr size_t IMPACT_ENTRY_COUNT = 15u;
    constexpr size_t IMPACT_ENTRY_ALPHA_COUNT = 16u;
} // namespace

Actions_FxImpactTable::Actions_FxImpactTable(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_FxImpactTable::LoadFxImpactEntryLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(IMPACT_ENTRY_SIZE * count);

    if (count != IMPACT_ENTRY_COUNT)
        throw std::runtime_error("Pre-release impact tables can only be loaded as a whole");

    AlphaFill fill(stream, m_zone, IMPACT_ENTRY_SIZE * IMPACT_ENTRY_ALPHA_COUNT, IMPACT_ENTRY_SIZE * IMPACT_ENTRY_COUNT, 1u);
    fill.Move(0u, 0u, 0u, IMPACT_ENTRY_SIZE * IMPACT_ENTRY_COUNT);
    std::memcpy(m_alpha_entry, fill.Alpha(0u) + IMPACT_ENTRY_SIZE * IMPACT_ENTRY_COUNT, sizeof(m_alpha_entry));
    m_stream = &stream;

    return fill.Accessor();
}

void Actions_FxImpactTable::LoadAlphaImpactEntry(FxImpactTable* impactTable) const
{
    if (m_stream == nullptr)
        return;

    for (auto& raw : m_alpha_entry)
    {
        if (raw == 0u)
            continue;

        FxEffectDef* effect = nullptr;
        std::memcpy(&effect, &raw, sizeof(raw));

        Loader_FxEffectDef loader(m_zone, *m_stream);
        loader.Load(&effect);
        raw = 0u;
    }

    m_stream = nullptr;
}
