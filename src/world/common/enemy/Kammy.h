#include "common.h"
#include "sprite/npc/WorldKammy.h"
#include "sprite/npc/CalamityKammy.h"

#define KAMMY_ANIMS \
{ \
    .idle   = ANIM_WorldKammy_Anim01, \
    .walk   = ANIM_WorldKammy_Anim02, \
    .run    = ANIM_WorldKammy_Anim03, \
    .chase  = ANIM_WorldKammy_Anim03, \
    .anim_4 = ANIM_WorldKammy_Anim01, \
    .anim_5 = ANIM_WorldKammy_Anim01, \
    .death  = ANIM_WorldKammy_Anim00, \
    .hit    = ANIM_WorldKammy_Anim00, \
    .anim_8 = ANIM_WorldKammy_Anim03, \
    .anim_9 = ANIM_WorldKammy_Anim03, \
    .anim_A = ANIM_WorldKammy_Anim03, \
    .anim_B = ANIM_WorldKammy_Anim03, \
    .anim_C = ANIM_WorldKammy_Anim03, \
    .anim_D = ANIM_WorldKammy_Anim03, \
    .anim_E = ANIM_WorldKammy_Anim03, \
    .anim_F = ANIM_WorldKammy_Anim03, \
}

#define CALAMITY_KAMMY_ANIMS \
{ \
    .idle   = ANIM_CalamityKammy_Idle, \
    .walk   = ANIM_CalamityKammy_Idle, \
    .run    = ANIM_CalamityKammy_Idle, \
    .chase  = ANIM_CalamityKammy_Idle, \
    .anim_4 = ANIM_CalamityKammy_Idle, \
    .anim_5 = ANIM_CalamityKammy_Idle, \
    .death  = ANIM_CalamityKammy_Idle, \
    .hit    = ANIM_CalamityKammy_Idle, \
    .anim_8 = ANIM_CalamityKammy_Idle, \
    .anim_9 = ANIM_CalamityKammy_Idle, \
    .anim_A = ANIM_CalamityKammy_Idle, \
    .anim_B = ANIM_CalamityKammy_Idle, \
    .anim_C = ANIM_CalamityKammy_Idle, \
    .anim_D = ANIM_CalamityKammy_Idle, \
    .anim_E = ANIM_CalamityKammy_Idle, \
    .anim_F = ANIM_CalamityKammy_Idle, \
}
