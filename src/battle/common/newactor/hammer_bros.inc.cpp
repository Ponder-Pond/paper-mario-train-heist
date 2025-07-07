#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/HammerBrosSMB3.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"

namespace battle::actor {

namespace green_hammer_bro {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_Move_HealSelf;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_HAMMER_1    = 2,
    PRT_HAMMER_2    = 3,
    PRT_HAMMER_3    = 4,
    PRT_HAMMER_4    = 5,
    PRT_HAMMER_5    = 6,
};

// Actor Stats
constexpr s32 hp = 12;
constexpr s32 dmgHammerSurge = 2;
constexpr s32 amtHeal = 10;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HammerBrosSMB3_Anim_02,
    STATUS_KEY_STONE,     ANIM_HammerBrosSMB3_Anim_00,
    STATUS_KEY_SLEEP,     ANIM_HammerBrosSMB3_Anim_11,
    STATUS_KEY_POISON,    ANIM_HammerBrosSMB3_Anim_02,
    STATUS_KEY_STOP,      ANIM_HammerBrosSMB3_Anim_00,
    STATUS_KEY_STATIC,    ANIM_HammerBrosSMB3_Anim_02,
    STATUS_KEY_PARALYZE,  ANIM_HammerBrosSMB3_Anim_00,
    STATUS_KEY_DIZZY,     ANIM_HammerBrosSMB3_Anim_12,
    STATUS_KEY_FEAR,      ANIM_HammerBrosSMB3_Anim_12,
    STATUS_END,
};

s32 HammerAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HammerBrosSMB3_Anim_0D,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   1,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              50,
    STATUS_KEY_POISON,             60,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              100,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,             80,
    STATUS_KEY_PARALYZE,           70,
    STATUS_KEY_SHRINK,             70,
    STATUS_KEY_STOP,               80,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,         -1,
    STATUS_TURN_MOD_FEAR,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,       0,
    STATUS_TURN_MOD_SHRINK,         0,
    STATUS_TURN_MOD_STOP,          -2,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
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
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_HAMMER_2,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = HammerAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_HAMMER_3,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = HammerAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_HAMMER_4,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = HammerAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_HAMMER_5,
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

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    //Call(HPBarToHome, ACTOR_SELF)
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
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0F)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Anim_10)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0F)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Anim_10)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_10)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_15)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfNotFlag(LVar0, STATUS_FLAGS_IMMOBILIZED)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_17)
                Wait(10)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_02)
            ExecWait(EVS_Enemy_Recover)
        CaseEq(EVENT_SCARE_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_07)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_ScareAway)
            Return
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_07)
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
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_0E)
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Anim_02)
            ExecWait(EVS_Enemy_NoDamageHit)
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_HammerSurge_Miss = {
    Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(2.0))
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_02)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_0C)
    Wait(4)
    Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Call(SetPartSounds, ACTOR_SELF, LVar0, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
    Call(PlaySoundAtPart, ACTOR_SELF, LVar0, SOUND_HAMMER_BROS_THROW)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar1, LVar2, LVar3)
    Set(LVar2, 10)
    Call(RandInt, 60, LVar4)
    Set(LVar5, 30)
    Sub(LVar5, LVar4)
    Add(LVar1, LVar5)
    Call(SetPartJumpGravity, ACTOR_SELF, LVar0, Float(1.3))
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 15, TRUE)
    Sub(LVar1, 50)
    Set(LVar2, 10)
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 20, TRUE)
    Sub(LVar1, 30)
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 10, TRUE)
    Sub(LVar1, 20)
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 5, TRUE)
    Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, TRUE)
    Return
    End
};

EvtScript EVS_HammerSurge_Hit = {
    Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(2.0))
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_02)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_0C)
    Wait(4)
    Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Call(SetPartSounds, ACTOR_SELF, LVar0, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
    Call(PlaySoundAtPart, ACTOR_SELF, LVar0, SOUND_HAMMER_BROS_THROW)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar1, LVar2, LVar3)
    Call(SetPartJumpGravity, ACTOR_SELF, LVar0, Float(1.3))
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 15, TRUE)
    Sub(LVar1, 50)
    Set(LVar2, 10)
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 20, TRUE)
    Sub(LVar1, 30)
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 10, TRUE)
    Sub(LVar1, 20)
    Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 5, TRUE)
    Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, TRUE)
    Return
    End
};

