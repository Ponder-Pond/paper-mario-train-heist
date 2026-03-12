#include "common.h"
#include "ld_addrs.h"
#include "battle.h"
#include "battle_tables.h"
#include "hud_element.h"
#include "sprite.h"
#include "game_modes.h"
#include "battle/states/states.h"
#include "dx/debug_menu.h"

#include "area/kmr_part_1/area.hpp"

extern "C" {

BSS StageListRow* gCurrentStagePtr;
BSS s32 gBattleState;
BSS BattleStatus gBattleStatus;
BSS s32 gLastDrawBattleState;
BSS s32 gDefeatedBattleSubstate;
BSS s32 gBattleSubState;
BSS s32 gDefeatedBattleState;
BSS s32 gCurrentBattleID;
BSS s32 gCurrentStageID;
BSS Battle* gOverrideBattlePtr;
BSS Battle* gCurrentBattlePtr;

// standard battle area table entry
#define BTL_AREA(id, debugName) { \
    .name = debugName, \
    .dmaStart = battle_area_##id##_ROM_START, \
    .dmaEnd = battle_area_##id##_ROM_END, \
    .dmaDest = battle_area_##id##_VRAM, \
    .battles = &battle::area::id::Battles, \
    .stages = &battle::area::id::Stages, \
} \

#define BTL_AREA_C(id, debugName) { \
    .name = debugName, \
    .dmaStart = battle_area_##id##_ROM_START, \
    .dmaEnd = battle_area_##id##_ROM_END, \
    .dmaDest = battle_area_##id##_VRAM, \
    .battles = &b_area_##id##_Formations, \
    .stages = &b_area_##id##_Stages, \
} \

// extended battle area with a dmaTable, used by kzn2 for lava piranha animations
#define BTL_AREA_DMA(id, debugName) { \
    .name = debugName, \
    .dmaStart = battle_area_##id##_ROM_START, \
    .dmaEnd = battle_area_##id##_ROM_END, \
    .dmaDest = battle_area_##id##_VRAM, \
    .battles = &b_area_##id##_Formations, \
    .stages = &b_area_##id##_Stages, \
    .dmaTable = b_area_##id##_dmaTable, \
} \

// auxiliary battle area for omo which contains only additional enemy data
#define BTL_AREA_AUX(id, debugName) { \
    .name = debugName, \
    .dmaStart = battle_area_##id##_ROM_START, \
    .dmaEnd = battle_area_##id##_ROM_END, \
    .dmaDest = battle_area_##id##_VRAM, \
} \

// when adding an area, remember to update battle.h and add the new area to battle_tables.h as well
BattleArea gBattleAreas[] = {
    BTL_AREA(kmr_part_1, "KMR Part 1"),
    BTL_AREA_C(kmr_part_2, "KMR Part 2"),
    BTL_AREA_C(kmr_part_3, "KMR Part 3"),
    BTL_AREA_C(mac, "Toad Town"),
    BTL_AREA_C(hos, "Star Haven"),
    BTL_AREA_C(nok, "Plesant Path"),
    BTL_AREA_C(trd_part_1, "Fotress 1"),
    BTL_AREA_C(trd_part_2, "Fotress 2"),
    BTL_AREA_C(trd_part_3, "Fotress 3"),
    BTL_AREA_C(iwa, "Mountain"),
    BTL_AREA_C(sbk, "SBK"),
    BTL_AREA_C(isk_part_1, "ISk Part 1"),
    BTL_AREA_C(isk_part_2, "ISK Part 2"),
    BTL_AREA_C(mim, "MIM"),
    BTL_AREA_C(arn, "ARN"),
    BTL_AREA_C(dgb, "DGB"),
    BTL_AREA_C(omo, "OMO"),
    BTL_AREA_C(omo2, "OMO2"),
    BTL_AREA_C(omo3, "OMO3"),
    BTL_AREA_C(kgr, "KGR"),
    BTL_AREA_C(jan, "JAN"),
    BTL_AREA_C(jan2, "JAN2"),
    BTL_AREA_C(kzn, "KZN"),
    BTL_AREA_DMA(kzn2, "KZN2"),
    BTL_AREA_C(flo, "FLO"),
    BTL_AREA_C(flo2, "FLO2"),
    BTL_AREA_C(tik, "TIK"),
    BTL_AREA_C(tik2, "TIK2"),
    BTL_AREA_C(tik3, "TIK3"),
    BTL_AREA_C(sam, "SAM"),
    BTL_AREA_C(sam2, "SAM2"),
    BTL_AREA_C(pra, "PRA"),
    BTL_AREA_C(pra2, "PRA2"),
    BTL_AREA_C(pra3, "PRA3"),
    BTL_AREA_C(kpa, "KPA"),
    BTL_AREA_C(kpa2, "KPA2"),
    BTL_AREA_C(kpa3, "KPA3"),
    BTL_AREA_C(kpa4, "KPA4"),
    BTL_AREA_C(kkj, "KKJ"),
    BTL_AREA_C(dig, "DIG"),
    BTL_AREA_AUX(omo2_1, "OMO2 1"),
    BTL_AREA_AUX(omo2_2, "OMO2 2"),
    BTL_AREA_AUX(omo2_3, "OMO2 3"),
    BTL_AREA_AUX(omo2_4, "OMO2 4"),
    BTL_AREA_AUX(omo2_5, "OMO2 5"),
    BTL_AREA_AUX(omo2_6, "OMO2 6"),
};

