#ifdef ARCH_x64

#include "Game/IW4/IW4.h"
#include "Game/IW4MS/IW4MS.h"
#include "Zone/Zone.h"
#include "ZoneRetargeter.h"

#include <catch2/catch_test_macros.hpp>
#include <cstring>

namespace
{
    // A sound whose every field is distinguishable, so a value landing in the wrong slot shows up
    // as that value and names where it came from.
    //
    constexpr auto TEST_DATA_LEN = 45742u;
    constexpr auto TEST_RATE = 44100u;
    constexpr auto TEST_BITS = 16;
    constexpr auto TEST_CHANNELS = 2;
    constexpr auto TEST_BLOCK_SIZE = 4u; // channels * bits/8, which is what x86 carries here
    constexpr auto TEST_SAMPLES = 22871u;

    TEST_CASE("IW4MS: retargeting states a loaded sound as a WAVEFORMATEX", "[iw4ms][sound]")
    {
        Zone source("loadedsound", 0, GameId::IW4, GamePlatform::PC);

        auto* sound = source.Memory().Alloc<IW4::LoadedSound>();
        auto* data = source.Memory().Alloc<char>(TEST_DATA_LEN);

        sound->name = "weapons/test/weap_test_fire.wav";
        sound->sound.info.format = 1; // WAVE_FORMAT_PCM, which is also the Miles PCM tag
        sound->sound.info.data_ptr = nullptr;
        sound->sound.info.initial_ptr = nullptr;
        sound->sound.info.data_len = TEST_DATA_LEN;
        sound->sound.info.rate = TEST_RATE;
        sound->sound.info.bits = TEST_BITS;
        sound->sound.info.channels = TEST_CHANNELS;
        sound->sound.info.samples = TEST_SAMPLES;
        sound->sound.info.block_size = TEST_BLOCK_SIZE;
        sound->sound.data = data;

        source.m_pools.AddAsset(IW4::ASSET_TYPE_LOADED_SOUND, sound->name, sound, {}, {}, {});

        const auto retargeted = retarget::Retarget(source, GameId::IW4MS);
        REQUIRE(retargeted != nullptr);

        const auto* asset = retargeted->m_pools.GetAsset(IW4MS::ASSET_TYPE_LOADED_SOUND, sound->name);
        REQUIRE(asset != nullptr);

        const auto& mss = static_cast<const IW4MS::LoadedSound*>(asset->m_ptr)->sound;

        // What XAudio2 is handed. See the layout note on IW4MS::MssSound.
        //
        REQUIRE(mss.wFormatTag == 1u);
        REQUIRE(mss.nChannels == TEST_CHANNELS);
        REQUIRE(mss.nSamplesPerSec == TEST_RATE);
        REQUIRE(mss.nAvgBytesPerSec == TEST_RATE * TEST_BLOCK_SIZE);
        REQUIRE(mss.nBlockAlign == TEST_BLOCK_SIZE);
        REQUIRE(mss.wBitsPerSample == TEST_BITS);
        REQUIRE(mss.cbSize == 0u);

        // The byte count the writer streams the payload with, and the payload itself.
        //
        REQUIRE(mss.data_len == TEST_DATA_LEN);
        REQUIRE(mss.data == data);

        for (auto i = 0u; i < sizeof(mss.unknown_18); i++)
            REQUIRE(mss.unknown_18[i] == 0u);

        for (auto i = 0u; i < sizeof(mss.unknown_28); i++)
            REQUIRE(mss.unknown_28[i] == 0u);
    }

    TEST_CASE("IW4MS: a loaded sound sits where the retail loader reads it", "[iw4ms][sound][retail]")
    {
        using namespace IW4MS;

        // Load_LoadedSound at 0x140119D90 streams 64 bytes and points varMssSound at +8.
        //
        REQUIRE(sizeof(LoadedSound) == 64u);
        REQUIRE(offsetof(LoadedSound, sound) == 8u);

        // Load_MssSound at 0x1401206C0 streams 56 bytes, takes its Load_Stream size from +24 and
        // the payload pointer from +48.
        //
        REQUIRE(sizeof(MssSound) == 56u);
        REQUIRE(offsetof(MssSound, data_len) == 24u);
        REQUIRE(offsetof(MssSound, data) == 48u);

        // sub_1402C6600 reads the channel count at +2, the rate at +4 and the block alignment at
        // +12, which is where WAVEFORMATEX puts them.
        //
        REQUIRE(offsetof(MssSound, wFormatTag) == 0u);
        REQUIRE(offsetof(MssSound, nChannels) == 2u);
        REQUIRE(offsetof(MssSound, nSamplesPerSec) == 4u);
        REQUIRE(offsetof(MssSound, nAvgBytesPerSec) == 8u);
        REQUIRE(offsetof(MssSound, nBlockAlign) == 12u);
        REQUIRE(offsetof(MssSound, wBitsPerSample) == 14u);
        REQUIRE(offsetof(MssSound, cbSize) == 16u);
    }
} // namespace

#endif // ARCH_x64
