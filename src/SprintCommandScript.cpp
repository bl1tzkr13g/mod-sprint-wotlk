#include "ScriptMgr.h"
#include "Chat.h"
#include "SprintManager.h"
using namespace Acore::ChatCommands;

class SprintCommandScript : public CommandScript
{
public:
    SprintCommandScript() : CommandScript("SprintCommandScript") {}
    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commands = {
            { "sprintstart", HandleStart, SEC_PLAYER, Console::No },
            { "sprintstop", HandleStop, SEC_PLAYER, Console::No }
        };
        return commands;
    }
    static bool HandleStart(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Sprint::Start(player);
        return true;
    }
    static bool HandleStop(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (player) Sprint::Stop(player, Sprint::Get(player));
        return true;
    }
};
void AddSprintCommandScript() { new SprintCommandScript(); }
