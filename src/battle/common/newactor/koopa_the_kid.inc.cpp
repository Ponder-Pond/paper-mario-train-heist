#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/KoopaTheKid.h"
#include "sprite/npc/KoopaGang.h"
#include "sprite/npc/KoopaGang2.h"
#include "sprite/npc/HammerBrosSMB3.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "battle/common/newactor/bandit_tower.inc.cpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace koopa_the_kid {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent_Default;
extern EvtScript EVS_HandleEvent_Hide;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_EnterHide;
extern EvtScript EVS_ExitHide;
extern EvtScript EVS_SetupInhale;
extern EvtScript EVS_SetupInhale_Subscript_SendKoopaToCar;
extern EvtScript EVS_Attack_ShootKoopaShells;
extern EvtScript EVS_Attack_ShootKoopaShells_Subscript_ShootOne;
extern EvtScript EVS_Attack_SteelyDrop;
extern EvtScript EVS_EjectKoopa;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_STEELY      = 2,
};

// Actor Stats
constexpr s32 hp = 30;
constexpr s32 dmgKoopaGangInhale = 1;
constexpr s32 dmgSteelyDrop = 1;

// IDLE ANIMATION TABLES
// bowser default idle anim
s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaTheKid_Idle,
    STATUS_END,
};
// bowser hidden idle anim
s32 HideAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaTheKid_ClownCarStill,
    STATUS_END,
};
// steely idle anim
s32 SteelyAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaTheKid_BigSteely,
    STATUS_END,
};

// DEFENSE TABLES
// default defense table
s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

// STATUS TABLES
// default status table
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
        .flags = ACTOR_PART_FLAG_IGNORE_BELOW_CHECK | ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -22, 80 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -8 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_STEELY,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = SteelyAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

#include "common/StartRumbleWithParams.inc.c"

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent_Default))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_StateHide, false)
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_CountKoopaGang, 0)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Return
    End
};

EvtScript EVS_HandleEvent_Default = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            SetConst(LVar2, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            SetConst(LVar2, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Idle)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Idle)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_HandleEvent_Hide = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            Exec(EVS_EjectKoopa)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            Exec(EVS_EjectKoopa)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            SetConst(LVar2, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            SetConst(LVar2, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_ClownCarStill)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_ClownCarStill)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_EjectKoopa = {
    DebugPrintf("Run:BOWSER:EVS_EjectKoopa\n")
    Call(GetActorVar, ACTOR_SELF, AVAR_BowserPhase_CountKoopaGang, LVar0)
    IfGt(LVar0, 0)
        Sub(LVar0, 1)
        Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_CountKoopaGang, LVar0)
    EndIf
    DebugPrintf("Exit:BOWSER:EVS_EjectKoopa\n")
    Return
    End
};

EvtScript EVS_TakeTurn = {
    DebugPrintf("Run:BOWSER:EVS_TakeTurn\n")
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    // check bowser state
    Call(GetActorVar, ACTOR_SELF, AVAR_BowserPhase_StateHide, LVar0)
    Switch(LVar0)
        CaseEq(false)
            // check tower state
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_TowerState, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Boss_TowerState_Stable)
                    // inhale tower
                    ExecWait(EVS_SetupInhale)
                CaseEq(AVAL_Boss_TowerState_Unstable)
                    // set tower stable
                    ExecWait(koopa_gang::EVS_TowerUpdateStable)
                    // inhale tower
                    ExecWait(EVS_SetupInhale)
                CaseDefault
                    // big steely
                    ExecWait(EVS_Attack_SteelyDrop)
            EndSwitch
        CaseEq(true)
            // spit attack
            ExecWait(EVS_Attack_ShootKoopaShells)
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    DebugPrintf("Exit:BOWSER:EVS_TakeTurn\n")
    Return
    End
};

EvtScript EVS_EnterHide = {
    DebugPrintf("Run:BOWSER:EVS_EnterHide\n")
    // play hide animation
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Hide)
    Wait(20)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_ClownCarStill)
    // offset actor position
    Call(SetActorDispOffset, ACTOR_SELF, -6, 24, 0)
    // update hide state
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_StateHide, true)
    // update idle animations
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(HideAnims))
    // update damage flags
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_POPUP, true)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_TARGET_NO_DAMAGE, true)
    // update handle event
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent_Hide))
    DebugPrintf("Exit:BOWSER:EVS_EnterHide\n")
    Return
    End
};

EvtScript EVS_ExitHide = {
    DebugPrintf("Run:BOWSER:EVS_ExitHide\n")
    // play show animation
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Hide)
    Wait(20)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Idle)
    // reset actor position
    Call(SetActorDispOffset, ACTOR_SELF, 0, 0, 0)
    // reset hide state
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_StateHide, false)
    // reset idle animations
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    // reset damage flags
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_POPUP, false)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_TARGET_NO_DAMAGE, false)
    // reset handle event
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent_Default))
    DebugPrintf("Exit:BOWSER:EVS_ExitHide\n")
    Return
    End
};

