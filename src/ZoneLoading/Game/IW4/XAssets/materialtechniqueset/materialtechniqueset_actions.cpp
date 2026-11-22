#include "materialtechniqueset_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

using namespace IW4;

namespace
{
    constexpr size_t TECHSET_SIZE = 204u;
    constexpr size_t TECHSET_ALPHA_SIZE = 208u;
    constexpr size_t TECHSET_TECHNIQUES = 12u;
    constexpr size_t TECHSET_ALPHA_TECHNIQUES = 16u;
} // namespace

Actions_MaterialTechniqueSet::Actions_MaterialTechniqueSet(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_MaterialTechniqueSet::LoadMaterialTechniqueSetLayout(ZoneInputStream& stream, const size_t count) const
{
    if (m_zone.m_version < ZoneConstants::ZONE_VERSION_ALPHA_TECHSET_REMAP)
        return stream.LoadWithFill(TECHSET_SIZE * count);

    AlphaFill fill(stream, m_zone, TECHSET_ALPHA_SIZE, TECHSET_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        fill.Move(i, 0u, 0u, TECHSET_TECHNIQUES);
        fill.Move(i, TECHSET_TECHNIQUES, TECHSET_ALPHA_TECHNIQUES, TECHSET_SIZE - TECHSET_TECHNIQUES);
    }

    return fill.Accessor();
}

void Actions_MaterialTechniqueSet::RemapAlphaCodeConsts(MaterialPass* pass) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version) || pass->args == nullptr)
        return;

    const auto world = m_zone.m_version >= ZoneConstants::ZONE_VERSION_ALPHA_WORLD;
    const auto count = pass->perPrimArgCount + pass->perObjArgCount + pass->stableArgCount;
    for (auto i = 0; i < count; i++)
    {
        auto& arg = pass->args[i];
        if (arg.type != MTL_ARG_CODE_VERTEX_CONST && arg.type != MTL_ARG_CODE_PIXEL_CONST)
            continue;

        auto& index = arg.u.codeConst.index;
        if (index >= 58u && index <= 135u)
        {
            index -= 3u;
            if (world)
            {
                index -= 7u;
                if (index <= 53u)
                    index += 1u;
            }
        }
        else if (index >= 11u && index < 58u)
        {
            index -= 2u;
            if (world)
            {
                if (index > 15u && index < 30u)
                {
                    index -= 1u;
                    if (index == 19u)
                        index = 21u;
                }
                else if (index >= 50u)
                    index += 6u;
            }
        }
    }
}
