#include "material_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <string_view>

using namespace IW4;

namespace
{
    constexpr size_t MATERIAL_SIZE = 96u;
    constexpr size_t MATERIAL_INFO_SIZE = 24u;
    constexpr size_t MATERIAL_SORT_KEY = 5u;
    constexpr uint8_t SORTKEY_DISTORTION_ALPHA = 44u;
    constexpr uint8_t SORTKEY_DISTORTION = 43u;

    constexpr std::string_view DISTORTION_MATERIALS[]{
        "gfx_distortion_knife_trail",
        "gfx_distortion_heat_far",
        "gfx_distortion_ring_light",
        "gfx_distortion_heat",
    };
} // namespace

Actions_Material::Actions_Material(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_Material::LoadMaterialLayout(ZoneInputStream& stream, const size_t count) const
{
    if (m_zone.m_version < ZoneConstants::ZONE_VERSION_ALPHA_MATERIAL_INFO)
        return stream.LoadWithFill(MATERIAL_SIZE * count);

    AlphaFill fill(stream, m_zone, MATERIAL_SIZE, MATERIAL_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        fill.Move(i, 0u, 8u, 4u);
        fill.Move(i, 4u, 18u, 1u);
        fill.Move(i, 5u, 20u, 3u);
        fill.Move(i, 8u, 0u, 8u);
        fill.Move(i, 16u, 12u, 6u);
        fill.Move(i, MATERIAL_INFO_SIZE, MATERIAL_INFO_SIZE, MATERIAL_SIZE - MATERIAL_INFO_SIZE);

        if (fill.Game(i)[MATERIAL_SORT_KEY] == SORTKEY_DISTORTION_ALPHA)
            fill.Game(i)[MATERIAL_SORT_KEY] = SORTKEY_DISTORTION;
    }

    return fill.Accessor();
}

void Actions_Material::FixDistortionSortKey(Material* material) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version) || material->info.name == nullptr)
        return;

    if (material->info.sortKey < SORTKEY_DISTORTION)
        return;

    for (const auto name : DISTORTION_MATERIALS)
    {
        if (name == material->info.name)
        {
            material->info.sortKey = SORTKEY_DISTORTION_ALPHA;
            return;
        }
    }
}
