#include "gfxworld_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/XAssets/material/material_iw4_load_db.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>
#include <stdexcept>

using namespace IW4;

namespace
{
    constexpr size_t GFXWORLD_SIZE = 628u;
    constexpr size_t GFXWORLD_ALPHA_SIZE = GFXWORLD_SIZE + 968u;
    constexpr size_t GFXWORLD_SORT_KEY_EFFECT_DECAL = 40u;
    constexpr size_t GFXWORLD_SORT_KEY_EFFECT_AUTO = 44u;
    constexpr size_t GFXWORLD_SORT_KEY_DISTORTION = 48u;
    constexpr uint32_t SORTKEY_EFFECT_DECAL = 39u;
    constexpr uint32_t SORTKEY_EFFECT_AUTO = 48u;
    constexpr uint32_t SORTKEY_DISTORTION = 43u;

    void SetSortKeys(uint8_t* world)
    {
        std::memcpy(world + GFXWORLD_SORT_KEY_EFFECT_DECAL, &SORTKEY_EFFECT_DECAL, sizeof(SORTKEY_EFFECT_DECAL));
        std::memcpy(world + GFXWORLD_SORT_KEY_EFFECT_AUTO, &SORTKEY_EFFECT_AUTO, sizeof(SORTKEY_EFFECT_AUTO));
        std::memcpy(world + GFXWORLD_SORT_KEY_DISTORTION, &SORTKEY_DISTORTION, sizeof(SORTKEY_DISTORTION));
    }

    constexpr size_t SUN = 252u;
    constexpr size_t SUN_MATERIALS_END = 12u;
    constexpr size_t SUN_ALPHA_MATERIALS = 12u;
    constexpr size_t SUN_ALPHA_REST = 20u;
    constexpr size_t SUN_REST_SIZE = 84u;

    constexpr size_t GFXWORLD_AFTER_SUN = 348u;
    constexpr size_t GFXWORLD_ALPHA_AFTER_SUN = 1316u;
} // namespace

Actions_GfxWorld::Actions_GfxWorld(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_GfxWorld::LoadGfxWorldLayout(ZoneInputStream& stream, const size_t count) const
{
    m_stream = &stream;
    m_sun_materials[0] = 0u;
    m_sun_materials[1] = 0u;

    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(GFXWORLD_SIZE * count);

    if (count != 1u)
        throw std::runtime_error("Pre-release gfx worlds can only be loaded one at a time");

    if (m_zone.m_version < ZoneConstants::ZONE_VERSION_ALPHA_WORLD)
    {
        AlphaFill fill(stream, m_zone, GFXWORLD_SIZE, GFXWORLD_SIZE, count);
        fill.Move(0u, 0u, 0u, GFXWORLD_SIZE);
        SetSortKeys(fill.Game(0u));

        return fill.Accessor();
    }

    AlphaFill fill(stream, m_zone, GFXWORLD_ALPHA_SIZE, GFXWORLD_SIZE, count);
    fill.Move(0u, 0u, 0u, SUN + SUN_MATERIALS_END);
    fill.Move(0u, SUN + SUN_MATERIALS_END, SUN + SUN_ALPHA_REST, SUN_REST_SIZE);
    fill.Move(0u, GFXWORLD_AFTER_SUN, GFXWORLD_ALPHA_AFTER_SUN, GFXWORLD_SIZE - GFXWORLD_AFTER_SUN);
    std::memcpy(m_sun_materials, fill.Alpha(0u) + SUN + SUN_ALPHA_MATERIALS, sizeof(m_sun_materials));
    SetSortKeys(fill.Game(0u));

    return fill.Accessor();
}

void Actions_GfxWorld::LoadSunflareMaterials(sunflare_t* sun) const
{
    if (m_stream == nullptr)
        return;

    for (auto& raw : m_sun_materials)
    {
        if (raw == 0u)
            continue;

        Material* material = nullptr;
        std::memcpy(&material, &raw, sizeof(raw));

        Loader_Material loader(m_zone, *m_stream);
        loader.Load(&material);
        raw = 0u;
    }
}
