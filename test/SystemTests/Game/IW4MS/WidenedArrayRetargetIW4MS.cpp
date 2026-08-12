#ifdef ARCH_x64

#include "Game/IW4/IW4.h"
#include "Game/IW4MS/IW4MS.h"
#include "Zone/Zone.h"
#include "ZoneRetargeter.h"

#include <catch2/catch_test_macros.hpp>

namespace
{
    // Both structures gained bytes at the tail on x64 while keeping every field they had. The
    // writer emits count * sizeof(target) bytes from the array, so an array carried over at the
    // source stride puts every element after the first at a shifted offset.
    //
    TEST_CASE("IW4MS: retargeting rebuilds dynamic entity clients at the x64 stride", "[iw4ms][clipmap]")
    {
        static_assert(sizeof(IW4::DynEntityClient) == 12u);
        static_assert(sizeof(IW4MS::DynEntityClient) == 16u);

        constexpr auto FIRST_COUNT = 3u;
        constexpr auto SECOND_COUNT = 2u;

        Zone source("clipmap", 0, GameId::IW4, GamePlatform::PC);

        auto* clipMap = source.Memory().Alloc<IW4::clipMap_t>();
        auto* first = source.Memory().Alloc<IW4::DynEntityClient>(FIRST_COUNT);
        auto* second = source.Memory().Alloc<IW4::DynEntityClient>(SECOND_COUNT);

        clipMap->name = "maps/mp/mp_test.d3dbsp";
        clipMap->dynEntCount[0] = FIRST_COUNT;
        clipMap->dynEntCount[1] = SECOND_COUNT;
        clipMap->dynEntClientList[0] = first;
        clipMap->dynEntClientList[1] = second;

        for (auto i = 0u; i < FIRST_COUNT; i++)
            first[i] = {.physObjId = static_cast<int>(100 + i),
                        .flags = static_cast<uint16_t>(200 + i),
                        .lightingHandle = static_cast<uint16_t>(300 + i),
                        .health = static_cast<int>(400 + i)};

        for (auto i = 0u; i < SECOND_COUNT; i++)
            second[i] = {.physObjId = static_cast<int>(500 + i),
                         .flags = static_cast<uint16_t>(600 + i),
                         .lightingHandle = static_cast<uint16_t>(700 + i),
                         .health = static_cast<int>(800 + i)};

        source.m_pools.AddAsset(IW4::ASSET_TYPE_CLIPMAP_MP, clipMap->name, clipMap, {}, {}, {});

        const auto retargeted = retarget::Retarget(source, GameId::IW4MS);
        REQUIRE(retargeted != nullptr);

        const auto* asset = retargeted->m_pools.GetAsset(IW4MS::ASSET_TYPE_CLIPMAP_MP, clipMap->name);
        REQUIRE(asset != nullptr);

        const auto* rebuilt = static_cast<const IW4MS::clipMap_t*>(asset->m_ptr);

        REQUIRE(rebuilt->dynEntCount[0] == FIRST_COUNT);
        REQUIRE(rebuilt->dynEntCount[1] == SECOND_COUNT);
        REQUIRE(rebuilt->dynEntClientList[0] != nullptr);
        REQUIRE(rebuilt->dynEntClientList[1] != nullptr);

        for (auto i = 0u; i < FIRST_COUNT; i++)
        {
            const auto& client = rebuilt->dynEntClientList[0][i];

            REQUIRE(client.physObjId == static_cast<int>(100 + i));
            REQUIRE(client.flags == 200 + i);
            REQUIRE(client.lightingHandle == 300 + i);
            REQUIRE(client.health == static_cast<int>(400 + i));
        }

        for (auto i = 0u; i < SECOND_COUNT; i++)
        {
            const auto& client = rebuilt->dynEntClientList[1][i];

            REQUIRE(client.physObjId == static_cast<int>(500 + i));
            REQUIRE(client.health == static_cast<int>(800 + i));
        }
    }

