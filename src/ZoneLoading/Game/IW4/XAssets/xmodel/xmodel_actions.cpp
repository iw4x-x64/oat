#include "xmodel_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>
#include <stdexcept>

using namespace IW4;

namespace
{
    constexpr size_t XMODEL_SIZE = 304u;
    constexpr size_t XMODEL_ALPHA2_SIZE = 364u;
    constexpr size_t XMODEL_ALPHA_SIZE = 360u;
    constexpr size_t XMODEL_LOD_INFO = 64u;
    constexpr size_t XMODEL_ALPHA_LOD_INFO = 72u;
    constexpr size_t LOD_INFO_SIZE = 44u;
    constexpr size_t LOD_INFO_ALPHA_SIZE = 56u;
    constexpr size_t LOD_INFO_ALPHA_NAME = 12u;

    constexpr size_t XMODEL_SURFS_SIZE = 36u;
    constexpr size_t XMODEL_SURFS_ALPHA_SIZE = 48u;

    constexpr size_t XSURFACE_SIZE = 64u;
    constexpr size_t XSURFACE_ALPHA_SIZE = 84u;
    constexpr uint8_t XSURFACE_ALPHA_OWN_BUFFERS = 0x20u;

    constexpr uint32_t FOLLOWING = 0xFFFFFFFFu;
} // namespace

Actions_XModel::Actions_XModel(Zone& zone)
    : AssetLoadingActions(zone)
{
}

void Actions_XModel::SetModelSurfs(XModelLodInfo* lodInfo, XModelSurfs* modelSurfs) const
{
    if (modelSurfs)
    {
        lodInfo->modelSurfs = m_zone.Memory().Alloc<XModelSurfs>();
        memcpy(lodInfo->modelSurfs, modelSurfs, sizeof(XModelSurfs));
    }
}

ZoneStreamFillReadAccessor Actions_XModel::LoadXModelLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(XMODEL_SIZE * count);

    if (count != 1u)
        throw std::runtime_error("Pre-release xmodels can only be loaded one at a time");

    const auto alphaSize = m_zone.m_version == ZoneConstants::ZONE_VERSION_ALPHA2 ? XMODEL_ALPHA2_SIZE : XMODEL_ALPHA_SIZE;
    AlphaFill fill(stream, m_zone, alphaSize, XMODEL_SIZE, count);

    fill.Move(0u, 0u, 0u, 36u);
    fill.Move(0u, 36u, 44u, 28u);

    for (auto i = 0uz; i < std::extent_v<decltype(m_lod_names)>; i++)
    {
        const auto gameLod = XMODEL_LOD_INFO + i * LOD_INFO_SIZE;
        const auto alphaLod = XMODEL_ALPHA_LOD_INFO + i * LOD_INFO_ALPHA_SIZE;

        fill.Move(0u, gameLod, alphaLod, 12u);
        fill.Move(0u, gameLod + 12u, alphaLod + 16u, 32u);
        std::memcpy(&m_lod_names[i], fill.Alpha(0u) + alphaLod + LOD_INFO_ALPHA_NAME, sizeof(uint32_t));
    }

    fill.Move(0u, 236u, 292u, alphaSize - 296u);
    fill.Move(0u, XMODEL_SIZE - 8u, alphaSize - 8u, 8u);

    m_stream = &stream;
    m_lod_index = 0u;

    return fill.Accessor();
}

void Actions_XModel::LoadXModelLodInfoName(XModelLodInfo* lodInfo) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version) || m_stream == nullptr || m_lod_index >= std::extent_v<decltype(m_lod_names)>)
        return;

    if (m_lod_names[m_lod_index++] == FOLLOWING)
    {
        auto* name = m_stream->Alloc<char>(1);
        m_stream->LoadNullTerminated(name);
    }
}

ZoneStreamFillReadAccessor Actions_XModel::LoadXModelSurfsLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(XMODEL_SURFS_SIZE * count);

    return LoadAlphaPrefix(stream, m_zone, XMODEL_SURFS_ALPHA_SIZE, XMODEL_SURFS_SIZE, count);
}

ZoneStreamFillReadAccessor Actions_XModel::LoadXSurfaceLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(XSURFACE_SIZE * count);

    AlphaFill fill(stream, m_zone, XSURFACE_ALPHA_SIZE, XSURFACE_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        fill.Move(i, 0u, 0u, 12u);
        fill.Move(i, 12u, 16u, 20u);
        fill.Move(i, 32u, 40u, 8u);
        fill.Move(i, 40u, 52u, 24u);

        if (m_zone.m_version >= ZoneConstants::ZONE_VERSION_ALPHA_IMAGE)
        {
            const auto* alpha = fill.Alpha(i);
            if (!(alpha[2] & XSURFACE_ALPHA_OWN_BUFFERS))
                throw std::runtime_error("Pre-release xsurfaces without their own buffers are not supported");

            fill.Move(i, 6u, 3u, 1u);
            fill.Move(i, 2u, 4u, 4u);
        }
    }

    return fill.Accessor();
}
