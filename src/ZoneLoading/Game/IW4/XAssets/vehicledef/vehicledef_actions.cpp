#include "vehicledef_actions.h"

#include "Game/IW4/AlphaFillIW4.h"
#include "Game/IW4/AlphaWeaponIW4.h"
#include "Game/IW4/ZoneConstantsIW4.h"

#include <cstring>
#include <stdexcept>

using namespace IW4;

namespace
{
    constexpr size_t VEHICLE_DEF_SIZE = 720u;
    constexpr size_t VEHICLE_DEF_ALPHA_SIZE = 788u;
    constexpr size_t VEHICLE_DEF_ALPHA_GAP = 400u;
    constexpr size_t VEHICLE_DEF_ALPHA_GAP_SIZE = 8u;
    constexpr size_t VEHICLE_DEF_TURRET_WEAPON = 412u;
} // namespace

Actions_VehicleDef::Actions_VehicleDef(Zone& zone)
    : AssetLoadingActions(zone)
{
}

ZoneStreamFillReadAccessor Actions_VehicleDef::LoadVehicleDefLayout(ZoneInputStream& stream, const size_t count) const
{
    if (!ZoneConstants::IsAlphaVersion(m_zone.m_version))
        return stream.LoadWithFill(VEHICLE_DEF_SIZE * count);

    if (count != 1u)
        throw std::runtime_error("Pre-release vehicles can only be loaded one at a time");

    AlphaFill fill(stream, m_zone, VEHICLE_DEF_ALPHA_SIZE, VEHICLE_DEF_SIZE, count);
    fill.Move(0u, 0u, 0u, VEHICLE_DEF_ALPHA_GAP);
    fill.Move(0u,
              VEHICLE_DEF_ALPHA_GAP + VEHICLE_DEF_ALPHA_GAP_SIZE,
              VEHICLE_DEF_ALPHA_GAP,
              VEHICLE_DEF_SIZE - VEHICLE_DEF_ALPHA_GAP - VEHICLE_DEF_ALPHA_GAP_SIZE);

    auto* turretWeapon = fill.Game(0u) + VEHICLE_DEF_TURRET_WEAPON;
    std::memcpy(&m_turret_weapon, turretWeapon, sizeof(m_turret_weapon));
    std::memset(turretWeapon, 0, sizeof(m_turret_weapon));
    m_stream = &stream;

    return fill.Accessor();
}

void Actions_VehicleDef::LoadAlphaTurretWeapon(VehicleDef* vehicle) const
{
    if (m_stream == nullptr || m_turret_weapon == 0u)
        return;

    WeaponCompleteDef* weapon = nullptr;
    std::memcpy(&weapon, &m_turret_weapon, sizeof(m_turret_weapon));
    m_turret_weapon = 0u;

    AlphaWeaponLoader loader(m_zone, *m_stream);
    loader.Load(&weapon);
}
