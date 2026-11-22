#include "physpreset_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

using namespace IW4;

namespace
{
    constexpr size_t PHYS_PRESET_SIZE = 44u;
    constexpr size_t PHYS_PRESET_ALPHA_SIZE = 68u;
} // namespace

Actions_PhysPreset::Actions_PhysPreset(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_PhysPreset::LoadPhysPresetLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(PHYS_PRESET_SIZE * count);

    return LoadAlphaPrefix(stream, m_zone, PHYS_PRESET_ALPHA_SIZE, PHYS_PRESET_SIZE, count);
}
