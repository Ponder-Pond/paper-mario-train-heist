#include "trn_01.h"
#include "generated.h"
#include "world/common/npc/Bubba.inc.c"
#include "world/common/npc/Toad_Stationary.inc.c"
#include "world/common/npc/Luigi.inc.c"
#include "world/common/enemy/Kammy.inc.c"
#include "world/common/npc/Peach.inc.c"
#include "hud_element.h"

extern IconHudScriptPair gItemHudScripts[];

namespace trn_01 {

EvtScript EVS_NpcInteract_Toadsworth = {
    Call(SpeakToPlayer, NPC_Toadsworth, ANIM_Toadsworth_Talk, ANIM_Toadsworth_Idle, 0, MSG_TRN01_ToadsworthInteract)
    Return
    End
};

EvtScript EVS_NpcInit_Toadsworth = {
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_Toadsworth))
    Return
    End
};

EvtScript EVS_NpcInteract_ToadMinister = {
    Call(SpeakToPlayer, NPC_ToadMinister, ANIM_ToadMinister_Talk, ANIM_ToadMinister_Idle, 0, MSG_TRN01_ToadMinisterInteract)
    Return
    End
};

EvtScript EVS_NpcInit_ToadMinister = {
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_ToadMinister))
    Return
    End
};

EvtScript EVS_NpcInteract_Bubba = {
    Call(SpeakToPlayer, NPC_Bubba, ANIM_BigBubba_Talk, ANIM_BigBubba_Idle, 0, MSG_TRN01_BubbaInteract)
    Return
    End
};

EvtScript EVS_NpcInit_Bubba = {
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_Bubba))
    Return
    End
};

EvtScript EVS_NpcInteract_Luigi = {
    Call(DisablePlayerInput, TRUE)
    Call(ShowMessageAtScreenPos, MSG_TRN01_LuigiInteract, 160, 40)
    // Call(SpeakToPlayer, NPC_Luigi, ANIM_LuigiSleeping_LuigiSleep, ANIM_LuigiSleeping_LuigiSleep, 0, MSG_TRN01_LuigiInteract)
    Call(DisablePlayerInput, FALSE)
    Return
    End
};

EvtScript EVS_NpcInit_Luigi = {
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_Luigi))
    Return
    End
};

EvtScript EVS_NpcInteract_Kammy = {
    Call(SpeakToPlayer, NPC_CalamityKammy, ANIM_CalamityKammy_Talk, ANIM_CalamityKammy_Idle, 0, MSG_TRN01_KammyInteract)
    Return
    End
};

EvtScript EVS_NpcInit_Kammy = {
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_Kammy))
    Return
    End
};

EvtScript EVS_NpcInteract_Peach = {
    Call(SpeakToPlayer, NPC_Peach, ANIM_ParadePeach_Talk, ANIM_ParadePeach_IdleRaisedArms, 0, MSG_TRN01_PeachInteract)
    Return
    End
};

EvtScript EVS_NpcInit_Peach = {
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_Peach))
    Return
    End
};

ShopItemData TayceTItemInventory[TAYCE_T_ITEM_COUNT] = {
    { .itemID = ITEM_POTATO_SALAD,    .price =  5, .descMsg = MSG_ItemShopDesc_AttackFXA },
    { .itemID = ITEM_NUTTY_CAKE,  .price =  5, .descMsg = MSG_ItemShopDesc_HappyHeart },
    { .itemID = ITEM_APPLE_PIE, .price =  10, .descMsg = MSG_ItemShopDesc_HappyFlower },

    { .itemID = ITEM_YUMMY_MEAL,     .price =  20, .descMsg = MSG_ItemShopDesc_PayOff },
    { .itemID = ITEM_LIFE_SHROOM,   .price =  40, .descMsg = MSG_ItemShopDesc_ChillOut },
    { .itemID = ITEM_REPEL_GEL,    .price =  15, .descMsg = MSG_ItemShopDesc_PrettyLucky },
};

API_CALLABLE((TayceT_GetPlayerCoins)) {
    Bytecode* args = script->ptrReadPos;

    evt_set_variable(script, *args++, gPlayerData.coins);
    return ApiStatus_DONE2;
}

API_CALLABLE((TayceT_SetItemPurchased)) {
    Bytecode* args = script->ptrReadPos;
    s32 index = evt_get_variable(script, *args++);

    evt_set_variable(NULL, GF_TRN01_TayceTItem_00 + index, TRUE);
    return ApiStatus_DONE2;
}

