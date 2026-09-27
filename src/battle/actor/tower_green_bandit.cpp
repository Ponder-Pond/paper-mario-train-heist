#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "boss.hpp"
#include "koopa_gang_tower.hpp"
#include "sprite/npc/KoopaGang2.h"
#include "dx/debug_menu.h"

enum GreenBanditsParams {
    THIS_ACTOR_ID               = GREEN_ACTOR,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_GREEN_BANDIT,
    THIS_ACTOR_LEVEL            = ACTOR_LEVEL_GREEN_BANDIT,
    THIS_ANIM_IDLE              = ANIM_KoopaGang2_Green_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaGang2_Green_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang2_Green_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang2_Green_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang2_Green_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaGang2_Green_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaGang2_Green_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaGang2_Green_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaGang2_Green_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaGang2_Green_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaGang2_Green_StillToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaGang2_Green_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang2_Green_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang2_Green_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang2_Green_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang2_Green_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang2_Green_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang2_Green_PointForward,
};

#include "common_bandit_tower.inc.cpp"
