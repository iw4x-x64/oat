#pragma once

#include "Game/IGame.h"
#include "Zone/Zone.h"

#include <memory>

namespace retarget
{
    /**
     * \brief Rewrites a loaded zone as another target's zone, without
     * going through source.
     *
     * This exists for one case: the same game shipped for two word
     * sizes. IW4 and IW4MS are the same source recompiled, so on an x64
     * host most of their asset structs are the same objects in memory
     * and are handed to the other target's writer as they are. A few
     * are not: clipMap_t differs in trailing padding, snd_alias_list_t's
     * SpeakerMap was reshaped into the x64 mix matrix, water_t's spectrum
     * was split into planar halves, and LoadedSound's AILSOUNDINFO header
     * was reordered. Those are converted field by field before writing.
     *
     * \param source A zone already loaded for \p source.m_game_id. It
     *        owns the asset memory and must outlive the returned zone,
     *        which points into it. \param targetGame The game to write
     *        as. \return The retargeted zone, or nullptr if the pair is
     *        not supported, which is reported.
     */
    [[nodiscard]] std::unique_ptr<Zone> Retarget(const Zone& source, GameId targetGame);
} // namespace retarget
