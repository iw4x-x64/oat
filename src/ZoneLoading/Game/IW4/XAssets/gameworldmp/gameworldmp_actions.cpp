#include "gameworldmp_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>

using namespace IW4;

namespace
{
    constexpr size_t GAMEWORLD_MP_SIZE = 8u;
    constexpr size_t GAMEWORLD_SP_ALPHA_SIZE = 84u;
    constexpr size_t GAMEWORLD_SP_ALPHA_PATH = 4u;
    constexpr size_t GAMEWORLD_SP_ALPHA_VEHICLE_TRACK = 72u;
    constexpr size_t GAMEWORLD_SP_ALPHA_GLASS_DATA = 80u;

    constexpr uint32_t FOLLOWING = 0xFFFFFFFFu;

    constexpr size_t PATH_NODE_ALPHA_SIZE = 148u;
    constexpr size_t PATH_NODE_TOTAL_LINK_COUNT = 56u;
    constexpr size_t PATH_NODE_LINKS = 60u;
    constexpr size_t PATH_LINK_ALPHA_SIZE = 16u;
    constexpr size_t PATH_BASE_NODE_SIZE = 16u;
    constexpr size_t PATH_NODE_TREE_SIZE = 16u;

    constexpr size_t PATH_DATA_TAILS[][2]{{40u, 44u}, {48u, 52u}, {60u, 64u}};

    uint32_t U32(const uint8_t* data, const size_t offset)
    {
        uint32_t value;
        std::memcpy(&value, data + offset, sizeof(value));
        return value;
    }

    uint16_t U16(const uint8_t* data, const size_t offset)
    {
        uint16_t value;
        std::memcpy(&value, data + offset, sizeof(value));
        return value;
    }
} // namespace

Actions_GameWorldMp::Actions_GameWorldMp(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_GameWorldMp::LoadGameWorldMpLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(GAMEWORLD_MP_SIZE * count);

    AlphaFill fill(stream, m_zone, GAMEWORLD_SP_ALPHA_SIZE, GAMEWORLD_MP_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        const auto* alpha = fill.Alpha(i);

        uint32_t segments;
        std::memcpy(&segments, alpha + GAMEWORLD_SP_ALPHA_VEHICLE_TRACK, sizeof(segments));
        if (segments != 0u)
            throw std::runtime_error("Pre-release game worlds with vehicle tracks are not supported");

        if (m_zone.m_version < ZoneConstants::ZONE_VERSION_ALPHA3)
        {
            if (count != 1u)
                throw std::runtime_error("Pre-release game worlds can only be loaded one at a time");

            std::memcpy(m_path.data(), alpha + GAMEWORLD_SP_ALPHA_PATH, m_path.size());
            m_stream = &stream;
        }

        fill.Move(i, 0u, 0u, 4u);
        fill.Move(i, 4u, GAMEWORLD_SP_ALPHA_GLASS_DATA, 4u);
    }

    return fill.Accessor();
}

uint8_t* Actions_GameWorldMp::LoadBytes(const unsigned align, const size_t size) const
{
    auto* data = m_stream->Alloc<uint8_t>(align);
    m_stream->Load<uint8_t>(data, size);
    return data;
}

void Actions_GameWorldMp::LoadPathNodeTrees(const size_t count) const
{
    const auto* trees = LoadBytes(4u, PATH_NODE_TREE_SIZE * count);
    for (auto i = 0uz; i < count; i++)
    {
        const auto* tree = trees + PATH_NODE_TREE_SIZE * i;
        if (static_cast<int32_t>(U32(tree, 0u)) >= 0)
        {
            for (const auto child : {8u, 12u})
            {
                if (U32(tree, child) == FOLLOWING)
                    LoadPathNodeTrees(1u);
            }
        }
        else if (U32(tree, 12u) != 0u)
            LoadBytes(2u, sizeof(uint16_t) * U32(tree, 8u));
    }
}

void Actions_GameWorldMp::LoadAlphaPathData(GameWorldMp*) const
{
    if (m_stream == nullptr)
        return;

    const auto* path = m_path.data();
    const auto nodeCount = U32(path, 0u);

    if (U32(path, 4u) != 0u)
    {
        const auto* nodes = LoadBytes(4u, PATH_NODE_ALPHA_SIZE * nodeCount);
        for (auto i = 0uz; i < nodeCount; i++)
        {
            const auto* node = nodes + PATH_NODE_ALPHA_SIZE * i;
            if (U32(node, PATH_NODE_LINKS) != 0u)
                LoadBytes(4u, PATH_LINK_ALPHA_SIZE * U16(node, PATH_NODE_TOTAL_LINK_COUNT));
        }
    }

    m_stream->PushBlock(XFILE_BLOCK_RUNTIME);
    if (U32(path, 8u) != 0u)
        LoadBytes(16u, PATH_BASE_NODE_SIZE * nodeCount);
    m_stream->PopBlock();

    if (U32(path, 16u) != 0u)
        LoadBytes(2u, sizeof(uint16_t) * nodeCount);
    if (U32(path, 20u) != 0u)
        LoadBytes(2u, sizeof(uint16_t) * nodeCount);
    if (U32(path, 28u) != 0u)
        LoadBytes(1u, U32(path, 24u));
    if (U32(path, 36u) != 0u)
        LoadPathNodeTrees(U32(path, 32u));

    for (const auto& [size, pointer] : PATH_DATA_TAILS)
    {
        if (U32(path, pointer) != 0u)
            LoadBytes(1u, U32(path, size));
    }

    m_stream = nullptr;
}
