#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "boss.hpp"
#include "koopa_gang_tower.hpp"
#include "train_heist_actors.hpp"
#include "sprite/npc/KoopaGang2.h"
#include "sprite/npc/ShyGuy.h"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace stage_guy {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Dummy;

enum ActorPartIDs {
    PRT_MAIN        = 1,
};

// Actor Stats
constexpr s32 hp = 50;

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,               0,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,               0,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,            0,
    STATUS_KEY_SHRINK,              0,
    STATUS_KEY_STOP,                0,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,          0,
    STATUS_TURN_MOD_FEAR,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,       0,
    STATUS_TURN_MOD_SHRINK,         0,
    STATUS_TURN_MOD_STOP,           0,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 24 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_ShyGuy_Black_Anim01,
    STATUS_END,
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_Dummy))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Dummy))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_Dummy))
    Return
    End
};

EvtScript EVS_Dummy = {
    Return
    End
};

}; // namespace stage_guy
namespace green_bandit_tower {

// these are the only parameters that vary among koopa gang actors
enum ThisBanditsParams {
    THIS_ACTOR_ID               = GREEN_ACTOR,
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

}; // namespace green_bandit_tower

namespace yellow_bandit_tower {

enum ThisBanditsParams {
    THIS_ACTOR_ID               = YELLOW_ACTOR,
    THIS_ANIM_IDLE              = ANIM_KoopaGang2_Yellow_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaGang2_Yellow_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang2_Yellow_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang2_Yellow_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang2_Yellow_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaGang2_Yellow_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaGang2_Yellow_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaGang2_Yellow_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaGang2_Yellow_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaGang2_Yellow_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaGang2_Yellow_StillToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaGang2_Yellow_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang2_Yellow_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang2_Yellow_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang2_Yellow_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang2_Yellow_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang2_Yellow_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang2_Yellow_PointForward,
};

#include "common_bandit_tower.inc.cpp"

}; // namespace yellow_bandit_tower

namespace black_bandit_tower {

// these are the only parameters that vary among koopa gang actors
enum ThisBanditsParams {
    THIS_ACTOR_ID               = BLACK_ACTOR,
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

}; // namespace black_bandit_tower

namespace red_bandit_tower {

// these are the only parameters that vary among koopa gang actors
enum ThisBanditsParams {
    THIS_ACTOR_ID               = RED_ACTOR,
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

}; // namespace red_bandit_tower

namespace koopa_gang {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_BeginPhase;
extern EvtScript EVS_TryFormingTower;
extern EvtScript EVS_BroadcastToKoopaBandits;
extern EvtScript EVS_ResetFormation;
extern EvtScript EVS_ResetFormation_Subscript_FallToPosition;
extern EvtScript EVS_ResetFormation_Subscript_PlayThumbsUpFX;
extern EvtScript EVS_DebugPrintKoopaGangPosition;
extern EvtScript EVS_MoveKoopaGangOffscreen;
extern EvtScript EVS_TowerUpdateStable;

enum ActorPartIDs {
    PRT_TOWER           = 1,
};

#include "common/StartRumbleWithParams.inc.c"

// Actor Stats
constexpr s32 hp = 1;

s32 TowerDefense[] = {
    ELEMENT_NORMAL,   1,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,            -1,
    STATUS_KEY_SLEEP,               0,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,               0,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,            0,
    STATUS_KEY_SHRINK,              0,
    STATUS_KEY_STOP,                0,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,          0,
    STATUS_TURN_MOD_FEAR,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,       0,
    STATUS_TURN_MOD_SHRINK,         0,
    STATUS_TURN_MOD_STOP,           0,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_TOWER,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = NULL,
        .defenseTable = TowerDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, 0)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_None)
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Toppled) // prevents first-turn tower attack
    // Call(SetActorVar, ACTOR_SELF, AVAR_Boss_BowserTaunts, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerHeight, 0)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_TOWER, ACTOR_PART_TARGET_NO_DAMAGE, true)
    Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_Scene_BowserPhase)
    Return
    End
};

API_CALLABLE((PlayLandOnTowerFX)) {
    Bytecode* args = script->ptrReadPos;
    s32 actorID = evt_get_variable(script, *args++);
    Actor* actor = get_actor(actorID);

    play_movement_dust_effects(2, actor->state.goalPos.x - 15.0f, actor->state.goalPos.y, actor->state.goalPos.z, actor->state.angle);
    return ApiStatus_DONE2;
}

