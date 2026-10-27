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
    float exhaustionSeconds = 0.0f;
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
    if (state.running || state.energy <= 0.01f || state.exhaustionSeconds > 0.0f) return false;
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
        if (state.energy <= 0.0f)
        {
            Stop(p, state);
            if (sConfigMgr->GetOption<bool>("Sprint.Exhaustion.Enabled", true))
                state.exhaustionSeconds = std::max(0.0f, sConfigMgr->GetOption<float>("Sprint.Exhaustion.DelaySeconds", 3.0f));
        }
    }
    else if (state.exhaustionSeconds > 0.0f)
    {
        // Consume the delay first. A large server tick may also contain recovery time.
        float delayConsumed = std::min(seconds, state.exhaustionSeconds);
        state.exhaustionSeconds -= delayConsumed;
        seconds -= delayConsumed;
    }
    if (!state.running && state.exhaustionSeconds <= 0.0f && seconds > 0.0f)
    {
        float recovery = std::max(0.1f, sConfigMgr->GetOption<float>("Sprint.RecoverySeconds", 8.0f));
        float multiplier = 1.0f;

        // Vitality uses current health percentage, not the Stamina attribute.
        if (sConfigMgr->GetOption<bool>("Sprint.Vitality.Enabled", true))
        {
            float hp = std::clamp(p->GetHealthPct() / 100.0f, 0.0f, 1.0f);
            float minimum = std::clamp(sConfigMgr->GetOption<float>("Sprint.Vitality.MinimumMultiplier", 0.2f), 0.0f, 1.0f);
            multiplier *= minimum + (1.0f - minimum) * hp;
        }
        if (p->IsInCombat())
            multiplier *= std::max(0.0f, sConfigMgr->GetOption<float>("Sprint.CombatRecoveryMultiplier", 0.5f));
        if (p->IsSitState() && !p->IsInCombat())
            multiplier *= std::max(0.0f, sConfigMgr->GetOption<float>("Sprint.RestingRecoveryMultiplier", 2.0f));

        state.energy = std::min(1.0f, state.energy + (seconds / recovery) * multiplier);
    }
    state.elapsedMs += diff;
    if (state.elapsedMs >= 250)
    {
        state.elapsedMs %= 250;
        if (state.running || state.energy < 1.0f) Send(p, state);
    }
}
}
