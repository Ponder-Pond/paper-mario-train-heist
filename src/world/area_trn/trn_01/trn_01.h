/// @file trn_01.h
/// @brief  - Lobby

#include "common.h"
#include "generated.h"
#include "message_ids.h"
#include "map.h"

#include "../trn.h"
#include "mapfs/trn_01_shape.h"
#include "mapfs/trn_01_hit.h"

#include "sprite/npc/Toadsworth.h"
#include "sprite/npc/ToadMinister.h"
#include "sprite/npc/TayceT.h"
#include "sprite/npc/BigBubba.h"
#include "sprite/npc/Toad.h"
#include "sprite/npc/LuigiSleeping.h"
#include "sprite/npc/CalamityKammy.h"
#include "sprite/npc/ParadePeach.h"

#define TOAD_BADGE_COUNT 14 // Badge Shop

#define TAYCE_T_ITEM_COUNT 6 // Item Shop

namespace trn_01 {

enum {
    MF_PurchasedBadge   = MapFlag(0),
    MF_PurchasedItem    = MapFlag(1),
};

enum {
    NPC_Toadsworth      = 0,
    NPC_ToadMinister    = 1,
    NPC_Bubba           = 2,
    NPC_Luigi           = 3,
    NPC_CalamityKammy   = 4,
    NPC_Peach           = 5,
    NPC_TayceT          = 6,
    NPC_Toad            = 7,
};



extern EvtScript EVS_Main;
extern EvtScript EVS_StartTexPanners;
extern EvtScript EVS_TrainBounce;
extern NpcGroupList DefaultNPCs;
extern EvtScript EVS_NpcInit_Toad;
extern EvtScript EVS_NpcInteract_Toad;
extern EvtScript EVS_NpcInit_TayceT;
extern EvtScript EVS_NpcInteract_TayceT;
extern EvtScript EVS_Scene_BeginGame;

}; // namespace trn_01
