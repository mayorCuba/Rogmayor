#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "AchievementMgr.h"
#include "DB2Stores.h"

// ---------------------------------------------------------------------
// Legion Allied Races Unlock
//
// Unlocks the four Allied Races introduced with patch 7.3.5 by completing
// their hidden character achievements:
//   12450 - Allied Races: Void Elf (Hidden Character Achievement)
//   12451 - Allied Races: Lightforged Draenei (Hidden Character Achievement)
//   12452 - Allied Races: Highmountain Tauren (Hidden Character Achievement)
//   12453 - Allied Races: Nightborne (Hidden Character Achievement)
// ---------------------------------------------------------------------
namespace BattlePay
{
    enum AlliedRaceUnlockAchievement : uint32
    {
        VoidElf         = 12450,
        Lightforged     = 12451,
        Highmountain    = 12452,
        Nightborne      = 12453
    };
}

class BattlePay_AlliedRaces : public BattlePayProductScript
{
public:
    explicit BattlePay_AlliedRaces(std::string scriptName) : BattlePayProductScript(scriptName) { }

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        Player* player = session ? session->GetPlayer() : nullptr;
        if (!player)
            return;

        static constexpr uint32 achievements[] =
        {
            BattlePay::VoidElf,
            BattlePay::Lightforged,
            BattlePay::Highmountain,
            BattlePay::Nightborne
        };

        for (uint32 achievementId : achievements)
        {
            if (AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementId))
                if (!player->HasAchieved(achievementId))
                    player->CompletedAchievement(achievement);
        }
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        Player* player = session ? session->GetPlayer() : nullptr;
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(
                Battlepay::String::NeedToBeInGame,
                session->GetSessionDbLocaleIndex());
            return false;
        }

        if (player->HasAchieved(BattlePay::VoidElf) &&
            player->HasAchieved(BattlePay::Lightforged) &&
            player->HasAchieved(BattlePay::Highmountain) &&
            player->HasAchieved(BattlePay::Nightborne))
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
        return R"({"service":"allied_races","version":1})";
    }
};

void AddSC_BattlePay_AlliedRaces()
{
    new BattlePay_AlliedRaces("battlepay_service_allied_races");
}