API_CALLABLE((TayceT_ShopItemsPopup)) {
    PlayerData* playerData = &gPlayerData;
    PopupMenu* menu;
    s32 selected, menuPos, i;

    if (isInitialCall) {
        script->functionTempPtr[2] = heap_malloc(sizeof(*menu));
        menu = (PopupMenu*)script->functionTempPtr[2];
        menuPos = 0;
        for (i = 0; i < TAYCE_T_ITEM_COUNT; i++) {
            if (!evt_get_variable(NULL, GF_TRN01_TayceTItem_00 + i)) {
                ItemData* item = &gItemTable[TayceTItemInventory[i].itemID];
                IconHudScriptPair* itemHudScripts = &gItemHudScripts[item->hudElemID];
                menu->userIndex[menuPos] = i;
                menu->nameMsg[menuPos] = item->nameMsg;
                menu->ptrIcon[menuPos] = itemHudScripts->enabled;
                menu->enabled[menuPos] = playerData->coins >= TayceTItemInventory[i].price;
                if (playerData->coins < TayceTItemInventory[i].price) {
                    menu->ptrIcon[menuPos] = itemHudScripts->disabled;
                    menu->enabled[menuPos] = FALSE;
                }
                menu->descMsg[menuPos] = TayceTItemInventory[i].descMsg;
                menu->value[menuPos] = TayceTItemInventory[i].price;
                menuPos++;
            }
        }
        menu->popupType = POPUP_MENU_BUY_ITEM;
        menu->numEntries = menuPos;
        menu->initialPos = 0;
        create_standard_popup_menu(menu);
        script->functionTemp[0] = 0;
    }

    menu = (PopupMenu*)script->functionTempPtr[2];
    if (script->functionTemp[0] == 0) {
        script->functionTemp[1] = menu->result;
        if (script->functionTemp[1] != POPUP_RESULT_CHOOSING) {
            hide_popup_menu();
        } else {
            return ApiStatus_BLOCK;
        }
    }
    script->functionTemp[0]++;
    if (script->functionTemp[0] < 20) {
        return ApiStatus_BLOCK;
    }

    destroy_popup_menu();
    selected = script->functionTemp[1];
    if (selected != POPUP_RESULT_CANCEL) {
        ShopItemData* selectedItem;
        i = menu->userIndex[selected - 1];
        selectedItem = &TayceTItemInventory[i];
        script->varTable[0] = selectedItem->itemID;
        script->varTable[1] = selectedItem->price;
        script->varTable[2] = i;
        script->varTable[4] = gItemTable[selectedItem->itemID].nameMsg;
    } else {
        script->varTable[0] = -1;
    }

    heap_free(script->functionTempPtr[2]);
    return ApiStatus_DONE2;
}

EvtScript EVS_NpcInteract_TayceT = {
    IfGe(GB_TRN01_TayceT_PurchaseCount, TAYCE_T_ITEM_COUNT)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT6)
        Return
    EndIf
    IfEq(MF_PurchasedItem, TRUE)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT3)
    Else
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT1)
    EndIf
    Call(ShowChoice, MSG_Choice_0014)
    IfEq(LVar0, 1)
        Call(ContinueSpeech, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT4)
        Return
    EndIf
    Call((TayceT_GetPlayerCoins), LVar0)
    IfEq(LVar0, 0)
        Call(ContinueSpeech, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT5)
        Return
    EndIf
    Call(ContinueSpeech, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT7)
    Label(0)
    Call((TayceT_ShopItemsPopup))
    Wait(10)
    IfEq(LVar0, -1)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT8)
        Return
    EndIf
    Call((TayceT_GetPlayerCoins), LVar3)
    IfLt(LVar3, LVar1)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT8)
        Goto(0)
    EndIf
    Call(SetMessageText, LVar4, 0)
    Call(SetMessageValue, LVar1, 1)
    Call(SetMessageValue, LVar5, 2)
    IfEq(LVar1, 1)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT2)
    Else
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT9)
    EndIf
    Set(LVar3, LVar0)
    Call(ShowChoice, MSG_Choice_000D)
    IfEq(LVar0, 1)
        Call(ContinueSpeech, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT10)
        Goto(0)
    EndIf
    Call(CloseMessage)
    Mul(LVar1, -1)
    Call(AddCoin, LVar1)
    Add(GB_TRN01_TayceT_PurchaseCount, 1)
    Set(MF_PurchasedItem, TRUE)
    Call((TayceT_SetItemPurchased), LVar2)
    // awkward
    // #define NAME_SUFFIX _Tayce_T
    Set(LVar0, LVar3)
    Call(ShowGotItem, LVar0, TRUE, 0)
    Call(AddItem, LVar0, LVar1)
    // EVT_GIVE_REWARD(LVar3)
    // #define NAME_SUFFIX
    IfGe(GB_TRN01_TayceT_PurchaseCount, TAYCE_T_ITEM_COUNT)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT11)
        Return
    EndIf
    Call((TayceT_GetPlayerCoins), LVar0)
    IfLe(LVar0, 0)
        Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT11)
        Return
    EndIf
    Call(SpeakToPlayer, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT12)
    Call(ShowChoice, MSG_Choice_000D)
    IfEq(LVar0, 1)
        Call(ContinueSpeech, NPC_TayceT, ANIM_TayceT_Cooking, ANIM_TayceT_Cooking, 0, MSG_TRN01_BuyTayceT13)
        Return
    EndIf
    Call(CloseMessage)
    Goto(0)
    Return
    End
};

