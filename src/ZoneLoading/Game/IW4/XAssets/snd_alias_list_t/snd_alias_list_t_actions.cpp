#include "snd_alias_list_t_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

using namespace IW4;

namespace
{
    constexpr size_t SND_ALIAS_SIZE = 100u;
    constexpr size_t SND_ALIAS_ALPHA_SIZE = 108u;
} // namespace

Actions_snd_alias_list_t::Actions_snd_alias_list_t(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_snd_alias_list_t::Loadsnd_alias_tLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(SND_ALIAS_SIZE * count);

    AlphaFill fill(stream, m_zone, SND_ALIAS_ALPHA_SIZE, SND_ALIAS_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        fill.Move(i, 0u, 0u, 60u);
        fill.Move(i, 60u, 68u, 20u);
        fill.Move(i, 80u, 88u, 20u);
    }

    return fill.Accessor();
}
