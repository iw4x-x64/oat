#include "AlphaWeaponIW4.h"

#include "Game/IW4/XAssets/fxeffectdef/fxeffectdef_iw4_load_db.h"
#include "Game/IW4/XAssets/material/material_iw4_load_db.h"
#include "Game/IW4/XAssets/physcollmap/physcollmap_iw4_load_db.h"
#include "Game/IW4/XAssets/physpreset/physpreset_iw4_load_db.h"
#include "Game/IW4/XAssets/tracerdef/tracerdef_iw4_load_db.h"
#include "Game/IW4/XAssets/xmodel/xmodel_iw4_load_db.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>
#include <vector>

using namespace IW4;

namespace
{
    constexpr uint32_t FOLLOWING = 0xFFFFFFFFu;
    constexpr size_t SOUND_ARRAY_COUNT = 31u;

    constexpr int VERSION_ALPHA3 = 318;
    constexpr int VERSION_WEAPON_FLAT = 359;

    size_t WeaponSize(const int version)
    {
        if (version >= VERSION_WEAPON_FLAT)
            return 3120u;
        if (version >= ZoneConstants::ZONE_VERSION_ALPHA_IMAGE)
            return 3068u;
        if (version >= VERSION_ALPHA3)
            return 3156u;

        return 3112u;
    }

    template<typename T, typename Loader> void LoadHandle(Zone& zone, ZoneInputStream& stream, const uint32_t raw)
    {
        if (raw == 0u)
            return;

        T* asset = nullptr;
        std::memcpy(&asset, &raw, sizeof(raw));

        Loader loader(zone, stream);
        loader.Load(&asset);
    }
} // namespace

AlphaWeaponLoader::AlphaWeaponLoader(Zone& zone, ZoneInputStream& stream)
    : ContentLoaderBase(zone, stream)
{
}

void AlphaWeaponLoader::Load(WeaponCompleteDef** pAsset)
{
    m_stream.PushBlock(XFILE_BLOCK_TEMP);

    if (*pAsset != nullptr)
    {
        const auto zonePtrType = GetZonePointerType(*pAsset);
        if (zonePtrType == ZonePointerType::FOLLOWING || zonePtrType == ZonePointerType::INSERT)
        {
            static_cast<void>(m_stream.AllocOutOfBlock<uint32_t>(4));

            uintptr_t toInsertLookupEntry = 0;
            if (zonePtrType == ZonePointerType::INSERT)
                toInsertLookupEntry = m_stream.InsertPointerAliasLookup();

            LoadBody();

            if (zonePtrType == ZonePointerType::INSERT)
                m_stream.SetInsertedPointerAliasLookup(toInsertLookupEntry, nullptr);
        }

        *pAsset = nullptr;
    }

    m_stream.PopBlock();
}

uint32_t AlphaWeaponLoader::Raw(const size_t offset) const
{
    const auto at = m_shift + offset;
    if (at + sizeof(uint32_t) > m_size)
        return 0u;

    uint32_t value;
    m_fill->Fill(value, at);
    return value;
}

int16_t AlphaWeaponLoader::Short(const size_t offset) const
{
    const auto at = m_shift + offset;
    if (at + sizeof(int16_t) > m_size)
        return 0;

    int16_t value;
    m_fill->Fill(value, at);
    return value;
}

void AlphaWeaponLoader::String(const size_t offset)
{
    const auto raw = Raw(offset);
    if (raw == 0u)
        return;


    const char* str = nullptr;
    std::memcpy(&str, &raw, sizeof(raw));
    varXString = &str;
    LoadXString(false);
}

void AlphaWeaponLoader::Strings(const size_t offset, const size_t count)
{
    std::vector<const char*> strs(count, nullptr);
    for (auto i = 0uz; i < count; i++)
    {
        const auto raw = Raw(offset + i * sizeof(uint32_t));
        std::memcpy(&strs[i], &raw, sizeof(raw));
    }

    varXString = strs.data();
    LoadXStringArray(false, count);
}

void AlphaWeaponLoader::SoundArray(const size_t offset)
{
    if (Raw(offset) != FOLLOWING)
        return;

    auto* sounds = m_stream.Alloc<uint32_t>(4);
    m_stream.Load<uint32_t>(sounds, SOUND_ARRAY_COUNT);

    for (auto i = 0uz; i < SOUND_ARRAY_COUNT; i++)
        LoadSound(sounds[i]);
}

void AlphaWeaponLoader::Vec2Array(const size_t offset, const size_t countOffset)
{
    if (Raw(offset) != FOLLOWING)
        return;

    const auto count = Short(countOffset);
    auto* vec2 = m_stream.Alloc<float>(4);
    m_stream.Load<float>(vec2, 2u * static_cast<size_t>(count));
}

void AlphaWeaponLoader::LoadSound(const uint32_t raw)
{
    if (raw != FOLLOWING)
        return;

    auto* name = m_stream.Alloc<uint32_t>(4);
    m_stream.Load<uint32_t>(name);

    const char* str = nullptr;
    std::memcpy(&str, name, sizeof(*name));
    varXString = &str;
    LoadXString(false);
}

void AlphaWeaponLoader::Sound(const size_t offset)
{
    LoadSound(Raw(offset));
}

void AlphaWeaponLoader::Sounds(const size_t offset, const size_t count)
{
    for (auto i = 0uz; i < count; i++)
        Sound(offset + i * sizeof(uint32_t));
}

void AlphaWeaponLoader::Model(const size_t offset)
{
    LoadHandle<XModel, Loader_XModel>(m_zone, m_stream, Raw(offset));
}

