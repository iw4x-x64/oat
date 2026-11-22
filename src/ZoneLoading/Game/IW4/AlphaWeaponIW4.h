#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/ContentLoaderBase.h"

#include <cstdint>

namespace IW4
{
    class AlphaWeaponLoader final : public ContentLoaderBase
    {
    public:
        AlphaWeaponLoader(Zone& zone, ZoneInputStream& stream);

        void Load(WeaponCompleteDef** pAsset);

    private:
        void LoadBody();

        [[nodiscard]] uint32_t Raw(size_t offset) const;
        [[nodiscard]] int16_t Short(size_t offset) const;

        void String(size_t offset);
        void Strings(size_t offset, size_t count);
        void LoadSound(uint32_t raw);
        void Sound(size_t offset);
        void Sounds(size_t offset, size_t count);
        void SoundArray(size_t offset);
        void Vec2Array(size_t offset, size_t countOffset);
        void Model(size_t offset);
        void Effect(size_t offset);
        void MaterialHandle(size_t offset);
        void Collmap(size_t offset);
        void Preset(size_t offset);
        void Tracer(size_t offset);

        const ZoneStreamFillReadAccessor* m_fill = nullptr;
        size_t m_size = 0;
        size_t m_shift = 0;
    };
} // namespace IW4