EvtScript EVS_NpcInit_TayceT = {
    Set(MF_PurchasedItem, FALSE)
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_TayceT))
    Return
    End
};

ShopItemData ToadBadgeInventory[TOAD_BADGE_COUNT] = {
    { .itemID = ITEM_FIRE_SHIELD,    .price =  3, .descMsg = MSG_ItemShopDesc_AttackFXA },
    { .itemID = ITEM_HAPPY_HEART_A,  .price =  15, .descMsg = MSG_ItemShopDesc_HappyHeart },
    { .itemID = ITEM_HAPPY_FLOWER_A, .price =  15, .descMsg = MSG_ItemShopDesc_HappyFlower },

    { .itemID = ITEM_POWER_JUMP,     .price =  10, .descMsg = MSG_ItemShopDesc_PayOff },
    { .itemID = ITEM_SHRINK_STOMP,   .price =  7, .descMsg = MSG_ItemShopDesc_ChillOut },
    { .itemID = ITEM_POWER_SMASH,    .price =  10, .descMsg = MSG_ItemShopDesc_PrettyLucky },

    { .itemID = ITEM_JUMP_CHARGE,    .price = 3, .descMsg = MSG_ItemShopDesc_FeelingFine },
    { .itemID = ITEM_POWER_RUSH,     .price = 7, .descMsg = MSG_ItemShopDesc_Peekaboo },

    { .itemID = ITEM_CLOSE_CALL,     .price = 5, .descMsg = MSG_ItemShopDesc_ZapTap },
    { .itemID = ITEM_LAST_STAND,     .price = 5, .descMsg = MSG_ItemShopDesc_HeartFinder },
    { .itemID = ITEM_PRETTY_LUCKY,   .price = 10, .descMsg = MSG_ItemShopDesc_FlowerFinder },

    { .itemID = ITEM_DOUBLE_DIP,       .price = 3, .descMsg = MSG_ItemShopDesc_HPDrain },
    { .itemID = ITEM_QUICK_CHANGE,     .price = 20, .descMsg = MSG_ItemShopDesc_MoneyMoney },
    { .itemID = ITEM_ALLOR_NOTHING,    .price = 10, .descMsg = MSG_ItemShopDesc_FlowerSaver },
};

API_CALLABLE((Toad_GetPlayerStarPieces)) {
    Bytecode* args = script->ptrReadPos;

    evt_set_variable(script, *args++, gPlayerData.starPieces);
    return ApiStatus_DONE2;
}

API_CALLABLE((Toad_SetBadgePurchased)) {
    Bytecode* args = script->ptrReadPos;
    s32 index = evt_get_variable(script, *args++);

    evt_set_variable(NULL, GF_TRN01_ToadBadge_00 + index, TRUE);
    return ApiStatus_DONE2;
}

