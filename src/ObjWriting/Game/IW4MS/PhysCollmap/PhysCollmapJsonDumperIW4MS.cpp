#include "PhysCollmapJsonDumperIW4MS.h"

#include "Dumping/AssetDumpingContext.h"
#include "PhysCollmap/PhysCollmapCommon.h"
#include "World/WorldJsonCommon.h"

#include <iomanip>
#include <nlohmann/json.hpp>

using namespace nlohmann;
using namespace IW4MS;

namespace
{
    [[nodiscard]] json CreatePlane(const cplane_s& plane)
    {
        return json{
            {"normal", world::Vec3(plane.normal)   },
            {"dist",   plane.dist                  },
            {"type",   static_cast<int>(plane.type)},
        };
    }

    [[nodiscard]] json CreateBrush(const BrushWrapper& brushWrapper)
    {
        const auto& brush = brushWrapper.brush;

        const auto planeIndex = [&brushWrapper](const cplane_s* plane) -> int
        {
            if (!plane || !brushWrapper.planes)
                return -1;

            return static_cast<int>(plane - brushWrapper.planes);
        };

        const auto sideBytes = [](const unsigned char (&values)[2][3])
        {
            auto jSides = json::array();
            for (const auto& side : values)
                jSides.emplace_back(json::array({static_cast<int>(side[0]), static_cast<int>(side[1]), static_cast<int>(side[2])}));

            return jSides;
        };

        return json{
            {"bounds", world::Bounds(brushWrapper.bounds)},
            {"totalEdgeCount", brushWrapper.totalEdgeCount},
            {"planes", world::Array(brushWrapper.planes, brush.numsides, CreatePlane)},
            {"numsides", brush.numsides},
            {"glassPieceIndex", brush.glassPieceIndex},
            {"sides",
             world::Array(brush.sides,
             brush.numsides,
             [&planeIndex](const cbrushside_t& side)
                          {
                              return json{
                                  {"plane", planeIndex(side.plane)},
                                  {"materialNum", side.materialNum},
                                  {"firstAdjacentSideOffset", static_cast<int>(side.firstAdjacentSideOffset)},
                                  {"edgeCount", static_cast<int>(side.edgeCount)},
                              };
                          })},
            {"baseAdjacentSide", world::Base64(brush.baseAdjacentSide, static_cast<size_t>(brushWrapper.totalEdgeCount))},
            {"axialMaterialNum",
             json::array({json::array({brush.axialMaterialNum[0][0], brush.axialMaterialNum[0][1], brush.axialMaterialNum[0][2]}),
                          json::array({brush.axialMaterialNum[1][0], brush.axialMaterialNum[1][1], brush.axialMaterialNum[1][2]})})},
            {"firstAdjacentSideOffsets", sideBytes(brush.firstAdjacentSideOffsets)},
            {"edgeCount", sideBytes(brush.edgeCount)},
        };
    }
} // namespace

namespace phys_collmap
{
    void JsonDumperIW4MS::DumpAsset(AssetDumpingContext& context, const XAssetInfo<AssetPhysCollMap::Type>& asset)
    {
        const auto assetFile = context.OpenAssetFile(GetJsonFileNameForAssetName(asset.m_name));
        if (!assetFile)
            return;

        const auto* physCollmap = asset.Asset();

        json jRoot;
        jRoot["_type"] = "physcollmap";
        jRoot["_version"] = 1;
        jRoot["_game"] = "iw4";

        jRoot["mass"] = {
            {"centerOfMass",      world::Vec3(physCollmap->mass.centerOfMass)     },
            {"momentsOfInertia",  world::Vec3(physCollmap->mass.momentsOfInertia) },
            {"productsOfInertia", world::Vec3(physCollmap->mass.productsOfInertia)},
        };
        jRoot["bounds"] = world::Bounds(physCollmap->bounds);

        jRoot["geoms"] = world::Array(
            physCollmap->geoms,
            physCollmap->count,
            [](const PhysGeomInfo& geom)
            {
                json jGeom{
                    {"type",        geom.type                                                                                                          },
                    {"orientation", json::array({world::Vec3(geom.orientation[0]), world::Vec3(geom.orientation[1]), world::Vec3(geom.orientation[2])})},
                    {"bounds",      world::Bounds(geom.bounds)                                                                                         },
                };

                if (geom.brushWrapper)
                    jGeom["brushWrapper"] = CreateBrush(*geom.brushWrapper);

                return jGeom;
            });

        *assetFile << std::setw(4) << jRoot << "\n";
    }
} // namespace phys_collmap
