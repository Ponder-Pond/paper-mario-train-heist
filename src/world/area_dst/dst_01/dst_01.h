#include "common.h"
#include "generated.h"
#include "message_ids.h"
#include "map.h"
#include "../dst.h"

#include "mapfs/dst_01_shape.h"
#include "mapfs/dst_01_hit.h"

#include "sprite/npc/RussT.h"

namespace dst_01 {

enum {
    NPC_Pokey1,
    NPC_Bandit1,
    NPC_Pokey2,
    NPC_Bandit2,
    NPC_Cleft,
};

enum {
    MV_EnemiesDefeated = MapVar(0),
};

extern EvtScript EVS_Main;
extern EvtScript EVS_BindExitTriggers;
extern EvtScript EVS_TexPan;
extern EvtScript EVS_SetFoliage;
extern EvtScript EVS_MakeEntities;
extern EvtScript EVS_SpawnChest;

}; // namespace dst_01
