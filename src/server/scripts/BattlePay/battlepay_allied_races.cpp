#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"

// ---------------------------------------------------------------------
// Razas aliadas de Legion (paywall via BattlePay)
//
// Mecanismo: el desbloqueo de razas aliadas en este core se resuelve
// por logro A NIVEL DE CUENTA (no de personaje), vía la tabla
// account_achievement (CharacterDatabase) + WorldSession::SetAchievement
// para que tenga efecto inmediato en memoria sin necesidad de relog.
//
// IDs de logro confirmados (ver race_unlock_requirement, WorldDatabase):
//   raceID 27 = Nightborne            -> achievementId 12244
//   raceID 28 = Highmountain Tauren   -> achievementId 12245
//   raceID 29 = Void Elf              -> achievementId 12242
//   raceID 30 = Lightforged Draenei   -> achievementId 12243
// (todas expansion = 6, Legion)
// ---------------------------------------------------------------------

static void GrantAlliedRaceAchievement(WorldSession* session, Player* player, uint32 achievementId)
{
    // Persistir en account_achievement para que sobreviva a un logout/relog
    // (esto es lo que gatilla el desbloqueo de la raza en pantalla de creación)
    CharacterDatabase.PExecute(
        "INSERT INTO account_achievement (account, first_guid, achievement, date) "
        "VALUES (%u, %u, %u, UNIX_TIMESTAMP()) "
        "ON DUPLICATE KEY UPDATE date = UNIX_TIMESTAMP()",
        session->GetAccountId(), player->GetGUID().GetCounter(), achievementId);

    // Efecto inmediato en memoria (sin esperar al próximo login)
    session->SetAchievement(achievementId);

    // Completar el logro "de verdad" (popup, marca en la ventana de logros, etc.)
    if (AchievementEntry const* achievement = sAchievementStore.LookupEntry(achievementId))
        player->CompletedAchievement(achievement);
}

template<uint32 t_AchievementId> class BattlePay_AlliedRace : public BattlePayProductScript
{
public:
    explicit BattlePay_AlliedRace(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        GrantAlliedRaceAchievement(session, player, t_AchievementId);
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        if (session->HasAchievement(t_AchievementId))
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::YouAlreadyOwnThat, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

void AddSC_BattlePay_AlliedRaces()
{
    new BattlePay_AlliedRace<12244>("legion_nightborne");
    new BattlePay_AlliedRace<12245>("legion_highmountain");
    new BattlePay_AlliedRace<12242>("legion_void");
    new BattlePay_AlliedRace<12243>("legion_lfdraenei");
}
