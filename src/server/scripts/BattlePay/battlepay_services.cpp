#include "ScriptMgr.h"
#include "Player.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "DB2Stores.h"
#include "DatabaseEnv.h"

#define ITEM_HEARTHSTONE 6948

template<uint32 t_Level> class BattlePay_Level : BattlePayProductScript
{
public:
    explicit BattlePay_Level(std::string scriptName) : BattlePayProductScript(scriptName) {}

    void OnProductDelivery(WorldSession* session, Battlepay::Product const& /*product*/) override
    {
        auto player = session->GetPlayer();
        if (!player)
            return;

        player->GiveLevel(t_Level);
        player->CastSpell(player, 34092, true);
        player->CastSpell(player, 54198, true);
        player->ModifyMoney(5000000);

        // only support providing gear when the level boost to level 90 or higher
        if (t_Level >= 90)
        {
            std::vector<Item*> toBeMailedCurrentEquipment;
            for (uint8 es = EquipmentSlots::EQUIPMENT_SLOT_START; es < EquipmentSlots::EQUIPMENT_SLOT_END; ++es)
            {
                if (Item* currentEquiped = player->GetItemByPos(INVENTORY_SLOT_BAG_0, es))
                {
                    ItemPosCountVec off_dest;
                    if (player->CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, currentEquiped, false) == EQUIP_ERR_OK)
                    {
                        player->RemoveItem(INVENTORY_SLOT_BAG_0, es, true);
                        player->StoreItem(off_dest, currentEquiped, true);
                    }
                    else
                        toBeMailedCurrentEquipment.push_back(currentEquiped);
                }
            }

            if (!toBeMailedCurrentEquipment.empty())
            {
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                MailDraft draft("Inventory Full: Old Equipment.",
                    "To equip your new level boost gear, your old gear had to be unequiped. You did not have enough free bag space, the items that could not be added to your bag you can find in this mail.");

                for (Item* currentEquiped : toBeMailedCurrentEquipment)
                {
                    player->MoveItemFromInventory(currentEquiped, true);
                    currentEquiped->DeleteFromInventoryDB(trans);                   // deletes item from character's inventory
                    currentEquiped->SaveToDB(trans);                                // recursive and not have transaction guard into self, item not in inventory and can be save standalone
                    draft.AddItem(currentEquiped);
                }
                
                draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
                CharacterDatabase.CommitTransaction(trans);
            }

            std::vector<uint32> toBeMailedNewItems;
            for (const uint32 item : sDB2Manager.GetLowestIdItemLoadOutItemsBy(player->getClass(), t_Level < 100 ? 3 : 6))
                if (!player->StoreNewItemInBestSlots(item, 1))
                    if (item != ITEM_HEARTHSTONE || !player->HasItemCount(ITEM_HEARTHSTONE, 1, true))
                        toBeMailedNewItems.push_back(item);

            if (!toBeMailedNewItems.empty())
            {
                SQLTransaction trans = CharacterDatabase.BeginTransaction();
                MailDraft draft("Inventory Full: Level Boost Items.",
                    "You did not have enough free bag space to add all your complementary level boost items to your bags, those that did not fit you can find in this mail.");

                for (const uint32 item : toBeMailedNewItems)
                {
                    if (Item* pItem = Item::CreateItem(item, 1, player))
                    {
                        pItem->SaveToDB(trans);
                        draft.AddItem(pItem);
                    }
                }

                draft.SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MailCheckMask(MAIL_CHECK_MASK_COPIED | MAIL_CHECK_MASK_RETURNED));
                CharacterDatabase.CommitTransaction(trans);
            }
        }

        player->SaveToDB();
    }

    bool CanBuy(WorldSession* session, Battlepay::Product const& /*product*/, std::string& reason) override
    {
        auto player = session->GetPlayer();
        if (!player)
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::NeedToBeInGame, session->GetSessionDbLocaleIndex());
            return false;
        }

        if (t_Level <= player->getLevel())
        {
            reason = sObjectMgr->GetTrinityString(Battlepay::String::TooHighLevel, session->GetSessionDbLocaleIndex());
            return false;
        }

        return true;
    }
};

