#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "AchievementMgr.h"
#include "DB2Stores.h"

// ---------------------------------------------------------------------
// WoD Pathfinder (Draenor Pathfinder, achievement 10018)
// ---------------------------------------------------------------------
class BattlePay_WodPathfinder : public BattlePayProductScript
{
public:
    explicit BattlePay_WodPathfinder(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        if (AchievementEntry const* achievement = sAchievementStore.LookupEntry(10018)) // Draenor Pathfinder
            player->CompletedAchievement(achievement);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        if (player->HasAchieved(10018))
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::YouAlreadyOwnThat, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

// ---------------------------------------------------------------------
// Legion Pathfinder (Broken Isles Pathfinder, Part One 11190 + Part Two 11446)
// ---------------------------------------------------------------------
class BattlePay_LegionPathfinder : public BattlePayProductScript
{
public:
    explicit BattlePay_LegionPathfinder(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        if (AchievementEntry const* partOne = sAchievementStore.LookupEntry(11190)) // Broken Isles Pathfinder, Part One
            player->CompletedAchievement(partOne);

        if (AchievementEntry const* partTwo = sAchievementStore.LookupEntry(11446)) // Broken Isles Pathfinder, Part Two
            player->CompletedAchievement(partTwo);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        if (player->HasAchieved(11190) && player->HasAchieved(11446))
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::YouAlreadyOwnThat, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

void AddSC_BattlePay_Pathfinder()
{
    new BattlePay_WodPathfinder("wod_pathfinder");
    new BattlePay_LegionPathfinder("legion_pathfinder");
}
