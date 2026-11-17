#include "Game/IW4/IW4.h"
#include "Gdt/GdtLookup.h"
#include "IObjLoader.h"
#include "SearchPath/MockSearchPath.h"
#include "XModel/Gltf/GltfBinOutput.h"
#include "XModel/Gltf/GltfWriter.h"
#include "XModel/XModelCommon.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    XModelBone MakeBone(std::string name, std::optional<unsigned> parentIndex)
    {
        XModelBone bone{};
        bone.name = std::move(name);
        bone.parentIndex = parentIndex;
        bone.scale[0] = bone.scale[1] = bone.scale[2] = 1.0f;
        bone.globalRotation = {0.0f, 0.0f, 0.0f, 1.0f};
        bone.localRotation = {0.0f, 0.0f, 0.0f, 1.0f};
        return bone;
    }

    // A triangle skinned to j_mainroot, with a j_head child and a tag_eye
    // grandchild.
    std::string GivenSkinnedModel()
    {
        XModelCommon common;
        common.m_name = "partclass";
        common.m_vertices = {
            {.coordinates = {0, 0, 0}, .normal = {0, 0, 1}, .color = {1, 1, 1, 1}, .uv = {0, 0}},
            {.coordinates = {1, 0, 0}, .normal = {0, 0, 1}, .color = {1, 1, 1, 1}, .uv = {1, 0}},
            {.coordinates = {0, 1, 0}, .normal = {0, 0, 1}, .color = {1, 1, 1, 1}, .uv = {0, 1}},
        };
        common.m_bones = {
            MakeBone("j_mainroot", std::nullopt),
            MakeBone("j_head", 0u),
            MakeBone("tag_eye", 1u),
        };
        common.m_bone_weight_data.weights = {
            {.boneIndex = 0, .weight = 1.0f}
        };
        common.m_vertex_bone_weights = {
            {.weightOffset = 0, .weightCount = 1},
            {.weightOffset = 0, .weightCount = 1},
            {.weightOffset = 0, .weightCount = 1},
        };

        XModelMaterial material{};
        material.ApplyDefaults();
        material.name = "material";
        common.m_materials.push_back(material);
        common.m_objects.push_back({.name = "partclass", .materialIndex = 0, .m_faces = {XModelFace{{0, 1, 2}}}});

        std::ostringstream modelData(std::ios::binary);
        const gltf::BinOutput modelOutput(modelData);
        gltf::Writer::CreateWriter(&modelOutput, "iw4", "test")->Write(common);
        return modelData.str();
    }

    struct LinkResult
    {
        bool success;
        std::vector<std::pair<std::string, unsigned>> bones;
    };

    LinkResult Link(nlohmann::json metadata, const std::string& csv)
    {
        metadata["_game"] = "iw4";
        metadata["_type"] = "xmodel";
        metadata["_version"] = 2;
        metadata["flags"] = 0;
        metadata["type"] = "animated";
        metadata["lods"] = {
            {{"file", "partclass.glb"}, {"distance", 100}}
        };

        MockSearchPath searchPath;
        searchPath.AddFileData("partclass.glb", GivenSkinnedModel());
        searchPath.AddFileData("xmodel/partclass.json", metadata.dump());
        searchPath.AddFileData("partclassification.csv", csv);

        Zone zone("partclass", 0, GameId::IW4, GamePlatform::PC);
        AssetCreatorCollection creators(zone);
        GdtLookup gdt;
        IObjLoader::GetObjLoaderForGame(GameId::IW4)->ConfigureCreatorCollection(creators, zone, searchPath, gdt);
        IgnoredAssetLookup ignored;
        ignored.m_ignored_asset_lookup.emplace("material", IW4::ASSET_TYPE_MATERIAL);
        AssetCreationContext context(zone, &creators, &ignored);

        LinkResult result{};
        result.success = creators.CreateAsset(IW4::ASSET_TYPE_XMODEL, "partclass", context).HasBeenSuccessful();
        if (!result.success)
            return result;

        const auto* asset = zone.m_pools.GetAsset<IW4::AssetXModel>("partclass");
        REQUIRE(asset);
        const auto& model = *asset->Asset();
        REQUIRE(model.partClassification);
        for (auto i = 0u; i < model.numBones; i++)
            result.bones.emplace_back(zone.m_script_strings[model.boneNames[i]], model.partClassification[i]);

        return result;
    }

    unsigned HitLocOf(const LinkResult& result, const std::string& bone)
    {
        for (const auto& [name, hitLoc] : result.bones)
        {
            if (name == bone)
                return hitLoc;
        }

        FAIL("bone not found: " << bone);
        return 0;
    }

    constexpr auto CSV = "j_mainroot,torso_lower\nj_head,head\ntag_eye,head\n";

    TEST_CASE("IW4 xmodel without partClassification uses partclassification.csv", "[iw4][system][xmodel][partclassification]")
    {
        const auto result = Link(nlohmann::json::object(), CSV);
        REQUIRE(result.success);
        CHECK(HitLocOf(result, "j_mainroot") == IW4::HITLOC_TORSO_LWR);
        CHECK(HitLocOf(result, "j_head") == IW4::HITLOC_HEAD);
        CHECK(HitLocOf(result, "tag_eye") == IW4::HITLOC_HEAD);
    }

    TEST_CASE("IW4 xmodel partClassification is applied exactly over the csv", "[iw4][system][xmodel][partclassification]")
    {
        const nlohmann::json metadata{
            {"partClassification", {{"j_mainroot", "gun"}, {"j_head", "helmet"}}},
        };

        const auto result = Link(metadata, CSV);
        REQUIRE(result.success);
        CHECK(HitLocOf(result, "j_mainroot") == IW4::HITLOC_GUN);
        CHECK(HitLocOf(result, "j_head") == IW4::HITLOC_HELMET);
        // Listed in the csv but not in the explicit table.
        CHECK(HitLocOf(result, "tag_eye") == IW4::HITLOC_NONE);
    }

    TEST_CASE("IW4 xmodel with empty partClassification is unclassified", "[iw4][system][xmodel][partclassification]")
    {
        const nlohmann::json metadata{
            {"partClassification", nlohmann::json::object()},
        };

        const auto result = Link(metadata, CSV);
        REQUIRE(result.success);
        for (const auto& [name, hitLoc] : result.bones)
        {
            CAPTURE(name);
            CHECK(hitLoc == IW4::HITLOC_NONE);
        }
    }

    TEST_CASE("IW4 xmodel partClassification rejects unknown hit locations", "[iw4][system][xmodel][partclassification]")
    {
        const nlohmann::json metadata{
            {"partClassification", {{"j_head", "brain"}}},
        };

        CHECK_FALSE(Link(metadata, CSV).success);
    }
} // namespace
