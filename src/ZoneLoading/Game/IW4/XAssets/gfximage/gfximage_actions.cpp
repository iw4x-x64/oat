#include "gfximage_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cassert>
#include <cstring>

using namespace IW4;

namespace
{
    constexpr size_t IMAGE_SIZE = 32u;
    constexpr size_t IMAGE_ALPHA_SIZE = 36u;
    constexpr size_t IMAGE_ALPHA_SIZE_LARGE = 52u;
    constexpr size_t IMAGE_CATEGORY = 6u;
    constexpr size_t IMAGE_NAME = 28u;

    void FixImageCategory(uint8_t& category)
    {
        switch (category)
        {
        case 9:
            category = IMG_CATEGORY_AUTO_GENERATED;
            break;
        case 10:
            category = IMG_CATEGORY_LIGHTMAP;
            break;
        case 11:
            category = IMG_CATEGORY_LOAD_FROM_FILE;
            break;
        default:
            break;
        }
    }
} // namespace

Actions_GfxImage::Actions_GfxImage(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_GfxImage::LoadGfxImageLayout(ZoneInputStream& stream, const size_t count) const
{
    if (m_zone.m_version < ZoneConstants::ZONE_VERSION_ALPHA_IMAGE)
        return stream.LoadWithFill(IMAGE_SIZE * count);

    const auto large = m_zone.m_version >= ZoneConstants::ZONE_VERSION_ALPHA_IMAGE_LARGE;
    AlphaFill fill(stream, m_zone, large ? IMAGE_ALPHA_SIZE_LARGE : IMAGE_ALPHA_SIZE, IMAGE_SIZE, count);
    for (auto i = 0uz; i < count; i++)
    {
        if (large)
        {
            fill.Move(i, 0u, 0u, 20u);
            fill.Move(i, 20u, 32u, 7u);
            fill.Move(i, 27u, 40u, 1u);
            fill.Move(i, IMAGE_NAME, 48u, 4u);
        }
        else
        {
            fill.Move(i, 0u, 0u, IMAGE_NAME);
            fill.Move(i, IMAGE_NAME, 32u, 4u);
        }

        FixImageCategory(fill.Game(i)[IMAGE_CATEGORY]);
    }

    return fill.Accessor();
}

block_t Actions_GfxImage::LoadGfxImageLoadDefBlock(const block_t block) const
{
    if (m_zone.m_version >= ZoneConstants::ZONE_VERSION_ALPHA_IMAGE)
        return XFILE_BLOCK_VIRTUAL;

    return block;
}

void Actions_GfxImage::OnImageLoaded(GfxImage* image) const
{
    image->cardMemory.platform[0] = 0;
}

void Actions_GfxImage::LoadImageData(GfxImageLoadDef* loadDef, GfxImage* image) const
{
    const size_t loadDefSize = offsetof(GfxImageLoadDef, data) + loadDef->resourceSize;

    image->texture.loadDef = static_cast<GfxImageLoadDef*>(m_zone.Memory().AllocRaw(loadDefSize));
    memcpy(image->texture.loadDef, loadDef, loadDefSize);
}
