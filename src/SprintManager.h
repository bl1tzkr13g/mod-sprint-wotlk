#pragma once
#include "SprintConfig.h"
#include "Chat.h"
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace Sprint
{
struct State
{
    float energy = 1.0f;
    float baselineSpeed = 1.0f;
    uint32 elapsedMs = 0;
    bool running = false;
};
inline std::unordered_map<uint32, State>& States()
{
    static std::unordered_map<uint32, State> states;
    return states;
}
inline State& Get(Player* p) { return States()[p->GetGUID().GetCounter()]; }

inline void Send(Player* p, State const& state)
{
    if (!p || !p->GetSession()) return;
    int pct = std::clamp(static_cast<int>(std::round(state.energy * 100.0f)), 0, 100);
    ChatHandler(p->GetSession()).SendSysMessage(("AC_SPRINT:" + std::to_string(pct)).c_str());
}
inline void Stop(Player* p, State& state)
{
    if (!state.running) return;
    state.running = false;
    float bonus = std::max(0.0f, sConfigMgr->GetOption<float>("Sprint.SpeedBonus", 0.5f));
    if (std::fabs(p->GetSpeedRate(MOVE_RUN) - (state.baselineSpeed + bonus)) < 0.005f)
        p->SetSpeed(MOVE_RUN, state.baselineSpeed, true);
    else
        p->UpdateSpeed(MOVE_RUN, true);
}
inline bool Start(Player* p)
{
    if (!Allowed(p)) return false;
    State& state = Get(p);
    if (state.running || state.energy <= 0.01f) return false;
    state.baselineSpeed = p->GetSpeedRate(MOVE_RUN);
    float bonus = std::max(0.0f, sConfigMgr->GetOption<float>("Sprint.SpeedBonus", 0.5f));
    p->SetSpeed(MOVE_RUN, state.baselineSpeed + bonus, true);
    state.running = true;
    return true;
}
inline void Tick(Player* p, uint32 diff)
{
    State& state = Get(p);
    float duration = Duration(p);
    if (state.running && !Allowed(p)) Stop(p, state);
    float seconds = static_cast<float>(diff) / 1000.0f;
    if (state.running)
    {
        state.energy = std::max(0.0f, state.energy - seconds / duration);
        if (state.energy <= 0.0f) Stop(p, state);
    }
    else
    {
        float recovery = std::max(0.1f, sConfigMgr->GetOption<float>("Sprint.RecoverySeconds", 8.0f));
        state.energy = std::min(1.0f, state.energy + seconds / recovery);
    }
    state.elapsedMs += diff;
    if (state.elapsedMs >= 250)
    {
        state.elapsedMs %= 250;
        if (state.running || state.energy < 1.0f) Send(p, state);
    }
}
}
