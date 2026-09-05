#include "ObjLoaderIW4.h"

#include "Asset/GlobalAssetPoolsLoader.h"
#include "Game/IW4/AssetMarkerIW4.h"
#include "Game/IW4/FX/LoaderEfxIW4.h"
#include "Game/IW4/FX/LoaderImpactFxIW4.h"
#include "Game/IW4/Font/FontLoaderIW4.h"
#include "Game/IW4/GameIW4.h"
#include "Game/IW4/IW4.h"
#include "Game/IW4/Image/ImageLoaderEmbeddedIW4.h"
#include "Game/IW4/Image/ImageLoaderExternalIW4.h"
#include "Game/IW4/Maps/LoaderMapEntsIW4.h"
#include "Game/IW4/PhysCollmap/LoaderPhysCollmapIW4.h"
#include "Game/IW4/Sound/LoaderLoadedSoundIW4.h"
#include "Game/IW4/Sound/LoaderSoundAliasIW4.h"
#include "Game/IW4/Techset/PixelShaderLoaderIW4.h"
#include "Game/IW4/Techset/VertexShaderLoaderIW4.h"
#include "Game/IW4/Tracer/GdtLoaderTracerIW4.h"
#include "Game/IW4/Tracer/RawLoaderTracerIW4.h"
#include "Game/IW4/Vehicle/GdtLoaderVehicleIW4.h"
#include "Game/IW4/Vehicle/RawLoaderVehicleIW4.h"
#include "Game/IW4/Weapon/AccuracyGraphLoaderIW4.h"
#include "Game/IW4/World/LoaderClipMapIW4.h"
#include "Game/IW4/World/LoaderComWorldIW4.h"
#include "Game/IW4/World/LoaderFxWorldIW4.h"
#include "Game/IW4/World/LoaderGameWorldIW4.h"
#include "Game/IW4/World/LoaderGfxWorldIW4.h"
#include "Game/IW4/XAnim/XAnimLoaderIW4.h"
#include "Game/IW4/XModel/LoaderXModelIW4.h"
#include "Leaderboard/LoaderLeaderboardIW4.h"
#include "LightDef/LightDefLoaderIW4.h"
#include "Localize/LoaderLocalizeIW4.h"
#include "Material/LoaderMaterialIW4.h"
#include "Menu/LoaderMenuListIW4.h"
#include "ObjLoading.h"
#include "PhysPreset/GdtLoaderPhysPresetIW4.h"
#include "PhysPreset/RawLoaderPhysPresetIW4.h"
#include "RawFile/LoaderRawFileIW4.h"
#include "Sound/LoaderSoundCurveIW4.h"
#include "StringTable/LoaderStringTableIW4.h"
#include "StructuredDataDef/LoaderStructuredDataDefIW4.h"
#include "Weapon/GdtLoaderWeaponIW4.h"
#include "Weapon/RawLoaderWeaponIW4.h"

#include <memory>

using namespace IW4;

void ObjLoader::LoadReferencedContainersForZone(ISearchPath& searchPath, Zone& zone) const {}

void ObjLoader::UnloadContainersOfZone(Zone& zone) const {}

namespace
{
    void ConfigureDefaultCreators(AssetCreatorCollection& collection, Zone& zone)
    {
        auto& memory = zone.Memory();

        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetPhysPreset>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetPhysCollMap>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetXAnim>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetXModelSurfs>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetXModel>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetMaterial>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetPixelShader>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetVertexShader>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetVertexDecl>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetTechniqueSet>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetImage>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetSound>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetSoundCurve>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetLoadedSound>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetClipMapSp>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetClipMapMp>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetComWorld>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetGameWorldSp>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetGameWorldMp>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetMapEnts>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetFxWorld>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetGfxWorld>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetLightDef>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetFont>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetMenuList>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetMenu>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetLocalize>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetWeapon>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetFx>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetImpactFx>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetRawFile>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetStringTable>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetLeaderboard>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetStructuredDataDef>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetTracer>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetVehicle>>(memory));
        collection.AddDefaultAssetCreator(std::make_unique<DefaultAssetCreator<AssetAddonMapEnts>>(memory));
    }

    void ConfigureGlobalAssetPoolsLoaders(AssetCreatorCollection& collection, Zone& zone)
    {
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetPhysPreset>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetPhysCollMap>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetXAnim>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetXModel>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetMaterial>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetPixelShader>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetVertexShader>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetVertexDecl>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetTechniqueSet>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetImage>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetSound>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetSoundCurve>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetLoadedSound>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetClipMapMp>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetComWorld>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetGameWorldSp>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetGameWorldMp>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetMapEnts>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetFxWorld>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetGfxWorld>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetLightDef>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetFont>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetMenuList>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetMenu>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetLocalize>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetWeapon>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetFx>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetImpactFx>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetRawFile>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetStringTable>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetLeaderboard>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetStructuredDataDef>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetTracer>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetVehicle>>(zone));
        collection.AddAssetCreator(std::make_unique<GlobalAssetPoolsLoader<AssetAddonMapEnts>>(zone));
    }

    void ConfigureLoaders(AssetCreatorCollection& collection, Zone& zone, ISearchPath& searchPath, IGdtQueryable& gdt)
    {
        auto& memory = zone.Memory();

        collection.AddAssetCreator(phys_preset::CreateRawLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(phys_preset::CreateGdtLoaderIW4(memory, gdt, zone));
        collection.AddAssetCreator(phys_collmap::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(xanim::CreateLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(xmodel::CreateLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(material::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(techset::CreateVertexShaderLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(techset::CreatePixelShaderLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(image::CreateLoaderEmbeddedIW4(memory, searchPath));
        collection.AddAssetCreator(image::CreateLoaderExternalIW4(memory, searchPath));
        collection.AddAssetCreator(sound::CreateSoundAliasLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(sound_curve::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(sound::CreateLoadedSoundLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(world::CreateClipMapSpLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(world::CreateClipMapMpLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(world::CreateComWorldLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(world::CreateGameWorldSpLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(world::CreateGameWorldMpLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(map_ents::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(world::CreateFxWorldLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(world::CreateGfxWorldLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(light_def::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(font::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(menu::CreateMenuListLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(localize::CreateLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(weapon::CreateRawLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(weapon::CreateGdtLoaderIW4(memory, searchPath, gdt, zone));
        collection.AddAssetCreator(fx::CreateEfxLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(fx::CreateImpactFxLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(raw_file::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(string_table::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(leaderboard::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(structured_data_def::CreateLoaderIW4(memory, searchPath));
        collection.AddAssetCreator(tracer::CreateRawLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(tracer::CreateGdtLoaderIW4(memory, gdt, zone));
        collection.AddAssetCreator(vehicle::CreateRawLoaderIW4(memory, searchPath, zone));
        collection.AddAssetCreator(vehicle::CreateGdtLoaderIW4(memory, gdt, zone));
        collection.AddAssetCreator(addon_map_ents::CreateLoaderIW4(memory, searchPath));

        collection.AddSubAssetCreator(weapon::CreateAccuracyGraphLoaderIW4(memory, searchPath));
    }
} // namespace

void ObjLoader::ConfigureCreatorCollection(AssetCreatorCollection& collection, Zone& zone, ISearchPath& searchPath, IGdtQueryable& gdt) const
{
    ConfigureDefaultCreators(collection, zone);
    ConfigureLoaders(collection, zone, searchPath, gdt);
    ConfigureGlobalAssetPoolsLoaders(collection, zone);
}
