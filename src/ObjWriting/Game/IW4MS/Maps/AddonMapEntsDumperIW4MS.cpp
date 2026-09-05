#include "AddonMapEntsDumperIW4MS.h"

#include "Dumping/AssetDumpingContext.h"
#include "Game/IW4MS/Maps/MapTriggersJsonIW4MS.h"
#include "Maps/MapEntsCommon.h"

#include <algorithm>
#include <iomanip>
#include <nlohmann/json.hpp>

using namespace nlohmann;

using namespace IW4MS;

namespace addon_map_ents
{
    void DumperIW4MS::DumpAsset(AssetDumpingContext& context, const XAssetInfo<AssetAddonMapEnts::Type>& asset)
    {
        const auto* addonMapEnts = asset.Asset();
        const auto assetFile = context.OpenAssetFile(GetEntsFileNameForAssetName(asset.m_name));

        if (!assetFile)
            return;

        auto& stream = *assetFile;

        stream.write(addonMapEnts->entityString, std::max(addonMapEnts->numEntityChars - 1, 0));

        const auto triggersFile = context.OpenAssetFile(GetTriggersFileNameForAssetName(asset.m_name));
        if (!triggersFile)
            return;

        json jRoot;
        jRoot["_type"] = "addonmapents";
        jRoot["_version"] = 1;
        jRoot["_game"] = "iw4";
        jRoot["trigger"] = map_ents::CreateTriggersJsonIW4MS(addonMapEnts->trigger);

        *triggersFile << std::setw(4) << jRoot << "\n";
    }
} // namespace addon_map_ents
