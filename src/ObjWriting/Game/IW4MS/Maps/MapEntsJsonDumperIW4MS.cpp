#include "MapEntsJsonDumperIW4MS.h"

#include "Dumping/AssetDumpingContext.h"
#include "MapTriggersJsonIW4MS.h"
#include "Maps/MapEntsCommon.h"

#include <iomanip>
#include <nlohmann/json.hpp>

using namespace nlohmann;
using namespace IW4MS;

namespace map_ents
{
    void JsonDumperIW4MS::DumpAsset(AssetDumpingContext& context, const XAssetInfo<AssetMapEnts::Type>& asset)
    {
        const auto assetFile = context.OpenAssetFile(GetTriggersFileNameForAssetName(asset.m_name));
        if (!assetFile)
            return;

        const auto* mapEnts = asset.Asset();

        json jRoot;
        jRoot["_type"] = "mapents";
        jRoot["_version"] = 1;
        jRoot["_game"] = "iw4";

        jRoot["trigger"] = CreateTriggersJsonIW4MS(mapEnts->trigger);

        jRoot["stages"] = world::Array(mapEnts->stages,
                                       static_cast<size_t>(mapEnts->stageCount),
                                       [](const Stage& stage)
                                       {
                                           return json{
                                               {"name",                 stage.name ? stage.name : ""                },
                                               {"origin",               world::Vec3(stage.origin)                   },
                                               {"triggerIndex",         stage.triggerIndex                          },
                                               {"sunPrimaryLightIndex", static_cast<int>(stage.sunPrimaryLightIndex)},
                                           };
                                       });

        *assetFile << std::setw(4) << jRoot << "\n";
    }
} // namespace map_ents