EvtScript EVS_Attack_HammerSurge = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Thread
        Set(LVar0, 0)
        Loop(100)
            Add(LVar0, 20)
            Mod(LVar0, 360)
            Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_1, 0, 0, LVar0)
            Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_2, 0, 0, LVar0)
            Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_3, 0, 0, LVar0)
            Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_4, 0, 0, LVar0)
            Call(SetPartRotation, ACTOR_SELF, PRT_HAMMER_5, 0, 0, LVar0)
            Wait(1)
        EndLoop
    EndThread
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 23)
    Add(LVar1, 23)
    Sub(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2)
    Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_2, LVar0, LVar1, LVar2)
    Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_3, LVar0, LVar1, LVar2)
    Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_4, LVar0, LVar1, LVar2)
    Call(SetPartPos, ACTOR_SELF, PRT_HAMMER_5, LVar0, LVar1, LVar2)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfFlag(LVar0, STATUS_FLAG_SHRINK)
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_1, Float(0.4), Float(0.4), Float(0.4))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_2, Float(0.4), Float(0.4), Float(0.4))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_3, Float(0.4), Float(0.4), Float(0.4))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_4, Float(0.4), Float(0.4), Float(0.4))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_5, Float(0.4), Float(0.4), Float(0.4))
    Else
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_1, Float(1.0), Float(1.0), Float(1.0))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_2, Float(1.0), Float(1.0), Float(1.0))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_3, Float(1.0), Float(1.0), Float(1.0))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_4, Float(1.0), Float(1.0), Float(1.0))
        Call(SetPartScale, ACTOR_SELF, PRT_HAMMER_5, Float(1.0), Float(1.0), Float(1.0))
    EndIf
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Set(LVarA, LVar0)
            Set(LVar0, PRT_HAMMER_1)
            Exec(EVS_HammerSurge_Miss)
            Wait(5)
            Set(LVar0, PRT_HAMMER_2)
            Exec(EVS_HammerSurge_Miss)
            Wait(5)
            Set(LVar0, PRT_HAMMER_3)
            Exec(EVS_HammerSurge_Miss)
            Wait(5)
            Set(LVar0, PRT_HAMMER_4)
            Exec(EVS_HammerSurge_Miss)
            Wait(5)
            Set(LVar0, PRT_HAMMER_5)
            Exec(EVS_HammerSurge_Miss)
            Wait(5)
            Wait(20)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_02)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Wait(15)
            Call(YieldTurn)
            Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
            Wait(20)
            Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            Return
        EndCaseGroup
    EndSwitch
    Thread
        Set(LVar0, PRT_HAMMER_1)
        Exec(EVS_HammerSurge_Hit)
        Wait(5)
        Set(LVar0, PRT_HAMMER_2)
        Exec(EVS_HammerSurge_Hit)
        Wait(5)
        Set(LVar0, PRT_HAMMER_3)
        Exec(EVS_HammerSurge_Hit)
        Wait(5)
        Set(LVar0, PRT_HAMMER_4)
        Exec(EVS_HammerSurge_Hit)
        Wait(5)
        Set(LVar0, PRT_HAMMER_5)
        Exec(EVS_HammerSurge_Hit)
    EndThread
    Wait(21)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgHammerSurge, BS_FLAGS1_INCLUDE_POWER_UPS)
    Wait(5)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgHammerSurge, BS_FLAGS1_NICE_HIT)
    Wait(5)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgHammerSurge, BS_FLAGS1_NICE_HIT)
    Wait(5)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_02)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgHammerSurge, BS_FLAGS1_NICE_HIT)
    Wait(5)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, DMG_STATUS_KEY(STATUS_FLAG_SHRINK, 2, 100), dmgHammerSurge, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Wait(20)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    ExecWait(EVS_Attack_HammerSurge)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace green_hammer_bro

namespace yellow_hammer_bro {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_HAMMER_1    = 2,
};

