#pragma once
#include "Config.h"
#include "Player.h"
#include "Map.h"
#include <algorithm>
#include <cmath>

namespace Sprint
{
inline bool Enabled() { return sConfigMgr->GetOption<bool>("Sprint.Enabled", true); }

inline bool Allowed(Player* p)
{
    if (!p || !p->IsInWorld() || !p->IsAlive() || !Enabled()) return false;
    Map* map = p->GetMap();
    if (!map) return false;
    if (map->IsBattleArena() && !sConfigMgr->GetOption<bool>("Sprint.AllowArenas", false)) return false;
    if (map->IsBattleground() && !sConfigMgr->GetOption<bool>("Sprint.AllowBattlegrounds", false)) return false;
    if (!sConfigMgr->GetOption<bool>("Sprint.AllowCombat", false) && p->IsInCombat()) return false;
    if (!sConfigMgr->GetOption<bool>("Sprint.AllowMounts", false) && p->IsMounted()) return false;
    if (p->IsFlying() || p->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) || p->GetVehicle()) return false;
    return true;
}

inline float Duration(Player* p)
{
    float stamina = std::max(0.0f, p->GetStat(STAT_STAMINA));
    float reference = std::max(1.0f, sConfigMgr->GetOption<float>("Sprint.StaminaReference", 100.0f));
    float seconds = sConfigMgr->GetOption<float>("Sprint.DurationBase", 4.0f)
        + sConfigMgr->GetOption<float>("Sprint.DurationLogFactor", 2.5f) * std::log2(1.0f + stamina / reference);
    return std::clamp(seconds, 1.0f, std::max(1.0f, sConfigMgr->GetOption<float>("Sprint.DurationCap", 17.0f)));
}
}
