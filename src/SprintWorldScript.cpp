#include "ScriptMgr.h"
#include "WorldSessionMgr.h"
#include "SprintManager.h"

class SprintWorldScript : public WorldScript
{
public:
    SprintWorldScript() : WorldScript("SprintWorldScript") {}
    void OnUpdate(uint32 diff) override
    {
        for (auto const& entry : sWorldSessionMgr->GetAllSessions())
        {
            WorldSession* session = entry.second;
            if (!session) continue;
            Player* player = session->GetPlayer();
            if (!player || !player->IsInWorld()) continue;
            Sprint::Tick(player, diff);
        }
    }
};
void AddSprintWorldScript() { new SprintWorldScript(); }
