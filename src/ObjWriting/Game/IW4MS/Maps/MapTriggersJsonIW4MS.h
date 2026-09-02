#pragma once

#include "Game/IW4MS/IW4MS.h"
#include "World/WorldJsonCommon.h"

#include <nlohmann/json.hpp>

namespace map_ents
{
    [[nodiscard]] inline nlohmann::json CreateTriggersJsonIW4MS(const IW4MS::MapTriggers& triggers)
    {
        return nlohmann::json{
            {"models",
             world::Array(triggers.models,
             triggers.count,
             [](const IW4MS::TriggerModel& model)
                          {
                              return nlohmann::json{
                                  {                                "contents",                model.contents},
                                  { "hullCount", model.hullCount},
                                  {"firstHull", model.firstHull},
                              };
                          })},
            {"hulls",
             world::Array(triggers.hulls,
             triggers.hullCount,
             [](const IW4MS::TriggerHull& hull)
                          {
                              return nlohmann::json{
                                  {"bounds", world::Bounds(hull.bounds)},
                                  {"contents", hull.contents},
                                  {"slabCount", hull.slabCount},
                                  {"firstSlab", hull.firstSlab},
                              };
                          })},
            {"slabs",
             world::Array(triggers.slabs,
             triggers.slabCount,
             [](const IW4MS::TriggerSlab& slab)
                          {
                              return nlohmann::json{
                                  {"dir", world::Vec3(slab.dir)},
                                  {"midPoint", slab.midPoint},
                                  {"halfSize", slab.halfSize},
                              };
                          })},
        };
    }
} // namespace map_ents
