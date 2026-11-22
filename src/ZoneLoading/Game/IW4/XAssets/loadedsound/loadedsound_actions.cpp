#include "loadedsound_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>

using namespace IW4;

namespace
{
    constexpr size_t LOADED_SOUND_SIZE = 44u;
    constexpr size_t LOADED_SOUND_ALPHA_SIZE = 48u;
} // namespace

Actions_LoadedSound::Actions_LoadedSound(Zone& zone)
    : AssetLoadingActions(zone)
{
}

void Actions_LoadedSound::SetSoundData(MssSound* sound) const
{
    if (sound->info.data_len > 0)
    {
        const auto* tempData = sound->data;
        sound->data = m_zone.Memory().Alloc<char>(sound->info.data_len);
        memcpy(sound->data, tempData, sound->info.data_len);
    }
    else
    {
        sound->data = nullptr;
    }
}

ZoneStreamFillReadAccessor Actions_LoadedSound::LoadLoadedSoundLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(LOADED_SOUND_SIZE * count);

    AlphaFill fill(stream, m_zone, LOADED_SOUND_ALPHA_SIZE, LOADED_SOUND_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        fill.Move(i, 0u, 0u, 28u);
        fill.Move(i, 28u, 32u, 16u);
    }

    return fill.Accessor();
}
