#pragma once

#include "Game/IW4MS/IW4MS.h"
#include "Utils/MemoryManager.h"
#include "World/WorldJsonLoadCommon.h"

#include <nlohmann/json.hpp>

namespace map_ents
{
    inline void LoadTriggersJsonIW4MS(const nlohmann::json& jTriggers, IW4MS::MapTriggers& triggers, MemoryManager& memory)
    {
        const auto& jModels = jTriggers.at("models");
        triggers.count = static_cast<unsigned>(jModels.size());
        triggers.models = world::Array<IW4MS::TriggerModel>(memory,
                                                            jModels,
                                                            [](const nlohmann::json& jModel, IW4MS::TriggerModel& model)
                                                            {
                                                                jModel.at("contents").get_to(model.contents);
                                                                jModel.at("hullCount").get_to(model.hullCount);
                                                                jModel.at("firstHull").get_to(model.firstHull);
                                                            });

        const auto& jHulls = jTriggers.at("hulls");
        triggers.hullCount = static_cast<unsigned>(jHulls.size());
        triggers.hulls = world::Array<IW4MS::TriggerHull>(memory,
                                                          jHulls,
                                                          [](const nlohmann::json& jHull, IW4MS::TriggerHull& hull)
                                                          {
                                                              world::Bounds(jHull.at("bounds"), hull.bounds);
                                                              jHull.at("contents").get_to(hull.contents);
                                                              jHull.at("slabCount").get_to(hull.slabCount);
                                                              jHull.at("firstSlab").get_to(hull.firstSlab);
                                                          });

        const auto& jSlabs = jTriggers.at("slabs");
        triggers.slabCount = static_cast<unsigned>(jSlabs.size());
        triggers.slabs = world::Array<IW4MS::TriggerSlab>(memory,
                                                          jSlabs,
                                                          [](const nlohmann::json& jSlab, IW4MS::TriggerSlab& slab)
                                                          {
                                                              world::Vec(jSlab.at("dir"), slab.dir);
                                                              jSlab.at("midPoint").get_to(slab.midPoint);
                                                              jSlab.at("halfSize").get_to(slab.halfSize);
                                                          });
    }
} // namespace map_ents
