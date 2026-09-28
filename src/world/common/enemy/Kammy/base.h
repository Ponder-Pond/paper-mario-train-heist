#pragma once
#include "npc.h"
#include "sprite/npc/WorldKammy.h"
#include "sprite/npc/CalamityKammy.h"

#define KAMMY_ANIMS \
{ \
    .idle   = ANIM_WorldKammy_Idle, \
    .walk   = ANIM_WorldKammy_Walk, \
    .run    = ANIM_WorldKammy_Run, \
    .chase  = ANIM_WorldKammy_Run, \
    .alert  = ANIM_WorldKammy_Idle, \
    .unused = ANIM_WorldKammy_Idle, \
    .death  = ANIM_WorldKammy_Still, \
    .hit    = ANIM_WorldKammy_Still, \
    .anim_8 = ANIM_WorldKammy_Run, \
    .anim_9 = ANIM_WorldKammy_Run, \
    .anim_A = ANIM_WorldKammy_Run, \
    .anim_B = ANIM_WorldKammy_Run, \
    .anim_C = ANIM_WorldKammy_Run, \
    .anim_D = ANIM_WorldKammy_Run, \
    .anim_E = ANIM_WorldKammy_Run, \
    .anim_F = ANIM_WorldKammy_Run, \
}

#define CALAMITY_KAMMY_ANIMS \
{ \
    .idle   = ANIM_CalamityKammy_Idle, \
    .walk   = ANIM_CalamityKammy_Idle, \
    .run    = ANIM_CalamityKammy_Idle, \
    .chase  = ANIM_CalamityKammy_Idle, \
    .alert = ANIM_CalamityKammy_Idle, \
    .unused = ANIM_CalamityKammy_Idle, \
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