// (in) Var0 : x pos
// (in) Var1 : y pos
// (in) Var2 : z pos
// (in) Var3 : actor id
EvtScript EVS_SetupInhale_Subscript_SendKoopaToCar = {
    DebugPrintf("Run:BOWSER:EVS_SetupInhale_Subscript_SendKoopaToCar\n")
    Call(SetActorJumpGravity, LVar3, Float(1.6))
    Add(LVar1, 20)
    Call(SetGoalPos, LVar3, LVar0, LVar1, LVar2)
    Call(JumpToGoal, LVar3, 20, false, false, false)
    Call(SetActorPos, LVar3, NPC_DISPOSE_LOCATION)
    Call(SetActorVar, LVar3, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
    Call(SetActorVar, LVar3, AVAR_Koopa_ToppleTurns, 2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EAT_OR_DRINK)
    DebugPrintf("Run:BOWSER:EVS_SetupInhale_Subscript_SendKoopaToCar\n")
    Return
    End
};

EvtScript EVS_SetupInhale = {
    DebugPrintf("Run:BOWSER:EVS_SetupInhale\n")
    ExecWait(EVS_EnterHide)
    // koopa gang enter car
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Set(LVar3, RED_ACTOR)
    Exec(EVS_SetupInhale_Subscript_SendKoopaToCar)
    Wait(5)
    Set(LVar3, BLACK_ACTOR)
    Exec(EVS_SetupInhale_Subscript_SendKoopaToCar)
    Wait(5)
    Set(LVar3, YELLOW_ACTOR)
    Exec(EVS_SetupInhale_Subscript_SendKoopaToCar)
    Wait(5)
    Set(LVar3, GREEN_ACTOR)
    ExecWait(EVS_SetupInhale_Subscript_SendKoopaToCar)
    // update tower state
    Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Toppled)
    Call(SetPartFlagBits, BOSS_ACTOR, koopa_gang::PRT_TOWER, ACTOR_PART_FLAG_NO_TARGET, true)
    // update koopa count
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_CountKoopaGang, 4)
    DebugPrintf("Exit:BOWSER:EVS_SetupInhale\n")
    Return
    End
};

EvtScript EVS_Attack_ShootKoopaShells = {
    DebugPrintf("Run:BOWSER:EVS_Attack_ShootKoopaShells\n")
    // aim at mario
    Call(MakeLerp, 0, 135, 15, EASING_COS_FAST_OVERSHOOT)
    Loop(0)
        Call(UpdateLerp)
        Call(SetActorRotation, ACTOR_SELF, 0, 0, LVar0)
        Wait(1)
        IfEq(LVar1, 0)
            BreakLoop
        EndIf
    EndLoop
    Wait(10)
    // count koopas
    Call(GetActorVar, ACTOR_SELF, AVAR_BowserPhase_CountKoopaGang, LVar0)
    Switch(LVar0)
        CaseEq(1)
            Goto(1)
        CaseEq(2)
            Goto(2)
        CaseEq(3)
            Goto(3)
        CaseEq(4)
            Goto(4)
    EndSwitch
    // nothing to fire
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_MOLE_POP)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 10)
    PlayEffect(EFFECT_WALKING_DUST, 2, LVar0, LVar1, LVar2, 0, 0)
    Wait(20)
    Goto(0)
    // fire red
    Label(4)
    Set(LVar0, RED_ACTOR)
    Exec(EVS_Attack_ShootKoopaShells_Subscript_ShootOne)
    Wait(10)
    // fire black
    Label(3)
    Set(LVar0, BLACK_ACTOR)
    Exec(EVS_Attack_ShootKoopaShells_Subscript_ShootOne)
    Wait(10)
    // fire yellow
    Label(2)
    Set(LVar0, YELLOW_ACTOR)
    Exec(EVS_Attack_ShootKoopaShells_Subscript_ShootOne)
    Wait(10)
    // fire green
    Label(1)
    Set(LVar0, GREEN_ACTOR)
    ExecWait(EVS_Attack_ShootKoopaShells_Subscript_ShootOne)
    // reset bowser
    Label(0)
    Call(MakeLerp, 135, 0, 15, EASING_COS_IN_OUT)
    Loop(0)
        Call(UpdateLerp)
        Call(SetActorRotation, ACTOR_SELF, 0, 0, LVar0)
        Wait(1)
        IfEq(LVar1, 0)
            BreakLoop
        EndIf
    EndLoop
    // exit hide state
    ExecWait(EVS_ExitHide)
    // reset koopa count
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_CountKoopaGang, 0)
    DebugPrintf("Exit:BOWSER:EVS_Attack_ShootKoopaShells\n")
    Return
    End
};

