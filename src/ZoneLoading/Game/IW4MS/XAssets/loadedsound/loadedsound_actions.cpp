#include "loadedsound_actions.h"

#include <cstring>

using namespace IW4MS;

Actions_LoadedSound::Actions_LoadedSound(Zone& zone)
    : AssetLoadingActions(zone)
{
}

void Actions_LoadedSound::SetSoundData(MssSound* sound) const
{
    if (sound->data_len > 0)
    {
        const auto* tempData = sound->data;
        sound->data = m_zone.Memory().Alloc<char>(sound->data_len);
        memcpy(sound->data, tempData, sound->data_len);
    }
    else
    {
        sound->data = nullptr;
    }
}
