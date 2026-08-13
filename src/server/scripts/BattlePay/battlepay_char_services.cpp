#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "CharacterService.h"

// ---------------------------------------------------------------------
// Rename
// ---------------------------------------------------------------------
class BattlePay_Rename : public BattlePayProductScript
{
public:
    explicit BattlePay_Rename(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        sCharacterService->SetRename(player);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

// ---------------------------------------------------------------------
// Faction Change
// ---------------------------------------------------------------------
class BattlePay_FactionChange : public BattlePayProductScript
{
public:
    explicit BattlePay_FactionChange(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        sCharacterService->ChangeFaction(player);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

// ---------------------------------------------------------------------
// Race Change
// ---------------------------------------------------------------------
class BattlePay_RaceChange : public BattlePayProductScript
{
public:
    explicit BattlePay_RaceChange(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        sCharacterService->ChangeRace(player);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

// ---------------------------------------------------------------------
// Customize (apariencia)
// ---------------------------------------------------------------------
class BattlePay_Customize : public BattlePayProductScript
{
public:
    explicit BattlePay_Customize(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        sCharacterService->Customize(player);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

void AddSC_BattlePay_CharServices()
{
    new BattlePay_Rename("battlepay_service_rename");
    new BattlePay_FactionChange("battlepay_service_change_faction");
    new BattlePay_RaceChange("battlepay_service_change_race");
    new BattlePay_Customize("battlepay_service_customize");
}