void reset_battle_status(void) {
    gGameStatusPtr->demoBattleFlags = 0;
    gBattleState = BATTLE_STATE_NONE;
    gBattleSubState = BTL_SUBSTATE_INIT;
    gLastDrawBattleState = BATTLE_STATE_NONE;
    gCurrentBattlePtr = NULL;
    gCurrentBattleID = 0;
    gCurrentStagePtr = NULL;
    gCurrentStageID = 0;
    gOverrideBattlePtr = NULL;
}

void load_battle_section(void) {
    BattleArea* battleArea = &gBattleAreas[UNPACK_BTL_AREA(gCurrentBattleID)];
    s32 battleIdx = UNPACK_BTL_INDEX(gCurrentBattleID);

    dma_copy((u8*)battleArea->dmaStart, (u8*)battleArea->dmaEnd, (u8*)battleArea->dmaDest);

    gCurrentBattlePtr = &(*battleArea->battles)[battleIdx];

    if (gCurrentStageID < 0) {
        gCurrentStagePtr = NULL;
    } else {
        gCurrentStagePtr = &(*battleArea->stages)[gCurrentStageID];
    }

    btl_set_state(BATTLE_STATE_START);
    gLastDrawBattleState = BATTLE_STATE_NONE;
}

void load_battle(s32 battleID) {
    gCurrentBattleID = battleID;
    set_game_mode(GAME_MODE_BATTLE);
    gBattleState = BATTLE_STATE_NONE;
    gLastDrawBattleState = BATTLE_STATE_NONE;
    gBattleSubState = BTL_SUBSTATE_INIT;
}

void set_battle_stage(s32 stageID) {
    gCurrentStageID = stageID;
}

void set_battle_formation(Battle* battle) {
    gOverrideBattlePtr = battle;
}

void setup_demo_player(void) {
    PlayerData* playerData = &gPlayerData;
    s32 i;

    playerData->curHP = 15;
    playerData->curMaxHP = 15;
    playerData->hardMaxHP = 15;
    playerData->curFP = 10;
    playerData->curMaxFP = 10;
    playerData->hardMaxFP = 10;
    playerData->level = 3;
    playerData->hasActionCommands = true;
    playerData->starPoints = 55;
    playerData->bootsLevel = 0;
    playerData->hammerLevel = 0;
    playerData->coins = 34;

    for (i = 1; i < ARRAY_COUNT(playerData->partners); i++) {
        playerData->partners[i].enabled = true;
        playerData->partners[i].level = 2;
    }

    playerData->curPartner = PARTNER_GOOMBARIO;

    for (i = 0; i < ARRAY_COUNT(playerData->badges); i++) {
        playerData->badges[i] = ITEM_NONE;
    }

    for (i = 0; i < ARRAY_COUNT(playerData->equippedBadges); i++) {
        playerData->equippedBadges[i] = ITEM_NONE;
    }

    for (i = 0; i < ARRAY_COUNT(playerData->invItems); i++) {
        playerData->invItems[i] = ITEM_NONE;
    }

    playerData->unused_288 = 0;
    playerData->merleeSpellType = MERLEE_SPELL_NONE;
    playerData->merleeCastsLeft = 0;
    playerData->merleeTurnCount = 0;
    playerData->maxStarPower = 0;
    playerData->starPower = 0;
    playerData->starBeamLevel = 0;
}

