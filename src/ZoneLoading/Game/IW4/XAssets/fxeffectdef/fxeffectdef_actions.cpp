#include "fxeffectdef_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>

using namespace IW4;

namespace
{
    constexpr size_t ELEM_DEF_SIZE = 252u;
    constexpr size_t ELEM_DEF_ALPHA_SIZE = 260u;
    constexpr size_t ELEM_DEF_ALPHA_NAME = 256u;
    constexpr size_t ELEM_DEF_TYPE = 176u;

    constexpr uint8_t ELEM_TYPE_ALPHA_REMOVED = 3u;
    constexpr uint8_t ELEM_TYPE_ALPHA_SHIFTED = 5u;

    constexpr uint32_t FOLLOWING = 0xFFFFFFFFu;
} // namespace

Actions_FxEffectDef::Actions_FxEffectDef(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_FxEffectDef::LoadFxElemDefLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(ELEM_DEF_SIZE * count);

    AlphaFill fill(stream, m_zone, ELEM_DEF_ALPHA_SIZE, ELEM_DEF_SIZE, count);
    m_elem_names.assign(count, 0u);
    m_elem_index = 0u;
    m_stream = &stream;

    for (auto i = 0uz; i < count; i++)
    {
        fill.Move(i, 0u, 0u, ELEM_DEF_SIZE);
        std::memcpy(&m_elem_names[i], fill.Alpha(i) + ELEM_DEF_ALPHA_NAME, sizeof(uint32_t));

        auto& elemType = fill.Game(i)[ELEM_DEF_TYPE];
        if (elemType == ELEM_TYPE_ALPHA_REMOVED)
            elemType = ELEM_TYPE_ALPHA_REMOVED - 1u;
        else if (elemType >= ELEM_TYPE_ALPHA_SHIFTED)
            elemType -= 2u;
    }

    return fill.Accessor();
}

void Actions_FxEffectDef::LoadFxElemDefName(FxElemDef* elemDef) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version) || m_stream == nullptr || m_elem_index >= m_elem_names.size())
        return;

    if (m_elem_names[m_elem_index++] == FOLLOWING)
    {
        auto* name = m_stream->Alloc<char>(1);
        m_stream->LoadNullTerminated(name);
    }
}
