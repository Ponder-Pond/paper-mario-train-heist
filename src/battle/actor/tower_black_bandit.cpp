#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "boss.hpp"
#include "koopa_gang_tower.hpp"
#include "sprite/npc/KoopaGang2.h"
#include "sprite/npc/ShyGuy.h"
#include "dx/debug_menu.h"

enum BlackBanditsParams {
    THIS_ACTOR_ID               = BLACK_ACTOR,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_BLACK_BANDIT,
    THIS_ACTOR_LEVEL            = ACTOR_LEVEL_BLACK_BANDIT,
    THIS_ANIM_IDLE              = ANIM_KoopaGang2_Black_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaGang2_Black_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang2_Black_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang2_Black_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang2_Black_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaGang2_Black_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaGang2_Black_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaGang2_Black_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaGang2_Black_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaGang2_Black_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaGang2_Black_StillToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaGang2_Black_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang2_Black_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang2_Black_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang2_Black_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang2_Black_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang2_Black_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang2_Black_PointForward,
};

#include "common_bandit_tower.inc.cpp"