void AlphaWeaponLoader::Effect(const size_t offset)
{
    LoadHandle<FxEffectDef, Loader_FxEffectDef>(m_zone, m_stream, Raw(offset));
}

void AlphaWeaponLoader::MaterialHandle(const size_t offset)
{
    LoadHandle<Material, Loader_Material>(m_zone, m_stream, Raw(offset));
}

void AlphaWeaponLoader::Collmap(const size_t offset)
{
    LoadHandle<PhysCollmap, Loader_PhysCollmap>(m_zone, m_stream, Raw(offset));
}

void AlphaWeaponLoader::Preset(const size_t offset)
{
    LoadHandle<PhysPreset, Loader_PhysPreset>(m_zone, m_stream, Raw(offset));
}

void AlphaWeaponLoader::Tracer(const size_t offset)
{
    LoadHandle<TracerDef, Loader_TracerDef>(m_zone, m_stream, Raw(offset));
}

void AlphaWeaponLoader::LoadBody()
{
    const auto version = m_zone.m_version;
    const auto flat = version >= VERSION_WEAPON_FLAT;
    const auto alpha3 = version >= VERSION_ALPHA3;

    m_size = WeaponSize(version);
    m_shift = 0u;

    const auto fill = m_stream.LoadWithFill(m_size);
    m_fill = &fill;
    m_stream.PushBlock(XFILE_BLOCK_VIRTUAL);

    String(0u);
    String(4u);
    String(8u);
    String(12u);
    Model(16u);

    if (flat)
    {
        for (auto offset = 20uz; offset <= 56u; offset += 4u)
            Model(offset);

        Strings(124u, 52u);
        Strings(124u + 52u * 4u, 52u);
        Strings(124u + 52u * 8u, 52u);

        Effect(908u);
        Effect(912u);

        Sounds(916u, 52u);

        SoundArray(1128u);
        SoundArray(1132u);

        for (auto offset = 1136uz; offset <= 1148u; offset += 4u)
            Effect(offset);

        for (const auto offset : {1152u, 1156u, 1372u, 1376u, 1380u, 1384u, 1388u, 1392u, 1400u, 1408u})
            MaterialHandle(offset);

        String(1428u);
        String(1436u);
        String(1452u);

        for (auto offset = 1716uz; offset <= 1728u; offset += 4u)
            MaterialHandle(offset);

        Collmap(1928u);
        Preset(1932u);

        Model(2020u);

        Effect(2028u);
        Effect(2032u);

        Sound(2036u);
        Sound(2040u);

        Effect(2304u);
        Effect(2308u);
        Effect(2336u);

        Sound(2340u);
        String(2516u);

        Vec2Array(2524u, 3044u);
        String(2520u);
        Vec2Array(2528u, 3046u);

        for (const auto offset : {2608u, 2612u, 2644u, 2648u, 2772u, 2776u})
            String(offset);

        Tracer(2780u);
        Sound(2808u);
        Effect(2812u);
        String(2816u);
        Sound(2832u);
        Sounds(2836u, 4u);
        Sounds(2852u, 4u);
        Sound(2868u);
        Sound(2872u);

        Sounds(2940u, 6u);

        String(2988u);
        String(3000u);
        String(3004u);
        MaterialHandle(3012u);
        MaterialHandle(3016u);
        MaterialHandle(3020u);

        Vec2Array(3048u, 3044u);
        Vec2Array(3052u, 3046u);
    }
    else
    {
        for (auto offset = 20uz; offset <= 144u; offset += 4u)
            Model(offset);
        for (auto offset = 148uz; offset <= 168u; offset += 4u)
            Model(offset);

        Strings(236u, 48u);
        Strings(428u, 48u);
        Strings(620u, 48u);

        Effect(972u);
        Effect(976u);

        Sounds(980u, 50u);
        if (alpha3)
        {
            Sounds(1184u, 2u);
            m_shift += 8u;
        }

        SoundArray(1184u);
        SoundArray(1188u);

        for (auto offset = 1192uz; offset <= 1204u; offset += 4u)
            Effect(offset);

        for (const auto offset : {1208u, 1212u, 1428u, 1432u, 1436u, 1440u, 1444u, 1448u, 1456u, 1464u})
            MaterialHandle(offset);

        String(1484u);
        String(1492u);
        String(1508u);

        for (auto offset = 1764uz; offset <= 1776u; offset += 4u)
            MaterialHandle(offset);

        Collmap(1964u);

        Model(2052u);

        Effect(2060u);
        Effect(2064u);

        Sound(2068u);
        Sound(2072u);

        Effect(2336u);
        Effect(2340u);
        Effect(2368u);

        Sound(2372u);
        String(2548u);

        Vec2Array(2556u, alpha3 ? 3076u : 3040u);
        String(2552u);
        Vec2Array(2560u, alpha3 ? 3078u : 3042u);

        for (const auto offset : {2640u, 2644u, 2676u, 2680u, 2804u, 2808u})
            String(offset);

        Tracer(2812u);
        Sound(2840u);
        Effect(2844u);
        String(2848u);
        Sound(2864u);
        Sounds(2868u, 4u);
        Sounds(2884u, 4u);
        Sound(2900u);
        Sound(2904u);

        if (alpha3)
        {
            Sounds(2972u, 6u);
            m_shift += 6u * 4u + 12u;
        }

        String(2984u);
        String(2996u);
        String(3000u);
        MaterialHandle(3008u);
        MaterialHandle(3012u);
        MaterialHandle(3016u);

        Vec2Array(3044u, 3040u);
        Vec2Array(3048u, 3042u);
    }

    m_stream.PopBlock();
    m_fill = nullptr;
}
