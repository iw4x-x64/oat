#include "AlphaFillIW4.h"

#include <cassert>
#include <cstring>

using namespace IW4;

AlphaFill::AlphaFill(ZoneInputStream& stream, Zone& zone, const size_t alphaSize, const size_t gameSize, const size_t count)
    : m_stream(stream),
      m_alpha_size(alphaSize),
      m_game_size(gameSize),
      m_count(count),
      m_alpha_fill(stream.LoadWithFill(alphaSize * count)),
      m_game(static_cast<uint8_t*>(zone.Memory().AllocRaw(gameSize * count))),
      m_relocations(std::make_shared<std::vector<ZoneStreamFillRelocation>>())
{
    std::memset(m_game, 0, gameSize * count);
}

const uint8_t* AlphaFill::Alpha(const size_t index) const
{
    assert(index < m_count);

    return static_cast<const uint8_t*>(m_alpha_fill.BlockBuffer(index * m_alpha_size));
}

uint8_t* AlphaFill::Game(const size_t index) const
{
    assert(index < m_count);

    return &m_game[index * m_game_size];
}

void AlphaFill::Move(const size_t index, const size_t gameOffset, const size_t alphaOffset, const size_t size)
{
    assert(gameOffset + size <= m_game_size);
    assert(alphaOffset + size <= m_alpha_size);

    std::memcpy(Game(index) + gameOffset, Alpha(index) + alphaOffset, size);
    m_relocations->emplace_back(index * m_game_size + gameOffset, index * m_alpha_size + alphaOffset, size);
}

ZoneStreamFillReadAccessor AlphaFill::Accessor()
{
    return ZoneStreamFillReadAccessor(m_game, m_alpha_fill.BlockBuffer(0), m_game_size * m_count, m_stream.GetPointerBitCount() / 8u, m_relocations);
}

namespace IW4
{
    ZoneStreamFillReadAccessor LoadAlphaPrefix(ZoneInputStream& stream, Zone& zone, const size_t alphaSize, const size_t gameSize, const size_t count)
    {
        AlphaFill fill(stream, zone, alphaSize, gameSize, count);
        for (auto i = 0uz; i < count; i++)
            fill.Move(i, 0u, 0u, gameSize);

        return fill.Accessor();
    }
} // namespace IW4