// (in) Var0 : koopa gang actorID
// (in) Var1 : expected tower height
// (in) Var2 : tower index (height - 1, height - 2, ..., 0)
EvtScript EVS_BuildTowerWithKoopa = {
    DebugPrintf("Run:BOSS:EVS_BuildTowerWithKoopa\n")
    Call(UseIdleAnimation, LVar0, false)
    Switch(LVar0)
        CaseEq(RED_ACTOR)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_PointForward)
            Call(SetActorYaw, RED_ACTOR, 0)
            Call(GetActorPos, RED_ACTOR, LVar0, LVar1, LVar2)
            Sub(LVar0, 22)
            Add(LVar1, 19)
            Wait(10)
            Wait(30)
            Set(LVar0, 48)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Call(SetBattleCamTarget, LVar2, LVar3, LVar4)
            Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
            Call(SetBattleCamOffsetY, 50)
            Call(SetBattleCamDist, 400)
            Call(MoveBattleCamOver, LVar0)
            Wait(LVar0)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, RED_ACTOR, LVar2, LVar3, LVar4)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_Run)
            Call(GetGoalPos, RED_ACTOR, LVar0, LVar1, LVar2)
            Call(GetActorPos, RED_ACTOR, LVar3, LVar4, LVar5)
            IfLt(LVar0, LVar3)
                Call(SetActorYaw, RED_ACTOR, 0)
            Else
                Call(SetActorYaw, RED_ACTOR, 180)
            EndIf
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_Leap)
            Wait(5)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, RED_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar0, 54)
            Call(AddGoalPos, RED_ACTOR, 15, LVar0, -10)
            Thread
                Wait(10)
                Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_Land)
            EndThread
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_Midair)
            Call(SetActorJumpGravity, RED_ACTOR, Float(1.6))
            Call(JumpToGoal, RED_ACTOR, 20, false, false, false)
            Call((PlayLandOnTowerFX), RED_ACTOR)
            Call(PlaySoundAtActor, RED_ACTOR, SOUND_KOOPA_BROS_LAND)
            Call(GetActorPos, RED_ACTOR, LVar3, LVar4, LVar5)
            Sub(LVar3, 15)
            Call(SetActorPos, RED_ACTOR, LVar3, LVar4, LVar5)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_Idle)
            Wait(5)
            Call(SetActorYaw, RED_ACTOR, 0)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_PointForward)
            Call(PlaySoundAtActor, RED_ACTOR, SOUND_SMALL_LENS_FLARE)
            Call(GetActorPos, RED_ACTOR, LVar0, LVar1, LVar2)
            Sub(LVar0, 22)
            Add(LVar1, 19)
            PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 30, 0)
            Wait(20)
            Wait(10)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_Idle)
            Call(SetActorVar, RED_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosA)
        CaseEq(GREEN_ACTOR)
            Call(SetAnimation, LVarA, 1, ANIM_KoopaGang2_Green_IdleCrouch)
            Wait(50)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
                Set(LVar4, 15)
                Call(SetGoalPos, GREEN_ACTOR, LVar2, LVar3, LVar4)
                Call(SetAnimation, GREEN_ACTOR, 1, ANIM_KoopaGang2_Green_Run)
                Call(RunToGoal, GREEN_ACTOR, 10, false)
                Call(SetAnimation, GREEN_ACTOR, 1, ANIM_KoopaGang2_Green_IdleCrouch)
                Call(SetActorVar, GREEN_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosB)
        CaseEq(YELLOW_ACTOR)
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaGang2_Yellow_IdleCrouch)
            Wait(60)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, YELLOW_ACTOR, LVar2, LVar3, LVar4)
            Call(AddGoalPos, YELLOW_ACTOR, 0, 18, -4)
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaGang2_Yellow_Leap)
            Wait(5)
            Call(AddGoalPos, YELLOW_ACTOR, 15, 0, 0)
            Thread
                Wait(10)
                Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaGang2_Yellow_Land)
            EndThread
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaGang2_Yellow_Midair)
            Call(SetActorJumpGravity, YELLOW_ACTOR, Float(1.6))
            Call(JumpToGoal, YELLOW_ACTOR, 20, false, false, false)
            Call((PlayLandOnTowerFX), YELLOW_ACTOR)
            Call(PlaySoundAtActor, YELLOW_ACTOR, SOUND_KOOPA_BROS_LAND)
            Call(GetActorPos, YELLOW_ACTOR, LVar3, LVar4, LVar5)
            Sub(LVar3, 15)
            Call(SetActorPos, YELLOW_ACTOR, LVar3, LVar4, LVar5)
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaGang2_Yellow_IdleCrouch)
            Call(SetActorVar, YELLOW_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosC)
        CaseEq(BLACK_ACTOR)
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaGang2_Black_IdleCrouch)
            Wait(80)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, BLACK_ACTOR, LVar2, LVar3, LVar4)
            Call(AddGoalPos, BLACK_ACTOR, 0, 36, -7)
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaGang2_Black_Leap)
            Wait(5)
            Call(AddGoalPos, BLACK_ACTOR, 15, 0, 0)
            Thread
                Wait(10)
                Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaGang2_Black_Land)
            EndThread
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaGang2_Black_Midair)
            Call(SetActorJumpGravity, BLACK_ACTOR, Float(1.6))
            Call(JumpToGoal, BLACK_ACTOR, 20, false, false, false)
            Call((PlayLandOnTowerFX), BLACK_ACTOR)
            Call(PlaySoundAtActor, BLACK_ACTOR, SOUND_KOOPA_BROS_LAND)
            Call(GetActorPos, BLACK_ACTOR, LVar3, LVar4, LVar5)
            Sub(LVar3, 15)
            Call(SetActorPos, BLACK_ACTOR, LVar3, LVar4, LVar5)
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaGang2_Black_IdleCrouch)
            Call(SetActorVar, BLACK_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosD)
    EndSwitch
    DebugPrintf("Exit:BOSS:EVS_BuildTowerWithKoopa\n")
    Return
    End
};

