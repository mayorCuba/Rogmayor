#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "TemporarySummon.h"

// ---------------------------------------------------------------------
// Goblin Mercenary Companion (entry 638)
// Invoca el NPC "Goblin Mercenary" (creature_template entry 638), lo
// hace seguir al jugador y lo pone en modo defensivo (ataca a quien
// ataque al jugador). El AIName de este creature está vacío en la DB,
// así que hoy usa el comportamiento de combate por defecto del motor
// (cuerpo a cuerpo / a distancia según su configuración base).
//
// PENDIENTE (a implementar en una próxima vuelta si se quiere más
// detalle): una CreatureAI propia con casteo periódico de hechizos
// temáticos de Hunter o Warrior, en vez del combate genérico.
// ---------------------------------------------------------------------
constexpr uint32 NPC_GOBLIN_MERCENARY = 638;
constexpr uint32 GOBLIN_MERCENARY_DURATION = 30 * 60 * 1000; // 30 minutos, ajustable

class BattlePay_GoblinMercenaryCompanion : public BattlePayProductScript
{
public:
    explicit BattlePay_GoblinMercenaryCompanion(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        if (Creature* companion = player->SummonCreature(NPC_GOBLIN_MERCENARY, *player,
                TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, GOBLIN_MERCENARY_DURATION))
        {
            companion->SetReactState(REACT_DEFENSIVE);
            companion->setFaction(player->getFaction());
            companion->GetMotionMaster()->MoveFollow(player, 2.0f, 0.0f);
        }
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

void AddSC_BattlePay_GoblinMercenary()
{
    new BattlePay_GoblinMercenaryCompanion("goblin_hunter");
    new BattlePay_GoblinMercenaryCompanion("goblin_warrior");
}