// (in) Var0 : koopa gang actorID
EvtScript EVS_Attack_ShootKoopaShells_Subscript_ShootOne = {
    DebugPrintf("Run:BOWSER:EVS_Attack_ShootKoopaShells_Subscript_ShootOne\n")
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SPIT_OUT)
    Call(GetActorPos, ACTOR_SELF, LVar1, LVar2, LVar3)
    Call(SetActorPos, LVar0, LVar1, LVar2, LVar3)
    Call(SetTargetActor, LVar0, ACTOR_PLAYER)
    Call(SetGoalToTarget, LVar0)
    Call(SetActorSpeed, LVar0, Float(16.0))
    Call(FlyToGoal, LVar0, 0, 0, EASING_LINEAR)
    Wait(2)
    Call(EnemyDamageTarget, LVar0, LVarA, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgKoopaGangInhale, 0)
    Call(SetActorJumpGravity, LVar0, Float(1.0))
    Call(GetActorPos, LVar0, LVar1, LVar2, LVar3)
    Sub(LVar1, 100)
    Call(RandInt, 50, LVar4)
    Add(LVar2, LVar4)
    Call(SetGoalPos, LVar0, LVar1, LVar2, LVar3)
    Call(JumpToGoal, LVar0, 20, false, true, false)
    Call(SetActorPos, LVar0, NPC_DISPOSE_LOCATION)
    DebugPrintf("Exit:BOWSER:EVS_Attack_ShootKoopaShells_Subscript_ShootOne\n")
    Return
    End
};

EvtScript EVS_Attack_SteelyDrop = {
    DebugPrintf("Run:BOWSER:EVS_Attack_SteelyDrop\n")
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    ExecWait(EVS_EnterHide)
    Call(GetActorRotation, ACTOR_SELF, LVar3, LVar4, LVar5)
    Call(MakeLerp, 0, 180, 15, EASING_COS_FAST_OVERSHOOT)
    Loop(0)
        Call(UpdateLerp)
        SetF(LVar5, LVar0)
        Call(SetActorRotation, ACTOR_SELF, LVar3, LVar4, LVar5)
        Wait(1)
        IfEq(LVar1, 0)
            BreakLoop
        EndIf
    EndLoop
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar1, 10)
    Set(LVar2, 25)
    Call(SetPartPos, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2)
    Set(LVar1, 0)
    Set(LVar2, 25)
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_STEELY, Float(2.0))
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_STEELY, Float(4.0))
    Call(SetPartFlagBits, ACTOR_SELF, PRT_STEELY, ACTOR_PART_FLAG_INVISIBLE, false)
    Call(SetAnimation, ACTOR_SELF, PRT_STEELY, ANIM_KoopaTheKid_BigSteely)
    Set(LVar1, 0)
    Set(LVar2, 25)
    Call(FallPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, 8)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(2.0))
    EndThread
    Sub(LVar0, 30)
    Set(LVar2, 25)
    Call(JumpPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, 10, true)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(1.5))
    EndThread
    Sub(LVar0, 30)
    Set(LVar2, 25)
    Call(JumpPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, 8, true)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(1.0))
    EndThread
    Call(FallPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, 8)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.5))
    EndThread
    Call(GetActorPos, ACTOR_PARTNER, LVar0, LVar1, LVar2)
    Add(LVar0, 40)
    Set(LVar2, 25)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_STEELY, Float(4.0))
    Call(RunPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, false)
    Call(SetGoalToTarget, ACTOR_SELF)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, 0, SUPPRESS_EVENT_ALL, 0, dmgSteelyDrop, BS_FLAGS1_TRIGGER_EVENTS)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PARTNER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Set(LVar2, 25)
    Call(SetActorSpeed, ACTOR_SELF, Float(4.0))
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(RunPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, false)
    Call(SetGoalToTarget, ACTOR_SELF)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, 0, SUPPRESS_EVENT_ALL, 0, dmgSteelyDrop, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 30)
            Set(LVar2, 25)
            Call(SetActorSpeed, ACTOR_SELF, Float(4.0))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(RunPartTo, ACTOR_SELF, PRT_STEELY, LVar0, LVar1, LVar2, false)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_STEELY, ACTOR_PART_FLAG_INVISIBLE, true)
            Wait(10)
            Call(GetActorRotation, ACTOR_SELF, LVar3, LVar4, LVar5)
            Call(MakeLerp, 180, 0, 15, EASING_COS_FAST_OVERSHOOT)
            Loop(0)
                Call(UpdateLerp)
                SetF(LVar5, LVar0)
                Call(SetActorRotation, ACTOR_SELF, LVar3, LVar4, LVar5)
                Wait(1)
                IfEq(LVar1, 0)
                    BreakLoop
                EndIf
            EndLoop
            ExecWait(EVS_ExitHide)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    DebugPrintf("Exit:BOWSER:EVS_Attack_SteelyDrop\n")
    Return
    End
};

}; // namespace koopa_the_kid

ActorBlueprint KoopaTheKid = {
    .flags = ACTOR_FLAG_FLYING,
    .maxHP = koopa_the_kid::hp,
    .type = ACTOR_TYPE_KOOPA_THE_KID,
    .level = ACTOR_LEVEL_KOOPA_THE_KID,
    .partCount = ARRAY_COUNT(koopa_the_kid::ActorParts),
    .partsData = koopa_the_kid::ActorParts,
    .initScript = &koopa_the_kid::EVS_Init,
    .statusTable = koopa_the_kid::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 4,
    .powerBounceChance = 0,
    .coinReward = 0,
    .size = { 120, 120 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -40, 67 },
    .statusTextOffset = { 10, 60 },
};

}; // namespace battle::actor
