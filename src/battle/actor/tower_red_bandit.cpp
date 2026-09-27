#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "boss.hpp"
#include "koopa_gang_tower.hpp"
#include "sprite/npc/KoopaGang2.h"
#include "sprite/npc/ShyGuy.h"
#include "dx/debug_menu.h"

enum RedBanditsParams {
    THIS_ACTOR_ID               = RED_ACTOR,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_RED_BANDIT,
    THIS_ACTOR_LEVEL            = ACTOR_LEVEL_RED_BANDIT,
    THIS_ANIM_IDLE              = ANIM_KoopaGang2_Red_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaGang2_Red_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang2_Red_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang2_Red_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang2_Red_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaGang2_Red_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaGang2_Red_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaGang2_Red_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaGang2_Red_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaGang2_Red_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaGang2_Red_StillToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaGang2_Red_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang2_Red_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang2_Red_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang2_Red_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang2_Red_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang2_Red_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang2_Red_PointForward,
};

#include "common_bandit_tower.inc.cpp"
