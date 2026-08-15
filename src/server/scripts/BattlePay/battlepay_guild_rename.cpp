#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "Guild.h"
#include "GuildMgr.h"

// ---------------------------------------------------------------------
// BattlePay - Guild Rename
// LegionCore 7.3.5 (Build 26972)
//
// The LegionCore already has native support for guild rename requests:
// Guild::SetRename(true) flags the guild for rename and the client can
// then perform the actual name change through the normal guild rename flow.
//
// This script only handles the BattlePay purchase validation and delivery.
// ---------------------------------------------------------------------

class BattlePay_GuildRename : public BattlePayProductScript
{
public:
    explicit BattlePay_GuildRename(std::string scriptName)
        : BattlePayProductScript(scriptName) { }

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        if (!session)
            return;

        Player* player = session->GetPlayer();
        if (!player)
            return;

        // GetGuildByLeader guarantees that the player is the current Guild Master.
        Guild* guild = sGuildMgr->GetGuildByLeader(player->GetGUID());
        if (!guild)
            return;

        // Do not create duplicate rename requests.
        if (guild->IsFlaggedForRename())
            return;

        // LegionCore handles persistence and notifies the guild members/client.
        guild->SetRename(true);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        Player* player = session ? session->GetPlayer() : nullptr;
        if (!player)
        {
            if (session)
                reason = sObjectMgr->GetTrinityString(
                    Battlepay::String::NeedToBeInGame,
                    session->GetSessionDbLocaleIndex());
            return false;
        }

        // Only the current Guild Master can purchase Guild Rename.
        Guild* guild = sGuildMgr->GetGuildByLeader(player->GetGUID());
        if (!guild)
        {
            reason = sObjectMgr->GetTrinityString(
                Battlepay::String::GuildMasterRequired,
                session->GetSessionDbLocaleIndex());
            return false;
        }

        // The guild already has a rename request waiting for the client.
        if (guild->IsFlaggedForRename())
        {
            reason = sObjectMgr->GetTrinityString(
                Battlepay::String::YouAlreadyOwnThat,
                session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }

    std::string GetCustomData(Battlepay::Product const& /*product*/) override
    {
        return R"({"service":"guild_rename"})";
    }
};

void AddSC_BattlePay_GuildRename()
{
    new BattlePay_GuildRename("battlepay_service_guild_rename");
}
