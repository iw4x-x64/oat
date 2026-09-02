#include "LoaderImpactFxIW4MS.h"

#include "FX/FxCommon.h"
#include "Game/IW4MS/MaterialConstantsIW4MS.h"
#include "Utils/Logging/Log.h"

#include <format>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using namespace nlohmann;
using namespace IW4MS;

namespace
{
    constexpr auto TABLE_ENTRY_COUNT = 15u;

    [[nodiscard]] std::string ImpactTypeKey(const unsigned index)
    {
        switch (index)
        {
        case IMPACT_TYPE_NONE:
            return "none";
        case IMPACT_TYPE_BULLET_SMALL:
            return "bullet_small";
        case IMPACT_TYPE_BULLET_LARGE:
            return "bullet_large";
        case IMPACT_TYPE_BULLET_AP:
            return "bullet_ap";
        case IMPACT_TYPE_BULLET_EXPLODE:
            return "bullet_explode";
        case IMPACT_TYPE_SHOTGUN:
            return "shotgun";
        case IMPACT_TYPE_SHOTGUN_EXPLODE:
            return "shotgun_explode";
        case IMPACT_TYPE_GRENADE_BOUNCE:
            return "grenade_bounce";
        case IMPACT_TYPE_GRENADE_EXPLODE:
            return "grenade_explode";
        case IMPACT_TYPE_ROCKET_EXPLODE:
            return "rocket_explode";
        case IMPACT_TYPE_PROJECTILE_DUD:
            return "projectile_dud";
        default:
            return std::format("unnamed_{}", index);
        }
    }

    class JsonLoader
    {
    public:
        JsonLoader(std::istream& stream, MemoryManager& memory, AssetCreationContext& context, AssetRegistration<AssetImpactFx>& registration)
            : m_stream(stream),
              m_memory(memory),
              m_context(context),
              m_registration(registration)
        {
        }

        bool Load(FxImpactTable& impactTable) const
        {
            try
            {
                const auto jRoot = json::parse(m_stream);
                std::string type;
                std::string game;
                unsigned version;

                jRoot.at("_type").get_to(type);
                jRoot.at("_version").get_to(version);
                jRoot.at("_game").get_to(game);

                if (type != "impactfx" || version != 1u || game != "iw4")
                {
                    con::error("Tried to load impactfx \"{}\" but did not find expected type impactfx of version 1 for game iw4", impactTable.name);
                    return false;
                }

                return LoadTable(jRoot.at("table"), impactTable);
            }
            catch (const json::exception& e)
            {
                con::error("Failed to parse json of impactfx \"{}\": {}", impactTable.name, e.what());
            }

            return false;
        }

    private:
        bool LoadTable(const json& jTable, FxImpactTable& impactTable) const
        {
            impactTable.table = m_memory.Alloc<FxImpactEntry>(TABLE_ENTRY_COUNT);

            auto success = true;
            for (auto i = 0u; i < TABLE_ENTRY_COUNT; i++)
            {
                const auto jEntry = jTable.find(ImpactTypeKey(i));
                if (jEntry == jTable.end())
                    continue;

                success = LoadEntry(*jEntry, impactTable.table[i]) && success;
            }

            return success;
        }

        bool LoadEntry(const json& jEntry, FxImpactEntry& entry) const
        {
            auto success = true;

            const auto jNonflesh = jEntry.find("nonflesh");
            if (jNonflesh != jEntry.end())
            {
                for (auto surfaceType = 0u; surfaceType < std::extent_v<decltype(entry.nonflesh)>; surfaceType++)
                {
                    const auto jEffect = jNonflesh->find(surfaceTypeNames[surfaceType]);
                    if (jEffect == jNonflesh->end())
                        continue;

                    success = LoadEffect(*jEffect, entry.nonflesh[surfaceType]) && success;
                }
            }

            const auto jFlesh = jEntry.find("flesh");
            if (jFlesh != jEntry.end())
            {
                const auto& jEffects = jFlesh->get_ref<const json::array_t&>();
                if (jEffects.size() != std::extent_v<decltype(entry.flesh)>)
                {
                    con::error("Impactfx entry has {} flesh effects instead of {}", jEffects.size(), std::extent_v<decltype(entry.flesh)>);
                    return false;
                }

                for (auto i = 0u; i < jEffects.size(); i++)
                    success = LoadEffect(jEffects[i], entry.flesh[i]) && success;
            }

            return success;
        }

        bool LoadEffect(const json& jEffect, FxEffectDef*& effect) const
        {
            if (jEffect.is_null())
            {
                effect = nullptr;
                return true;
            }

            const auto& name = jEffect.get_ref<const std::string&>();
            if (name.empty())
            {
                effect = nullptr;
                return true;
            }

            auto* asset = m_context.LoadDependency<AssetFx>(name);
            if (!asset)
            {
                con::error("Could not find fx \"{}\" of impactfx", name);
                return false;
            }

            m_registration.AddDependency(asset);
            effect = asset->Asset();

            return true;
        }

        std::istream& m_stream;
        MemoryManager& m_memory;
        AssetCreationContext& m_context;
        AssetRegistration<AssetImpactFx>& m_registration;
    };

    class ImpactFxLoader final : public AssetCreator<AssetImpactFx>
    {
    public:
        ImpactFxLoader(MemoryManager& memory, ISearchPath& searchPath)
            : m_memory(memory),
              m_search_path(searchPath)
        {
        }

        AssetCreationResult CreateAsset(const std::string& assetName, AssetCreationContext& context) override
        {
            const auto file = m_search_path.Open(fx::GetImpactFxJsonFileNameForAssetName(assetName));
            if (!file.IsOpen())
                return AssetCreationResult::NoAction();

            auto* impactTable = m_memory.Alloc<FxImpactTable>();
            impactTable->name = m_memory.Dup(assetName.c_str());

            AssetRegistration<AssetImpactFx> registration(assetName, impactTable);

            const JsonLoader loader(*file.m_stream, m_memory, context, registration);
            if (!loader.Load(*impactTable))
                return AssetCreationResult::Failure();

            return AssetCreationResult::Success(context.AddAsset(std::move(registration)));
        }

    private:
        MemoryManager& m_memory;
        ISearchPath& m_search_path;
    };
} // namespace

namespace fx
{
    std::unique_ptr<AssetCreator<AssetImpactFx>> CreateImpactFxLoaderIW4MS(MemoryManager& memory, ISearchPath& searchPath)
    {
        return std::make_unique<ImpactFxLoader>(memory, searchPath);
    }
} // namespace fx