EvtScript EVS_TryFormingTower = {
    DebugPrintf("Run:BOSS:EVS_TryFormingTower\n")
    #define LBL_WAIT_FOR_TOWER 0
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerHeight, 4)
    // exit if not ready
    Call(GetActorVar, GREEN_ACTOR, AVAR_Koopa_State, LVar0)
    IfEq(LVar0, AVAL_Koopa_State_Ready)
        Set(LVar0, GREEN_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
        Set(LVar0, YELLOW_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
        Set(LVar0, BLACK_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
        Set(LVar0, RED_ACTOR)
        ExecGetTID(EVS_BuildTowerWithKoopa, LVar1)
    Else
        DebugPrintf("Exit:BOSS:EVS_TryFormingTower\n")
        Return
    EndIf
    // hide status bar
    Call(EnableBattleStatusBar, false)
    // Wait until tower state is stable
    Label(LBL_WAIT_FOR_TOWER)
    IsThreadRunning(LVar1, LVar0)
        IfEq(LVar0, true)
            Wait(1)
            Goto(LBL_WAIT_FOR_TOWER)
        EndIf
    ExecWait(EVS_TowerUpdateStable)
    // show status bar
    Call(EnableBattleStatusBar, true)
    DebugPrintf("Exit:BOSS:EVS_TryFormingTower\n")
    Return
    End
    #undef LBL_WAIT_FOR_TOWER
};

EvtScript EVS_TowerUpdateStable = {
    DebugPrintf("Run:BOSS:EVS_TowerUpdateStable\n")
    // set script owner boss actor
    Call(SetOwnerID, BOSS_ACTOR)
    // exit if already stable
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    IfEq(LVar0, AVAL_Boss_TowerState_Stable)
        DebugPrintf("Exit:BOSS:EVS_TowerUpdateStable\n")
        Return
    EndIf
    // Finalize appearance/state of tower actor
    Call(SetPartFlagBits, ACTOR_SELF, PRT_TOWER, ACTOR_PART_FLAG_NO_TARGET, false)
    // reset tower offsets
    Set(LVar0, 68) // Was 92
    Call(SetTargetOffset, ACTOR_SELF, PRT_TOWER, -5, LVar0) // Was 36
    Call(SetActorSize, ACTOR_SELF, LVar0, 45)
    Call(GetActorPos, GREEN_ACTOR, LVar2, LVar3, LVar4)
    Call(SetActorPos, ACTOR_SELF, LVar2, LVar3, LVar4)
    // reset koopa state
    Set(LVarA, BOSS_CMD_STABLE)
    ExecWait(EVS_BroadcastToKoopaBandits)
    // reset tower state
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Stable)
    DebugPrintf("Exit:BOSS:EVS_TowerUpdateStable\n")
    Return
    End
};

EvtScript EVS_Broadcast_TowerUnstable = {
    DebugPrintf("Run:BOSS:EVS_Broadcast_TowerUnstable\n")
    // exit if already unstable
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    IfEq(LVar0, AVAL_Boss_TowerState_Unstable)
        DebugPrintf("Exit:BOSS:EVS_Broadcast_TowerUnstable\n")
        Return
    EndIf
    // update tower state
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Unstable)
    Set(LVarA, BOSS_CMD_UNSTABLE)
    ExecWait(EVS_BroadcastToKoopaBandits)
    // update tower offsets
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerHeight, LVarA)
    Set(LVar0, 79) // Was 92
    Call(SetTargetOffset, ACTOR_SELF, PRT_TOWER, -5, LVar0) // Was 23
    Call(SetActorSize, ACTOR_SELF, LVar0, 45)
    DebugPrintf("Exit:BOSS:EVS_Broadcast_TowerUnstable\n")
    Return
    End
};

