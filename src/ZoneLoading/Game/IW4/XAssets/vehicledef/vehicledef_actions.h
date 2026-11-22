#pragma once

#include "Game/IW4/IW4.h"
#include "Loading/AssetLoadingActions.h"
#include "Zone/Stream/ZoneInputStream.h"

#include <cstdint>

namespace IW4
{
    class Actions_VehicleDef final : public AssetLoadingActions
    {
    public:
        explicit Actions_VehicleDef(Zone& zone);

        [[nodiscard]] ZoneStreamFillReadAccessor LoadVehicleDefLayout(ZoneInputStream& stream, size_t count) const;
        void LoadAlphaTurretWeapon(VehicleDef* vehicle) const;

    private:
        mutable ZoneInputStream* m_stream = nullptr;
        mutable uint32_t m_turret_weapon = 0;
    };
} // namespace IW4
