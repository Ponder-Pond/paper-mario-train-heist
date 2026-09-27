#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/HammerBrosSMB3.h"
#include "boss.hpp"
#include "dx/debug_menu.h"

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_HAMMER_1    = 2,
};

// Actor Stats
constexpr s32 hp = 3;
constexpr s32 dmgHammerThrow = 2;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HammerBrosSMB3_Yellow_Idle,
    STATUS_KEY_STONE,     ANIM_HammerBrosSMB3_Yellow_Still,
    STATUS_KEY_SLEEP,     ANIM_HammerBrosSMB3_Yellow_Sleep,
    STATUS_KEY_POISON,    ANIM_HammerBrosSMB3_Yellow_Idle,
    STATUS_KEY_STOP,      ANIM_HammerBrosSMB3_Yellow_Still,
    STATUS_KEY_STATIC,    ANIM_HammerBrosSMB3_Yellow_Idle,
    STATUS_KEY_PARALYZE,  ANIM_HammerBrosSMB3_Yellow_Still,
    STATUS_KEY_DIZZY,     ANIM_HammerBrosSMB3_Yellow_Dizzy,
    STATUS_KEY_UNUSED,    ANIM_HammerBrosSMB3_Yellow_Dizzy,
    STATUS_END,
};

s32 HammerAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HammerBrosSMB3_Hammer,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              50,
    STATUS_KEY_POISON,             60,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              100,
    STATUS_KEY_UNUSED,                0,
    STATUS_KEY_STATIC,             80,
    STATUS_KEY_PARALYZE,           70,
    STATUS_KEY_SHRINK,             70,
    STATUS_KEY_STOP,               80,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,         -1,
    STATUS_TURN_MOD_UNUSED,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,       0,
    STATUS_TURN_MOD_SHRINK,         0,
    STATUS_TURN_MOD_STOP,          -2,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET | ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -3, 35 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -3, -10 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_HAMMER_1,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = HammerAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

extern "C" export ActorBlueprint blueprint = {
    .flags = 0,
    .maxHP = hp,
    .type = ACTOR_TYPE_YELLOW_HAMMER_BROS,
    .level = ACTOR_LEVEL_YELLOW_HAMMER_BROS,
    .partCount = ARRAY_COUNT(ActorParts),
    .partsData = ActorParts,
    .initScript = &EVS_Init,
    .statusTable = StatusTable,
    .escapeChance = 30,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 70,
    .coinReward = 2,
    .size = { 28, 38 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 35 },
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, true)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, true)
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    //Call(HPBarToHome, ACTOR_SELF)
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Yellow HB Actor ID: (%d)\n", LVar9)
    Return
    End
};

EvtScript EVS_Idle = {
    Label(0)
        Call(GetStatusFlags, ACTOR_SELF, LVar0)
        Switch(LVar0)
            CaseFlag(STATUS_FLAG_SLEEP)
                Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -5, 15)
                Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 0)
        EndSwitch
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_BurnHurt)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Yellow_BurnStill)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_BurnHurt)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Yellow_BurnStill)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_BurnStill)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_ShellEnter)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfNotFlag(LVar0, STATUS_FLAGS_IMMOBILIZED)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Yellow_ShellExit)
                Wait(10)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Idle)
            ExecWait(EVS_Enemy_Recover)
        CaseEq(EVENT_SCARE_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Run)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_ScareAway)
            Return
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Run)
            ExecWait(EVS_Enemy_AirLift)
        CaseEq(EVENT_BLOW_AWAY)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfNotFlag(LVar0, STATUS_FLAGS_IMMOBILIZED)
                IfFlag(LVar0, STATUS_FLAG_SHRINK)
                    Call(SetActorDispOffset, ACTOR_SELF, -2, 1, 0)
                Else
                    Call(SetActorDispOffset, ACTOR_SELF, -5, 3, 0)
                EndIf
            EndIf
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Hurt)
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Yellow_Idle)
            ExecWait(EVS_Enemy_NoDamageHit)
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Attack_HammerThrow = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(GetBattlePhase, LVar0)
    IfEq(LVar0, PHASE_FIRST_STRIKE)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 1)
    EndIf
    Call(GetBattlePhase, LVar0)
    IfEq(LVar0, PHASE_FIRST_STRIKE)
        ChildThread
            Set(LVar0, 0)
            Label(0)
            Add(LVar0, 20)
            Mod(LVar0, 360)
            Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_1, 0, 0, LVar0)
            Wait(1)
            Goto(0)
        EndChildThread
        Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_1, 0, 50, 0)
        Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, false)
        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Yellow_Throw)
        Goto(100)
    EndIf
    ChildThread
        Set(LVar0, 0)
        Label(0)
        Add(LVar0, 20)
        Mod(LVar0, 360)
        Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_1, 0, 0, LVar0)
        Wait(1)
        Goto(0)
    EndChildThread
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 23)
    Add(LVar1, 23)
    Sub(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Yellow_Throw)
    Wait(8)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, false)
    Label(100)
    Thread
        Wait(15)
        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Yellow_Idle)
    EndThread
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfFlag(LVar0, STATUS_FLAG_SHRINK)
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_1, Float(0.4), Float(0.4), Float(0.4))
    Else
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_1, Float(1.0), Float(1.0), Float(1.0))
    EndIf
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Set(LVarA, LVar0)
            Call(SetPartSounds, ACTOR_SELF, PRT_HAMMER_1, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
            Call(PlaySoundAtPart, ACTOR_SELF, PRT_HAMMER_1, SOUND_HAMMER_BROS_THROW)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 20)
            Set(LVar1, 10)
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_HAMMER_1, Float(1.3))
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 15, true)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVarA, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Sub(LVar0, 40)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 15, true)
            Sub(LVar0, 30)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 10, true)
            Sub(LVar0, 20)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 5, true)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, true)
            Wait(15)
            Call(YieldTurn)
            Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
            Wait(20)
            Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, true)
            Return
        EndCaseGroup
    EndSwitch
    Call(SetPartSounds, ACTOR_SELF, PRT_HAMMER_1, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_HAMMER_1, SOUND_HAMMER_BROS_THROW)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_HAMMER_1, Float(1.3))
    Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 15, true)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, DMG_STATUS_KEY(STATUS_FLAG_SHRINK, 2, 100), dmgHammerThrow, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(GetPartOffset, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, 10)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 20, true)
            Sub(LVar0, 30)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 10, true)
            Sub(LVar0, 20)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 5, true)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, true)
            Wait(20)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};


EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, false)
    ExecWait(EVS_Attack_HammerThrow)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};