// Actor Stats
constexpr s32 hp = 6;
constexpr s32 dmgHammerThrow = 4;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HammerBrosSMB3_Alt_Anim_02,
    STATUS_KEY_STONE,     ANIM_HammerBrosSMB3_Alt_Anim_00,
    STATUS_KEY_SLEEP,     ANIM_HammerBrosSMB3_Alt_Anim_11,
    STATUS_KEY_POISON,    ANIM_HammerBrosSMB3_Alt_Anim_02,
    STATUS_KEY_STOP,      ANIM_HammerBrosSMB3_Alt_Anim_00,
    STATUS_KEY_STATIC,    ANIM_HammerBrosSMB3_Alt_Anim_02,
    STATUS_KEY_PARALYZE,  ANIM_HammerBrosSMB3_Alt_Anim_00,
    STATUS_KEY_DIZZY,     ANIM_HammerBrosSMB3_Alt_Anim_12,
    STATUS_KEY_FEAR,      ANIM_HammerBrosSMB3_Alt_Anim_12,
    STATUS_END,
};

s32 HammerAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HammerBrosSMB3_Alt_Anim_0D,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   1,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              50,
    STATUS_KEY_POISON,             60,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              100,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,             80,
    STATUS_KEY_PARALYZE,           70,
    STATUS_KEY_SHRINK,             70,
    STATUS_KEY_STOP,               80,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,         -1,
    STATUS_TURN_MOD_FEAR,           0,
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

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    //Call(HPBarToHome, ACTOR_SELF)
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
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0F)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Alt_Anim_10)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0F)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Alt_Anim_10)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_10)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_15)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfNotFlag(LVar0, STATUS_FLAGS_IMMOBILIZED)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Alt_Anim_17)
                Wait(10)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_02)
            ExecWait(EVS_Enemy_Recover)
        CaseEq(EVENT_SCARE_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_07)
            SetConst(LVar2, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_ScareAway)
            Return
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_07)
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
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_0E)
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_HammerBrosSMB3_Alt_Anim_02)
            ExecWait(EVS_Enemy_NoDamageHit)
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_HammerThrow = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
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
        Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, FALSE)
        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Alt_Anim_0C)
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
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Alt_Anim_0C)
    Wait(8)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Label(100)
    Thread
        Wait(15)
        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HammerBrosSMB3_Alt_Anim_02)
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
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 15, TRUE)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVarA, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Sub(LVar0, 40)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 15, TRUE)
            Sub(LVar0, 30)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 10, TRUE)
            Sub(LVar0, 20)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 5, TRUE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Wait(15)
            Call(YieldTurn)
            Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
            Wait(20)
            Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            Return
        EndCaseGroup
    EndSwitch
    Call(SetPartSounds, ACTOR_SELF, PRT_HAMMER_1, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_HAMMER_1, SOUND_HAMMER_BROS_THROW)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_HAMMER_1, Float(1.3))
    Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 15, TRUE)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, DMG_STATUS_KEY(STATUS_FLAG_SHRINK, 2, 100), dmgHammerThrow, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(GetPartOffset, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, 10)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 20, TRUE)
            Sub(LVar0, 30)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 10, TRUE)
            Sub(LVar0, 20)
            Call(JumpPartTo, ACTOR_SELF, PRT_HAMMER_1, LVar0, LVar1, LVar2, 5, TRUE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_HAMMER_1, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Wait(20)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};


EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    ExecWait(EVS_Attack_HammerThrow)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace yellow_hammer_bro

ActorBlueprint GreenHammerBro = {
    .flags = ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_ATTACK, //0,
    .maxHP = green_hammer_bro::hp,
    .type = ACTOR_TYPE_GREEN_HAMMER_BROS,
    .level = ACTOR_LEVEL_GREEN_HAMMER_BROS,
    .partCount = ARRAY_COUNT(green_hammer_bro::ActorParts),
    .partsData = green_hammer_bro::ActorParts,
    .initScript = &green_hammer_bro::EVS_Init,
    .statusTable = green_hammer_bro::StatusTable,
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

ActorBlueprint YellowHammerBro = {
    .flags = 0,
    .maxHP = yellow_hammer_bro::hp,
    .type = ACTOR_TYPE_YELLOW_HAMMER_BROS,
    .level = ACTOR_LEVEL_YELLOW_HAMMER_BROS,
    .partCount = ARRAY_COUNT(yellow_hammer_bro::ActorParts),
    .partsData = yellow_hammer_bro::ActorParts,
    .initScript = &yellow_hammer_bro::EVS_Init,
    .statusTable = yellow_hammer_bro::StatusTable,
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

}; // namespace battle::actor