EvtScript EVS_Broadcast_ToppleHit = {
    DebugPrintf("Run:BOSS:EVS_Broadcast_ToppleHit\n")
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Toppled)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    IfNe(LVar0, EVENT_BURN_HIT)
        Set(LVarA, BOSS_CMD_TOPPLE_HIT)
    Else
        Set(LVarA, BOSS_CMD_TOPPLE_BURN_HIT)
    EndIf
    ExecWait(EVS_BroadcastToKoopaBandits)
    Wait(20)
    // move gang offstage
    ExecWait(EVS_MoveKoopaGangOffscreen)
    // update tower flag bits
    Call(SetPartFlagBits, ACTOR_SELF, PRT_TOWER, ACTOR_PART_FLAG_NO_TARGET, true)
    DebugPrintf("Exit:BOSS:EVS_Broadcast_ToppleHit\n")
    Return
    End
};

EvtScript EVS_MoveKoopaGangOffscreen = {
    DebugPrintf("Run:BOSS:EVS_MoveKoopaGangOffscreen\n")
    // adjust camera
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(SetBattleCamTarget, 70, 0, 0)
    Call(SetBattleCamDist, 350)
    Call(SetBattleCamOffsetY, 40)
    Call(MoveBattleCamOver, 15)
    Wait(15)
    // stage guy actions
    // jump in from offscreen
    Call(SetActorPos, ACTOR_STAGE_GUY, 200, 0, 0)
    Call(SetPartFlagBits, ACTOR_STAGE_GUY, 1, ACTOR_PART_FLAG_INVISIBLE, false)
    Call(UseIdleAnimation, ACTOR_STAGE_GUY, false)
    Call(SetAnimation, ACTOR_STAGE_GUY, 1, ANIM_ShyGuy_Black_Anim0A)
    Call(SetActorJumpGravity, ACTOR_STAGE_GUY, Float(1.6))
    Call(SetGoalPos, ACTOR_STAGE_GUY, 150, 0, 0)
    Call(JumpToGoal, ACTOR_STAGE_GUY, 15, false, false, false)
    // run to front of koopa gang
    Call(SetAnimation, ACTOR_STAGE_GUY, 1, ANIM_ShyGuy_Black_Anim03)
    Call(SetActorSounds, ACTOR_STAGE_GUY, ACTOR_SOUND_WALK, SOUND_NONE, SOUND_NONE)
    Call(SetGoalPos, ACTOR_STAGE_GUY, 0, 0, 0)
    Call(RunToGoal, ACTOR_STAGE_GUY, 20, false)
    Call(SetActorYaw, ACTOR_STAGE_GUY, 180)
    Call(SetGoalPos, ACTOR_STAGE_GUY, 0, 0, 15)
    Call(RunToGoal, ACTOR_STAGE_GUY, 5, false)
    // move stage guy offscreen
    Call(SetAnimation, ACTOR_STAGE_GUY, 1, ANIM_ShyGuy_Black_Anim02)
    Call(SetGoalPos, ACTOR_STAGE_GUY, 200, 0, 0)
    Thread
        Call(RunToGoal, ACTOR_STAGE_GUY, 60, false)
    EndThread
    // move koopa gang offscreen
    Call(SetGoalPos, GREEN_ACTOR, 200, 0, 5)
    Call(SetGoalPos, YELLOW_ACTOR, 200, 0, 10)
    Call(SetGoalPos, BLACK_ACTOR, 200, 0, 15)
    Call(SetGoalPos, RED_ACTOR, 200, 0, 20)
    Thread
        Call(RunToGoal, GREEN_ACTOR, 60, false)
    EndThread
    Wait(9)
    Thread
        Call(RunToGoal, YELLOW_ACTOR, 50, false)
    EndThread
    Wait(9)
    Thread
        Call(RunToGoal, BLACK_ACTOR, 40, false)
    EndThread
    Wait(9)
    Call(RunToGoal, RED_ACTOR, 30, false)
    // reset stage guy
    Call(ResetAllActorSounds, ACTOR_STAGE_GUY)
    Call(UseIdleAnimation, ACTOR_STAGE_GUY, true)
    Call(SetActorPos, ACTOR_STAGE_GUY, NPC_DISPOSE_LOCATION)
    Call(SetPartFlagBits, ACTOR_STAGE_GUY, 1, ACTOR_PART_FLAG_INVISIBLE, true)
    // update gang position
    Call(SetActorPos, GREEN_ACTOR, NPC_DISPOSE_LOCATION)
    Call(SetActorPos, YELLOW_ACTOR, NPC_DISPOSE_LOCATION)
    Call(SetActorPos, BLACK_ACTOR, NPC_DISPOSE_LOCATION)
    Call(SetActorPos, RED_ACTOR, NPC_DISPOSE_LOCATION)
    // update gang visible
    Call(SetPartFlagBits, GREEN_ACTOR, 1, ACTOR_PART_FLAG_INVISIBLE, true)
    Call(SetPartFlagBits, YELLOW_ACTOR, 1, ACTOR_PART_FLAG_INVISIBLE, true)
    Call(SetPartFlagBits, BLACK_ACTOR, 1, ACTOR_PART_FLAG_INVISIBLE, true)
    Call(SetPartFlagBits, RED_ACTOR, 1, ACTOR_PART_FLAG_INVISIBLE, true)
    DebugPrintf("Exit:BOSS:EVS_MoveKoopaGangOffscreen\n")
    Return
    End
};