API_CALLABLE((Toad_ShopBadgesPopup)) {
    PlayerData* playerData = &gPlayerData;
    PopupMenu* menu;
    s32 selected, menuPos, i;

    if (isInitialCall) {
        script->functionTempPtr[2] = heap_malloc(sizeof(*menu));
        menu = (PopupMenu*)script->functionTempPtr[2];
        menuPos = 0;
        for (i = 0; i < TOAD_BADGE_COUNT; i++) {
            if (!evt_get_variable(NULL, GF_TRN01_ToadBadge_00 + i)) {
                ItemData* item = &gItemTable[ToadBadgeInventory[i].itemID];
                IconHudScriptPair* itemHudScripts = &gItemHudScripts[item->hudElemID];
                menu->userIndex[menuPos] = i;
                menu->nameMsg[menuPos] = item->nameMsg;
                menu->ptrIcon[menuPos] = itemHudScripts->enabled;
                menu->enabled[menuPos] = playerData->starPieces >= ToadBadgeInventory[i].price;
                if (playerData->starPieces < ToadBadgeInventory[i].price) {
                    menu->ptrIcon[menuPos] = itemHudScripts->disabled;
                    menu->enabled[menuPos] = FALSE;
                }
                menu->descMsg[menuPos] = ToadBadgeInventory[i].descMsg;
                menu->value[menuPos] = ToadBadgeInventory[i].price;
                menuPos++;
            }
        }
        menu->popupType = POPUP_MENU_TRADE_FOR_BADGE;
        menu->numEntries = menuPos;
        menu->initialPos = 0;
        create_standard_popup_menu(menu);
        script->functionTemp[0] = 0;
    }

    menu = (PopupMenu*)script->functionTempPtr[2];
    if (script->functionTemp[0] == 0) {
        script->functionTemp[1] = menu->result;
        if (script->functionTemp[1] != POPUP_RESULT_CHOOSING) {
            hide_popup_menu();
        } else {
            return ApiStatus_BLOCK;
        }
    }
    script->functionTemp[0]++;
    if (script->functionTemp[0] < 20) {
        return ApiStatus_BLOCK;
    }

    destroy_popup_menu();
    selected = script->functionTemp[1];
    if (selected != POPUP_RESULT_CANCEL) {
        ShopItemData* selectedItem;
        i = menu->userIndex[selected - 1];
        selectedItem = &ToadBadgeInventory[i];
        script->varTable[0] = selectedItem->itemID;
        script->varTable[1] = selectedItem->price;
        script->varTable[2] = i;
        script->varTable[4] = gItemTable[selectedItem->itemID].nameMsg;
        script->varTable[5] = gMoveTable[gItemTable[selectedItem->itemID].moveID].costBP;
    } else {
        script->varTable[0] = -1;
    }

    heap_free(script->functionTempPtr[2]);
    return ApiStatus_DONE2;
}

EvtScript EVS_NpcInteract_Toad = {
    IfGe(GB_TRN01_Toad_PurchaseCount, TOAD_BADGE_COUNT)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad6)
        Return
    EndIf
    IfEq(MF_PurchasedBadge, TRUE)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad3)
    Else
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad1)
    EndIf
    Call(ShowChoice, MSG_Choice_0014)
    IfEq(LVar0, 1)
        Call(ContinueSpeech, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad4)
        Return
    EndIf
    Call((Toad_GetPlayerStarPieces), LVar0)
    IfEq(LVar0, 0)
        Call(ContinueSpeech, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad5)
        Return
    EndIf
    Call(ContinueSpeech, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad7)
    Label(0)
    Call((Toad_ShopBadgesPopup))
    Wait(10)
    IfEq(LVar0, -1)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad8)
        Return
    EndIf
    Call((Toad_GetPlayerStarPieces), LVar3)
    IfLt(LVar3, LVar1)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad8)
        Goto(0)
    EndIf
    Call(SetMessageText, LVar4, 0)
    Call(SetMessageValue, LVar1, 1)
    Call(SetMessageValue, LVar5, 2)
    IfEq(LVar1, 1)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad2)
    Else
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad9)
    EndIf
    Set(LVar3, LVar0)
    Call(ShowChoice, MSG_Choice_000D)
    IfEq(LVar0, 1)
        Call(ContinueSpeech, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad10)
        Goto(0)
    EndIf
    Call(CloseMessage)
    Mul(LVar1, -1)
    Call(AddStarPieces, LVar1)
    Add(GB_TRN01_Toad_PurchaseCount, 1)
    Set(MF_PurchasedBadge, TRUE)
    Call((Toad_SetBadgePurchased), LVar2)
    // awkward
    // #define NAME_SUFFIX _Toad
    Set(LVar0, LVar3)
    Call(ShowGotItem, LVar0, TRUE, 0)
    Call(AddItem, LVar0, LVar1)
    // EVT_GIVE_REWARD(LVar3)
    // #define NAME_SUFFIX
    IfGe(GB_TRN01_Toad_PurchaseCount, TOAD_BADGE_COUNT)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad11)
        Return
    EndIf
    Call((Toad_GetPlayerStarPieces), LVar0)
    IfLe(LVar0, 0)
        Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad11)
        Return
    EndIf
    Call(SpeakToPlayer, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad12)
    Call(ShowChoice, MSG_Choice_000D)
    IfEq(LVar0, 1)
        Call(ContinueSpeech, NPC_Toad, ANIM_Toad_Red_Talk, ANIM_Toad_Red_Idle, 0, MSG_TRN01_BorrowToad13)
        Return
    EndIf
    Call(CloseMessage)
    Goto(0)
    Return
    End
};

