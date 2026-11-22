#pragma once

#include "Zone/Stream/ZoneInputStream.h"
#include "Zone/Zone.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace IW4
{
    class AlphaFill
    {
    public:
        AlphaFill(ZoneInputStream& stream, Zone& zone, size_t alphaSize, size_t gameSize, size_t count);

        [[nodiscard]] const uint8_t* Alpha(size_t index) const;
        [[nodiscard]] uint8_t* Game(size_t index) const;

        void Move(size_t index, size_t gameOffset, size_t alphaOffset, size_t size);

        [[nodiscard]] ZoneStreamFillReadAccessor Accessor();

    private:
        ZoneInputStream& m_stream;
        size_t m_alpha_size;
        size_t m_game_size;
        size_t m_count;
        ZoneStreamFillReadAccessor m_alpha_fill;
        uint8_t* m_game;
        std::shared_ptr<std::vector<ZoneStreamFillRelocation>> m_relocations;
    };

    [[nodiscard]] ZoneStreamFillReadAccessor LoadAlphaPrefix(ZoneInputStream& stream, Zone& zone, size_t alphaSize, size_t gameSize, size_t count);
} // namespace IW4