// (in) LVarA : event
EvtScript EVS_BroadcastToKoopaBandits = {
    Exec(green_bandit_tower::EVS_HandleCommand)
    Exec(yellow_bandit_tower::EVS_HandleCommand)
    Exec(black_bandit_tower::EVS_HandleCommand)
    ExecGetTID(red_bandit_tower::EVS_HandleCommand, LVar1)
    Label(0)
        IsThreadRunning(LVar1, LVar0)
        IfEq(LVar0, true)
            Wait(1)
            Goto(0)
        EndIf
    Return
    End
};

// adds a random jitter to KoopaGang actors' X position while the tower is unstable
EvtScript EVS_Idle = {
    Label(0)
        Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
        IfNe(LVar0, AVAL_Boss_TowerState_Unstable)
            Wait(1)
            Goto(0)
        EndIf
        // get initial actor positions
        Call(GetActorPos, GREEN_ACTOR, LVarA, LVarE, LVarF)
        Call(GetActorPos, YELLOW_ACTOR, LVarC, LVarE, LVarF)
        Call(GetActorPos, BLACK_ACTOR, LVarD, LVarE, LVarF)
        Call(GetActorPos, RED_ACTOR, LVarB, LVarE, LVarF)
        // while tower is unstable, add random X offsets to koopa gang
        Label(1)
            // green
            Call(RandInt, 2, LVar0)
            Set(LVar1, 1)
            Sub(LVar0, LVar1)
            Add(LVar0, LVarA)
            Call(GetActorPos, GREEN_ACTOR, LVar1, LVar2, LVar3)
            Call(SetActorPos, GREEN_ACTOR, LVar0, LVar2, LVar3)
            // yellow
            Call(RandInt, 2, LVar0)
            Set(LVar1, 1)
            Sub(LVar0, LVar1)
            Add(LVar0, LVarC)
            Call(GetActorPos, YELLOW_ACTOR, LVar1, LVar2, LVar3)
            Call(SetActorPos, YELLOW_ACTOR, LVar0, LVar2, LVar3)
            // black
            Call(RandInt, 2, LVar0)
            Set(LVar1, 1)
            Sub(LVar0, LVar1)
            Add(LVar0, LVarD)
            Call(GetActorPos, BLACK_ACTOR, LVar1, LVar2, LVar3)
            Call(SetActorPos, BLACK_ACTOR, LVar0, LVar2, LVar3)
            // red
            Call(RandInt, 2, LVar0)
            Set(LVar1, 1)
            Sub(LVar0, LVar1)
            Add(LVar0, LVarB)
            Call(GetActorPos, RED_ACTOR, LVar1, LVar2, LVar3)
            Call(SetActorPos, RED_ACTOR, LVar0, LVar2, LVar3)
            // check tower state
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfEq(LVar0, AVAL_Boss_TowerState_Unstable)
                Wait(2)
                Goto(1)
            EndIf
        // once tower regains stability, reset X positions to initial values
        Call(GetActorPos, GREEN_ACTOR, LVar1, LVar2, LVar3)
        Call(SetActorPos, GREEN_ACTOR, LVarA, LVar2, LVar3)
        Call(GetActorPos, YELLOW_ACTOR, LVar1, LVar2, LVar3)
        Call(SetActorPos, YELLOW_ACTOR, LVarC, LVar2, LVar3)
        Call(GetActorPos, BLACK_ACTOR, LVar1, LVar2, LVar3)
        Call(SetActorPos, BLACK_ACTOR, LVarD, LVar2, LVar3)
        Call(GetActorPos, RED_ACTOR, LVar1, LVar2, LVar3)
        Call(SetActorPos, RED_ACTOR, LVarB, LVar2, LVar3)
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_HIT_COMBO)
            // set flags for player or partner hitting the koopa bros tower
            Call(GetBattleFlags, LVar0)
            IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Else
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            Set(LVarA, BOSS_CMD_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Wait(30)
            // if the attack was explosive, set both flags
            Call(GetLastElement, LVar0)
            IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            ExecWait(EVS_Broadcast_TowerUnstable)
        CaseEq(EVENT_HIT)
            // set flags for player or partner hitting the koopa bros tower
            Call(GetBattleFlags, LVar0)
            IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Else
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if the attack was explosive, set both flags
            Call(GetLastElement, LVar0)
            IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if this was the second hit, topple the tower
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            IfFlag(LVar0, AFLAG_Boss_PlayerHitTower)
                IfFlag(LVar0, AFLAG_Boss_PartnerHitTower)
                    ExecWait(EVS_Broadcast_ToppleHit)
                    Wait(20)
                    Call(UseIdleAnimation, ACTOR_SELF, true)
                    Return
                EndIf
            EndIf
            // this was the first hit
            Set(LVarA, BOSS_CMD_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Wait(30)
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfNe(LVar0, AVAL_Boss_TowerState_Unstable)
                ExecWait(EVS_Broadcast_TowerUnstable)
            EndIf
        CaseEq(EVENT_BURN_HIT)
            // set flags for player or partner hitting the koopa bros tower
            Call(GetBattleFlags, LVar0)
            IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Else
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if the attack was explosive, set both flags
            Call(GetLastElement, LVar0)
            IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if this was the second hit, topple the tower
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            IfFlag(LVar0, AFLAG_Boss_PlayerHitTower)
                IfFlag(LVar0, AFLAG_Boss_PartnerHitTower)
                    ExecWait(EVS_Broadcast_ToppleHit)
                    Wait(20)
                    Call(UseIdleAnimation, ACTOR_SELF, true)
                    Return
                EndIf
            EndIf
            // this was the first hit
            Set(LVarA, BOSS_CMD_BURN_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Wait(30)
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfNe(LVar0, AVAL_Boss_TowerState_Unstable)
                ExecWait(EVS_Broadcast_TowerUnstable)
            EndIf
        CaseEq(EVENT_ZERO_DAMAGE)
            Set(LVarA, BOSS_CMD_NO_DAMAGE_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
        CaseEq(EVENT_IMMUNE)
            // set both flags if the tower is already unstable
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfEq(LVar0, AVAL_Boss_TowerState_Unstable)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            IfFlag(LVar0, AFLAG_Boss_PlayerHitTower)
                IfFlag(LVar0, AFLAG_Boss_PartnerHitTower)
                    ExecWait(EVS_Broadcast_ToppleHit)
                    Wait(20)
                    Call(UseIdleAnimation, ACTOR_SELF, true)
                    Return
                EndIf
            EndIf
            // this was the first hit
            Set(LVarA, BOSS_CMD_NO_DAMAGE_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
        CaseOrEq(EVENT_DEATH)
        CaseOrEq(EVENT_BURN_DEATH)
        EndCaseGroup
        CaseDefault
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_DebugPrintKoopaGangPosition = {
    Call(GetActorPos, BOSS_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Tower Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, GREEN_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Green Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, YELLOW_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Yellow Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, BLACK_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Black Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, RED_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Red Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    DebugPrintf("Run:BOSS:EVS_TakeTurn\n")
    Call(UseIdleAnimation, ACTOR_SELF, false)
    // Reform stable tower if tipping
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    Switch(LVar0)
        CaseEq(AVAL_Boss_TowerState_Stable)
            DebugPrintf("Exit:BOSS:EVS_TakeTurn\n")
            Return
        CaseEq(AVAL_Boss_TowerState_Unstable)
            Wait(30)
            // stabilize tower
            ExecWait(EVS_TowerUpdateStable)
            DebugPrintf("Exit:BOSS:EVS_TakeTurn\n")
            Return
    EndSwitch
    // Check if ready
    Set(LVarA, BOSS_CMD_TRY_GET_UP)
    ExecWait(EVS_BroadcastToKoopaBandits)
    Wait(5)
    // Reset formation if ready
    Call(GetActorVar, GREEN_ACTOR, AVAR_Koopa_State, LVar0)
    IfEq(LVar0, AVAL_Koopa_State_Ready)
        ExecWait(EVS_ResetFormation)
    EndIf
    // Try tower operations
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    IfNe(LVar0, AVAL_Boss_TowerState_Stable)
        ExecWait(EVS_TryFormingTower)
    EndIf
    DebugPrintf("Exit:BOSS:EVS_TakeTurn\n")
    Return
    End
};

// (in) Var0 : actor id
// (in) Var1 : x pos
// (in) Var2 : z pos
// (in) Var3 : fall anim
// (in) Var4 : land anim
EvtScript EVS_ResetFormation_Subscript_FallToPosition = {
    DebugPrintf("Run:BOSS:EVS_ResetFormation_Subscript_FallToPosition\n")
    Call(SetAnimation, LVar0, 1, LVar3)
    Call(SetActorPos, LVar0, LVar1, 250, LVar2)
    Call(SetGoalPos, LVar0, LVar1, 0, LVar2)
    Call(SetActorJumpGravity, LVar0, Float(1.5))
    Call(SetActorSounds, LVar0, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
    Call(FallToGoal, LVar0, 30)
    Call(PlaySoundAtActor, LVar0, SOUND_KOOPA_BROS_LAND)
    Call(ResetActorSounds, LVar0, ACTOR_SOUND_JUMP)
    Call(ForceHomePos, LVar0, LVar1, 0, LVar2)
    Call(SetAnimation, LVar0, 1, LVar4)
    DebugPrintf("Exit:BOSS:EVS_ResetFormation_Subscript_FallToPosition\n")
    Return
    End
};

// (in) Var0 : actor id
EvtScript EVS_ResetFormation_Subscript_PlayThumbsUpFX = {
    DebugPrintf("Run:BOSS:EVS_ResetFormation_Subscript_PlayThumbsUpFX\n")
    Call(GetActorPos, LVar0, LVarA, LVarB, LVarC)
    Add(LVarA, 7)
    Add(LVarB, 28)
    Add(LVarC, 5)
    PlayEffect(EFFECT_LENS_FLARE, 0, LVarA, LVarB, LVarC, 30, 0)
    Call(PlaySoundAtActor, LVar0, SOUND_SMALL_LENS_FLARE)
    DebugPrintf("Exit:BOSS:EVS_ResetFormation_Subscript_PlayThumbsUpFX\n")
    Return
    End
};

EvtScript EVS_ResetFormation = {
    DebugPrintf("Run:BOSS:EVS_ResetFormation\n")
    Thread  // play fall sounds
        Wait(23)
        Call(PlaySoundAtActor, GREEN_ACTOR, SOUND_FALL_QUICK)
        Wait(5)
        Call(PlaySoundAtActor, YELLOW_ACTOR, SOUND_FALL_QUICK)
        Wait(5)
        Call(PlaySoundAtActor, BLACK_ACTOR, SOUND_FALL_QUICK)
        Wait(5)
        Call(PlaySoundAtActor, RED_ACTOR, SOUND_FALL_QUICK)
    EndThread
    Thread  // land from offscreen
        Set(LVar0, GREEN_ACTOR)
        Set(LVar1, 30)
        Set(LVar2, 5)
        Set(LVar3, ANIM_KoopaGang2_Green_Land)
        Set(LVar4, ANIM_KoopaGang2_Green_IdleCrouch)
        Exec(EVS_ResetFormation_Subscript_FallToPosition)
        Wait(5)
        Set(LVar0, YELLOW_ACTOR)
        Set(LVar1, 60)
        Set(LVar2, 10)
        Set(LVar3, ANIM_KoopaGang2_Yellow_Land)
        Set(LVar4, ANIM_KoopaGang2_Yellow_IdleCrouch)
        Exec(EVS_ResetFormation_Subscript_FallToPosition)
        Wait(5)
        Set(LVar0, BLACK_ACTOR)
        Set(LVar1, 90)
        Set(LVar2, 15)
        Set(LVar3, ANIM_KoopaGang2_Black_Land)
        Set(LVar4, ANIM_KoopaGang2_Black_IdleCrouch)
        Exec(EVS_ResetFormation_Subscript_FallToPosition)
        Wait(5)
        Set(LVar0, RED_ACTOR)
        Set(LVar1, 120)
        Set(LVar2, 20)
        Set(LVar3, ANIM_KoopaGang2_Red_Land)
        Set(LVar4, ANIM_KoopaGang2_Red_IdleCrouch)
        Exec(EVS_ResetFormation_Subscript_FallToPosition)
    EndThread
    Wait(30)
    // update camera
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(SetBattleCamTarget, 70, 46, 0)
    Call(SetBattleCamOffsetY, 0)
    Call(SetBattleCamDist, 292)
    Call(MoveBattleCamOver, 20)
    Wait(28)
    // do thumbs up
    // green
    Call(SetAnimation, GREEN_ACTOR, 1, ANIM_KoopaGang2_Green_ThumbsUp)
    Wait(5)
    Set(LVar0, GREEN_ACTOR)
    Exec(EVS_ResetFormation_Subscript_PlayThumbsUpFX)
    // yellow
    Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaGang2_Yellow_ThumbsUp)
    Wait(5)
    Set(LVar0, YELLOW_ACTOR)
    Exec(EVS_ResetFormation_Subscript_PlayThumbsUpFX)
    // black
    Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaGang2_Black_ThumbsUp)
    Wait(5)
    Set(LVar0, BLACK_ACTOR)
    Exec(EVS_ResetFormation_Subscript_PlayThumbsUpFX)
    // red
    Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaGang2_Red_ThumbsUp)
    Wait(5)
    Set(LVar0, RED_ACTOR)
    Exec(EVS_ResetFormation_Subscript_PlayThumbsUpFX)
    // wait for completion
    Wait(30)
    DebugPrintf("Exit:BOSS:EVS_ResetFormation\n")
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(GetEnemyMaxHP, ACTOR_SELF, LVar0)
    Call(SetEnemyHP, ACTOR_SELF, LVar0)
    Call(GetBattlePhase, LVar0)
    Switch(LVar0)
        CaseEq(PHASE_PLAYER_BEGIN)
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            BitwiseAndConst(LVar0, ~AFLAG_Boss_PlayerHitTower)
            Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            BitwiseAndConst(LVar0, ~AFLAG_Boss_PartnerHitTower)
            Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
        CaseEq(PHASE_ENEMY_BEGIN)
        CaseEq(PHASE_ENEMY_END)
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

}; // namespace koopa_gang

ActorBlueprint KoopaGang = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_SHADOW | ACTOR_FLAG_NO_DMG_APPLY | ACTOR_FLAG_NO_DMG_POPUP,
    .maxHP = koopa_gang::hp,
    .type = ACTOR_TYPE_KOOPA_GANG,
    .level = ACTOR_LEVEL_KOOPA_GANG,
    .partCount = ARRAY_COUNT(koopa_gang::ActorParts),
    .partsData = koopa_gang::ActorParts,
    .initScript = &koopa_gang::EVS_Init,
    .statusTable = koopa_gang::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 4,
    .powerBounceChance = 90,
    .coinReward = 0,
    .size = { 92, 45 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint GreenBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = green_bandit_tower::hp,
    .type = ACTOR_TYPE_GREEN_BANDIT,
    .level = ACTOR_LEVEL_GREEN_BANDIT,
    .partCount = ARRAY_COUNT(green_bandit_tower::ActorParts),
    .partsData = green_bandit_tower::ActorParts,
    .initScript = &green_bandit_tower::EVS_Init,
    .statusTable = green_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint YellowBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = yellow_bandit_tower::hp,
    .type = ACTOR_TYPE_YELLOW_BANDIT,
    .level = ACTOR_LEVEL_YELLOW_BANDIT,
    .partCount = ARRAY_COUNT(yellow_bandit_tower::ActorParts),
    .partsData = yellow_bandit_tower::ActorParts,
    .initScript = &yellow_bandit_tower::EVS_Init,
    .statusTable = yellow_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint BlackBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = black_bandit_tower::hp,
    .type = ACTOR_TYPE_BLACK_BANDIT,
    .level = ACTOR_LEVEL_BLACK_BANDIT,
    .partCount = ARRAY_COUNT(black_bandit_tower::ActorParts),
    .partsData = black_bandit_tower::ActorParts,
    .initScript = &black_bandit_tower::EVS_Init,
    .statusTable = black_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint RedBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = red_bandit_tower::hp,
    .type = ACTOR_TYPE_RED_BANDIT,
    .level = ACTOR_LEVEL_RED_BANDIT,
    .partCount = ARRAY_COUNT(red_bandit_tower::ActorParts),
    .partsData = red_bandit_tower::ActorParts,
    .initScript = &red_bandit_tower::EVS_Init,
    .statusTable = red_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint StageGuy = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR,
    .maxHP = stage_guy::hp,
    .type = ACTOR_TYPE_STAGE_GUY,
    .level = ACTOR_LEVEL_STAGE_GUY,
    .partCount = ARRAY_COUNT(stage_guy::ActorParts),
    .partsData = stage_guy::ActorParts,
    .initScript = &stage_guy::EVS_Init,
    .statusTable = stage_guy::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 75,
    .coinReward = 0,
    .size = { 30, 30 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