class playerScriptTokensAvailable : public PlayerScript
{
public:
    playerScriptTokensAvailable() : PlayerScript("playerScriptTokensAvailable") { }

    void OnLogin(Player* player) override
    {
        if (player)
            for (auto tokenType : sBattlePayDataStore->GetTokenTypes())
                if (tokenType.second.hasLoginMessage && player->GetSession()->GetTokenBalance(tokenType.first))
                    player->SendMessageToPlayer(tokenType.second.loginMessage.c_str());
    }
};

class reachedRefererThreshold : public PlayerScript
{
public:
    reachedRefererThreshold() : PlayerScript("reachedRefererThreshold") { }

    void OnLevelChanged(Player* player, uint8 oldLevel) override
    {
        if (player == nullptr)
            return;

        // Read the config values once (they cannot change mid-tick anyway).
        uint32 trackerToken = sWorld->getIntConfig(CONFIG_REFERRAL_TRACKER_TOKEN_TYPE);
        if (trackerToken == 0)
            return;

        uint32 threshold = sWorld->getIntConfig(CONFIG_REFERRAL_TRACKER_LEVEL_THRESHOLD);
        if (threshold == 0 || threshold != oldLevel + 1)
            return;

        if (!player->GetSession() || player->GetSession()->GetReferer() == 0)
            return;

        uint32 accountId = player->GetSession()->GetAccountId();
        uint32 referer = player->GetSession()->GetReferer();

        // Check that this account has not already counted towards the total referral count.
        // NOTE: this reads from the DB, so a character that just leveled up is not yet saved
        // with its new level and correctly does NOT block its own referral on the first run.
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_NUM_ACCOUNT_CHARS_REACHED_LEVEL);
        stmt->setUInt32(0, accountId);
        stmt->setUInt8(1, threshold);
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        uint64 charsReachedThreshold = result ? (*result)[0].GetUInt64() : 0;
        if (charsReachedThreshold >= 1)
            return;

        // Read the CURRENT (pre-increment) tracker balance once, so we can derive the
        // milestone from it. Deriving from the old value avoids an extra round-trip.
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_TOKEN);
        stmt->setUInt32(0, referer);
        stmt->setUInt8(1, trackerToken);
        result = LoginDatabase.Query(stmt);

        int64 oldTokenAmount = result ? (*result)[0].GetInt64() : 0;

        uint8 numReferralThresholdReached = 0;
        switch (oldTokenAmount)
        {
        case 0:  numReferralThresholdReached = 1;  break; // new total 1
        case 1:  numReferralThresholdReached = 2;  break; // new total 2
        case 4:  numReferralThresholdReached = 3;  break; // new total 5
        case 9:  numReferralThresholdReached = 4;  break; // new total 10
        case 14: numReferralThresholdReached = 5;  break; // new total 15
        case 24: numReferralThresholdReached = 6;  break; // new total 25
        default: break;
        }

        // Atomic increment: LOGIN_INS_OR_UPD_TOKEN is "amount = amount + ?" so no
        // lost updates even if two referrer events race. Execute() is non-blocking.
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_OR_UPD_TOKEN);
        stmt->setUInt32(0, referer);
        stmt->setUInt8(1, trackerToken);
        stmt->setInt64(2, 1);
        stmt->setInt64(3, 1);
        LoginDatabase.Execute(stmt);

        if (numReferralThresholdReached == 0)
            return;

        stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_OR_UPD_TOKEN);
        stmt->setUInt32(0, referer);
        stmt->setUInt8(1, trackerToken + numReferralThresholdReached);
        stmt->setInt64(2, 1);
        stmt->setInt64(3, 1);
        LoginDatabase.Execute(stmt);
    }
};

void AddSC_BattlePay_Services()
{
    new BattlePay_Level<90>("battlepay_service_level90");
    new BattlePay_Level<100>("battlepay_service_level100");
    new playerScriptTokensAvailable();
    new reachedRefererThreshold();
}