void load_demo_battle(u32 index) {
    PlayerData* playerData = &gPlayerData;
    u32 mode;
    s32 battleID;

    gGameStatusPtr->demoBattleFlags = 0;
    gGameStatusPtr->areaID = 0;
    gGameStatusPtr->mapID = 0;
    gGameStatusPtr->context = CONTEXT_WORLD;

    general_heap_create();
    clear_worker_list();
    clear_script_list();
    create_cameras();
    spr_init_sprites(PLAYER_SPRITES_MARIO_WORLD);
    clear_animator_list();
    clear_entity_models();
    clear_npcs();
    hud_element_clear_cache();
    clear_trigger_data();
    clear_model_data();
    clear_sprite_shading_data();
    reset_background_settings();
    reset_back_screen_overlay_progress();
    reset_battle_status();
    clear_encounter_status();
    clear_entity_data(true);
    clear_effect_data();
    clear_player_status();
    clear_printers();
    clear_item_entity_data();
    clear_player_data();
    initialize_status_bar();
    clear_item_entity_data();
    set_screen_overlay_params_front(OVERLAY_TYPE_9, 255.0f);

    switch (index) {
        case 0: // hammer first strike on Fuzzies
            setup_demo_player();
            mode = 0;
            playerData->hasActionCommands = false;
            battleID = BTL_DIG_FORMATION_00;
            break;
        case 1: // jump on Monty Mole
            setup_demo_player();
            mode = 0;
            playerData->curPartner = PARTNER_BOW;
            battleID = BTL_DIG_FORMATION_01;
            break;
        case 2: // Parakarry shell shot against Pokey
            setup_demo_player();
            mode = 0;
            playerData->curPartner = PARTNER_PARAKARRY;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_PARTNER_ACTING;
            battleID = BTL_DIG_FORMATION_02;
            break;
        case 3: // Thunder Rage on Shy Guys at the slot machine
            setup_demo_player();
            mode = 0;
            playerData->curPartner = PARTNER_WATT;
            battleID = BTL_DIG_FORMATION_03;
            break;
        case 4: // stomped by Tubba Blubba
            setup_demo_player();
            mode = 0;
            playerData->curPartner = PARTNER_KOOPER;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_ENEMY_ACTING;
            battleID = BTL_DIG_FORMATION_04;
            break;
        default:
            setup_demo_player();
            mode = 2;
            battleID = BTL_DIG_FORMATION_00;
    }

    gGameStatusPtr->debugEnemyContact = DEBUG_CONTACT_NONE;
    gGameStatusPtr->healthBarsEnabled = true;

    switch (mode) {
        case 0:
            gCurrentEncounter.firstStrikeType = FIRST_STRIKE_NONE;
            gCurrentEncounter.hitType = ENCOUNTER_TRIGGER_NONE;
            gCurrentEncounter.hitTier = 0;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_10;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_20;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_40;
            break;
        case 1:
            gCurrentEncounter.firstStrikeType = FIRST_STRIKE_PLAYER;
            gCurrentEncounter.hitType = ENCOUNTER_TRIGGER_HAMMER;
            gCurrentEncounter.hitTier = playerData->hammerLevel;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_10;
            break;
        case 2:
            gCurrentEncounter.firstStrikeType = FIRST_STRIKE_PLAYER;
            gCurrentEncounter.hitType = ENCOUNTER_TRIGGER_JUMP;
            gCurrentEncounter.hitTier = playerData->bootsLevel;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_10;
            break;
        case 3:
            gCurrentEncounter.firstStrikeType = FIRST_STRIKE_PLAYER;
            gCurrentEncounter.hitType = ENCOUNTER_TRIGGER_PARTNER;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_20;
            break;
        case 4:
            gCurrentEncounter.firstStrikeType = FIRST_STRIKE_ENEMY;
            gCurrentEncounter.hitType = ENCOUNTER_TRIGGER_NONE;
            gCurrentEncounter.hitTier = 0;
            gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_40;
            break;
    }

    evt_set_variable(NULL, GF_Tutorial_SwapTurnOrder, 1);
    gCurrentEncounter.unk_07 = 0;
    gCurrentEncounter.instigatorValue = 0;
    set_battle_stage(BTL_STAGE_DEFAULT);
    gGameStatusPtr->demoBattleFlags |= DEMO_BTL_FLAG_ENABLED;
    gOverrideFlags &= ~GLOBAL_OVERRIDES_DISABLE_DRAW_FRAME;
    load_battle(battleID);
}

} // extern "C"