EvtScript EVS_NpcInit_Toad = {
    Set(MF_PurchasedBadge, FALSE)
    Call(BindNpcInteract, NPC_SELF, Ref(EVS_NpcInteract_Toad))
    Return
    End
};

NpcData NpcData_Characters[] = {
    {
        .id = NPC_Toadsworth,
        .settings = &N(NpcSettings_Toad_Stationary),
        .pos = { GEN_TOADSWORTH_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_Toadsworth,
        .yaw = GEN_TOADSWORTH_DIR,
        .drops = NO_DROPS,
        .animations = TOADSWORTH_ANIMS, // Change to Toadsworth Animations
        .tattle = MSG_TRN01_ToadsworthTattle,
    },
    {
        .id = NPC_ToadMinister,
        .settings = &N(NpcSettings_Toad_Stationary),
        .pos = { GEN_TOAD_MINISTER_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_ToadMinister,
        .yaw = GEN_TOAD_MINISTER_DIR,
        .drops = NO_DROPS,
        .animations = TOAD_MINISTER_ANIMS,
        .tattle = MSG_TRN01_ToadMinisterTattle,
    },
    {
        .id = NPC_Bubba,
        .settings = &N(NpcSettings_Bubba),
        .pos = { GEN_BUBBA_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_Bubba,
        .yaw = GEN_BUBBA_DIR,
        .drops = NO_DROPS,
        .animations = BUBBA_ANIMS,
        .tattle = MSG_TRN01_BigBubbaTattle,
    },
    {
        .id = NPC_Luigi,
        .settings = &N(NpcSettings_Luigi),
        .pos = { GEN_LUIGI_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_Luigi,
        .yaw = GEN_LUIGI_DIR,
        .drops = NO_DROPS,
        .animations = LUIGI_SLEEPING_ANIMS,
        .tattle = MSG_TRN01_LuigiTattle,
    },
    {
        .id = NPC_CalamityKammy,
        .settings = &N(NpcSettings_Kammy),
        .pos = { GEN_CALAMITY_KAMMY_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_Kammy,
        .yaw = GEN_CALAMITY_KAMMY_DIR,
        .drops = NO_DROPS,
        .animations = CALAMITY_KAMMY_ANIMS,
        .tattle = MSG_TRN01_CalamityKammyTattle,
    },
    {
        .id = NPC_Peach,
        .settings = &N(NpcSettings_Peach),
        .pos = { GEN_PEACH_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_Peach,
        .yaw = GEN_PEACH_DIR,
        .drops = NO_DROPS,
        .animations = PEACH_NPC_ANIMS,
        .tattle = MSG_TRN01_PeachTattle,
    },
    {
        .id = NPC_TayceT,
        .settings = &N(NpcSettings_Toad_Stationary),
        .pos = { GEN_TAYCE_T_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_TayceT,
        .yaw = GEN_TAYCE_T_DIR,
        .drops = NO_DROPS,
        .animations = TAYCE_T_ANIMS,
        .tattle = MSG_TRN01_TayceTTattle,
    },
    {
        .id = NPC_Toad,
        .settings = &N(NpcSettings_Toad_Stationary),
        .pos = { GEN_TOAD_VEC },
        .flags = COMMON_PASSIVE_FLAGS | ENEMY_FLAG_NO_SHADOW_RAYCAST,
        .init = &EVS_NpcInit_Toad,
        .yaw = GEN_TOAD_DIR,
        .drops = NO_DROPS,
        .animations = TOAD_RED_ANIMS,
        .tattle = MSG_TRN01_ToadTattle,
    },
};

NpcGroupList DefaultNPCs = {
    NPC_GROUP(NpcData_Characters),
    {}
};

EvtScript EVS_Scene_BeginGame = {
    Call(DisablePlayerInput, TRUE)
    Thread
        Wait(5)
        Call(SetMusicTrack, 0, SONG_WHALE_THEME, 0, 8)
        Wait(5)
    EndThread
    Call(DisablePlayerInput, FALSE)
    Return
    End
};

}; // namespace trn_01