    TEST_CASE("IW4MS: retargeting rebuilds glass pieces at the x64 stride", "[iw4ms][fxworld]")
    {
        static_assert(sizeof(IW4::FxGlassPieceDynamics) == 36u);
        static_assert(sizeof(IW4MS::FxGlassPieceDynamics) == 48u);

        constexpr auto PIECE_COUNT = 4u;

        Zone source("fxworld", 0, GameId::IW4, GamePlatform::PC);

        auto* world = source.Memory().Alloc<IW4::FxWorld>();
        auto* pieces = source.Memory().Alloc<IW4::FxGlassPieceDynamics>(PIECE_COUNT);

        world->name = "maps/mp/mp_test.d3dbsp";
        world->glassSys.pieceLimit = PIECE_COUNT;
        world->glassSys.pieceDynamics = pieces;

        for (auto i = 0u; i < PIECE_COUNT; i++)
        {
            pieces[i].fallTime = static_cast<int>(10 + i);
            pieces[i].physObjId = static_cast<int>(20 + i);
            pieces[i].physJointId = static_cast<int>(30 + i);
            pieces[i].vel[0] = static_cast<float>(i) + 0.5f;
            pieces[i].vel[1] = static_cast<float>(i) + 1.5f;
            pieces[i].vel[2] = static_cast<float>(i) + 2.5f;
            pieces[i].avel[0] = static_cast<float>(i) + 3.5f;
            pieces[i].avel[1] = static_cast<float>(i) + 4.5f;
            pieces[i].avel[2] = static_cast<float>(i) + 5.5f;
        }

        source.m_pools.AddAsset(IW4::ASSET_TYPE_FXWORLD, world->name, world, {}, {}, {});

        const auto retargeted = retarget::Retarget(source, GameId::IW4MS);
        REQUIRE(retargeted != nullptr);

        const auto* asset = retargeted->m_pools.GetAsset(IW4MS::ASSET_TYPE_FXWORLD, world->name);
        REQUIRE(asset != nullptr);

        const auto& glass = static_cast<const IW4MS::FxWorld*>(asset->m_ptr)->glassSys;

        REQUIRE(glass.pieceLimit == PIECE_COUNT);
        REQUIRE(glass.pieceDynamics != nullptr);

        for (auto i = 0u; i < PIECE_COUNT; i++)
        {
            const auto& piece = glass.pieceDynamics[i];

            REQUIRE(piece.fallTime == static_cast<int>(10 + i));
            REQUIRE(piece.physObjId == static_cast<int>(20 + i));
            REQUIRE(piece.physJointId == static_cast<int>(30 + i));
            REQUIRE(piece.vel[0] == static_cast<float>(i) + 0.5f);
            REQUIRE(piece.vel[2] == static_cast<float>(i) + 2.5f);
            REQUIRE(piece.avel[0] == static_cast<float>(i) + 3.5f);
            REQUIRE(piece.avel[2] == static_cast<float>(i) + 5.5f);

            for (auto b = 0u; b < sizeof(piece.unknown_36); b++)
                REQUIRE(piece.unknown_36[b] == 0);
        }
    }

    // Every structure the two builds disagree about has to be one the retargeter rewrites. This
    // holds the list at the six that are known, so a header change that widens a seventh fails
    // here instead of in a converted zone.
    //
    TEST_CASE("IW4MS: the structures that differ from IW4 are the ones that are retargeted", "[iw4ms][retail]")
    {
        REQUIRE(sizeof(IW4::clipMap_t) != sizeof(IW4MS::clipMap_t));
        REQUIRE(sizeof(IW4::water_t) != sizeof(IW4MS::water_t));
        REQUIRE(sizeof(IW4::SpeakerMap) != sizeof(IW4MS::SpeakerMap));
        REQUIRE(sizeof(IW4::MSSChannelMap) != sizeof(IW4MS::MSSChannelMap));
        REQUIRE(sizeof(IW4::DynEntityClient) != sizeof(IW4MS::DynEntityClient));
        REQUIRE(sizeof(IW4::FxGlassPieceDynamics) != sizeof(IW4MS::FxGlassPieceDynamics));

        // MssSound keeps its size and changes its meaning, so it is checked by field in
        // LoadedSoundRetargetIW4MS.
        //
        REQUIRE(sizeof(IW4::MssSound) == sizeof(IW4MS::MssSound));

        // The containers the retargeter reaches through, which it can only do while they agree.
        //
        REQUIRE(sizeof(IW4::FxGlassSystem) == sizeof(IW4MS::FxGlassSystem));
        REQUIRE(sizeof(IW4::FxWorld) == sizeof(IW4MS::FxWorld));
        REQUIRE(sizeof(IW4::snd_alias_t) == sizeof(IW4MS::snd_alias_t));
        REQUIRE(sizeof(IW4::snd_alias_list_t) == sizeof(IW4MS::snd_alias_list_t));
        REQUIRE(sizeof(IW4::LoadedSound) == sizeof(IW4MS::LoadedSound));
        REQUIRE(sizeof(IW4::Material) == sizeof(IW4MS::Material));
        REQUIRE(sizeof(IW4::GfxWorld) == sizeof(IW4MS::GfxWorld));
    }
} // namespace

#endif // ARCH_x64
