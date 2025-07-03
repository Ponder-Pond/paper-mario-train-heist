#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/Bobomb.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"

namespace battle::actor {

namespace bob_omb {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_HandleEvent_Ignited;

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

enum ActorVars {
    AVAR_HitDuringCombo = 0,
    AVAR_IgnitedOnce    = 1,
};

// Actor Stats
constexpr s32 hp = 3;
constexpr s32 dmgExplosion = 6;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_Bobomb_Idle,
    STATUS_KEY_STONE,     ANIM_Bobomb_Still,
    STATUS_KEY_SLEEP,     ANIM_Bobomb_Sleep,
    STATUS_KEY_POISON,    ANIM_Bobomb_Walk,
    STATUS_KEY_STOP,      ANIM_Bobomb_Still,
    STATUS_KEY_STATIC,    ANIM_Bobomb_Idle,
    STATUS_KEY_PARALYZE,  ANIM_Bobomb_Still,
    STATUS_KEY_DIZZY,     ANIM_Bobomb_Dizzy,
    STATUS_KEY_FEAR,      ANIM_Bobomb_Dizzy,
    STATUS_END,
};

s32 IgnitedAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_Bobomb_WalkLit,
    STATUS_KEY_STONE,     ANIM_Bobomb_StillLit,
    STATUS_KEY_SLEEP,     ANIM_Bobomb_Sleep,
    STATUS_KEY_POISON,    ANIM_Bobomb_WalkLit,
    STATUS_KEY_STOP,      ANIM_Bobomb_StillLit,
    STATUS_KEY_STATIC,    ANIM_Bobomb_IdleLit,
    STATUS_KEY_PARALYZE,  ANIM_Bobomb_StillLit,
    STATUS_KEY_DIZZY,     ANIM_Bobomb_DizzyLit,
    STATUS_KEY_FEAR,      ANIM_Bobomb_DizzyLit,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              75,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,            100,
    STATUS_KEY_DIZZY,              80,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,           80,
    STATUS_KEY_SHRINK,             90,
    STATUS_KEY_STOP,               90,
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

s32 IgnitedStatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,               0,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              40,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,           40,
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
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 24 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAG_EXPLODE_ON_IGNITION,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -9 },
    },
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_BobOmbIgnited, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_HitDuringCombo, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_IgnitedOnce, FALSE)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

EvtScript EVS_Ignite = {
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfFlag(LVar0, STATUS_FLAGS_IMMOBILIZED)
        Return
    EndIf
    Label(0)
    Call(SetActorVar, ACTOR_SELF, AVAR_IgnitedOnce, TRUE)
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_BobOmbIgnited, TRUE)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(IgnitedAnims))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent_Ignited))
    Call(SetPartEventBits, ACTOR_SELF, PRT_MAIN, ACTOR_EVENT_FLAG_EXPLODE_ON_CONTACT, TRUE)
    Call(SetStatusTable, ACTOR_SELF, Ref(IgnitedStatusTable))
    Call(PlayLoopingSoundAtActor, ACTOR_SELF, 0, SOUND_LOOP_BOBOMB_FUSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_WalkLit)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetActorJumpGravity, ACTOR_SELF, Float(2.0))
    Call(SetGoalPos, ACTOR_SELF, LVar0, 0, LVar2)
    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
    Call(EnableActorPaletteEffects, ACTOR_SELF, PRT_MAIN, TRUE)
    Call(SetActorPaletteSwapParams, ACTOR_SELF, PRT_MAIN, SPR_PAL_Bobomb, SPR_PAL_Bobomb_Burst, 0, 10, 0, 10, 0, 0)
    Call(SetActorPaletteEffect, ACTOR_SELF, PRT_MAIN, ACTOR_PAL_ADJUST_BLEND_PALETTES_VARYING_INTERVALS)
    Wait(3)
    Call(StopLoopingSoundAtActor, ACTOR_SELF, 0)
    Return
    End
};

EvtScript EVS_Defuse = {
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_BobOmbIgnited, FALSE)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    Call(SetPartEventBits, ACTOR_SELF, PRT_MAIN, ACTOR_EVENT_FLAG_EXPLODE_ON_CONTACT, FALSE)
    Call(SetStatusTable, ACTOR_SELF, Ref(StatusTable))
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar2, 2)
    PlayEffect(EFFECT_LANDING_DUST, 3, LVar0, LVar1, LVar2, 0, 0)
    Call(StopLoopingSoundAtActor, ACTOR_SELF, 0)
    Call(EnableActorPaletteEffects, ACTOR_SELF, PRT_MAIN, FALSE)
    Return
    End
};

EvtScript EVS_Cleanup = {
    Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_BobOmbIgnited, LVar0)
    IfEq(LVar0, TRUE)
        Call(StopLoopingSoundAtActor, ACTOR_SELF, 0)
        Call(EnableActorPaletteEffects, ACTOR_SELF, PRT_MAIN, FALSE)
    EndIf
    Return
    End
};

EvtScript EVS_Explode = {
    ExecWait(EVS_Cleanup)
    Call(StartRumble, BTL_RUMBLE_PLAYER_MAX)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(0.75))
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
        Call(ShakeCam, CAM_BATTLE, 0, 10, Float(4.5))
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
    EndThread
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar2, 2)
    PlayEffect(EFFECT_SMOKE_RING, 0, LVar0, LVar1, LVar2, 0)
    Add(LVar1, 20)
    Add(LVar2, 2)
    PlayEffect(EFFECT_EXPLOSION, 0, LVar0, LVar1, LVar2, 0)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_BOMB_BLAST)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_HIT_COMBO)
            Call(GetLastDamage, ACTOR_SELF, LVar0)
            IfNe(LVar0, 0)
                Call(SetActorVar, ACTOR_SELF, AVAR_HitDuringCombo, TRUE)
            EndIf
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Hurt)
            ExecWait(EVS_Enemy_Hit)
        CaseEq(EVENT_HIT)
            Call(GetLastElement, LVarE)
            IfFlag(LVarE, DAMAGE_TYPE_SHOCK)
                Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_BurnHurt)
                SetConst(LVar2, ANIM_Bobomb_BurnStill)
                ExecWait(EVS_Enemy_BurnHit)
                ExecWait(EVS_Explode)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_BurnStill)
                Set(LVar2, EXEC_DEATH_NO_SPINNING)
                ExecWait(EVS_Enemy_Death)
                Return
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_Hurt)
                ExecWait(EVS_Enemy_Hit)
                Call(GetLastDamage, ACTOR_SELF, LVar0)
                IfNe(LVar0, 0)
                    ExecWait(EVS_Ignite)
                EndIf
            EndIf
        CaseOrEq(EVENT_BURN_HIT)
        CaseOrEq(EVENT_BURN_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnHurt)
            SetConst(LVar2, ANIM_Bobomb_BurnStill)
            ExecWait(EVS_Enemy_BurnHit)
            ExecWait(EVS_Explode)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnStill)
            Set(LVar2, EXEC_DEATH_NO_SPINNING)
            ExecWait(EVS_Enemy_Death)
            Return
        EndCaseGroup
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Hurt)
            ExecWait(EVS_Enemy_SpinSmashHit)
            Call(GetLastDamage, ACTOR_SELF, LVar0)
            IfNe(LVar0, 0)
                ExecWait(EVS_Ignite)
            EndIf
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Hurt)
            ExecWait(EVS_Enemy_SpinSmashHit)
            ExecWait(EVS_Cleanup)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_SHOCK_HIT)
        CaseOrEq(EVENT_SHOCK_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnHurt)
            Set(LVar2, EXEC_DEATH_NO_SPINNING)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_ZERO_DAMAGE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Idle)
            ExecWait(EVS_Enemy_NoDamageHit)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Idle)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetActorVar, ACTOR_SELF, AVAR_HitDuringCombo, LVar0)
            IfEq(LVar0, TRUE)
                ExecWait(EVS_Ignite)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            Call(GetLastElement, LVarE)
            IfFlag(LVarE, DAMAGE_TYPE_SHOCK)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_BurnHurt)
                SetConst(LVar2, ANIM_Bobomb_BurnStill)
                ExecWait(EVS_Enemy_BurnHit)
                ExecWait(EVS_Explode)
                Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_BurnStill)
                Set(LVar2, EXEC_DEATH_NO_SPINNING)
                ExecWait(EVS_Enemy_Death)
                Return
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_Hurt)
                ExecWait(EVS_Enemy_Hit)
                Wait(10)
                ExecWait(EVS_Cleanup)
                Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_Hurt)
                ExecWait(EVS_Enemy_Death)
            EndIf
            Return
        CaseEq(EVENT_EXPLODE_TRIGGER)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnHurt)
            SetConst(LVar2, ANIM_Bobomb_BurnStill)
            ExecWait(EVS_Enemy_BurnHit)
            ExecWait(EVS_Explode)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnStill)
            Set(LVar2, EXEC_DEATH_NO_SPINNING)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Idle)
            ExecWait(EVS_Enemy_Recover)
        CaseEq(EVENT_SCARE_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Run)
            SetConst(LVar2, ANIM_Bobomb_Hurt)
            ExecWait(EVS_Enemy_ScareAway)
            Return
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Dizzy)
            ExecWait(EVS_Enemy_AirLift)
        CaseEq(EVENT_BLOW_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Dizzy)
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_HandleEvent_Ignited = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_HIT_COMBO)
            Call(GetLastElement, LVarE)
            IfFlag(LVarE, DAMAGE_TYPE_WATER)
                ExecWait(EVS_Defuse)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_Hurt)
                ExecWait(EVS_Enemy_Hit)
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_HurtLit)
                ExecWait(EVS_Enemy_Hit)
            EndIf
        CaseEq(EVENT_HIT)
            Call(GetLastElement, LVarE)
            Switch(LVarE)
                CaseFlag(DAMAGE_TYPE_WATER)
                    ExecWait(EVS_Defuse)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, ANIM_Bobomb_Hurt)
                    ExecWait(EVS_Enemy_Hit)
                CaseFlag(DAMAGE_TYPE_SHOCK)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, ANIM_Bobomb_BurnHurt)
                    ExecWait(EVS_Enemy_Hit)
                    ExecWait(EVS_Explode)
                    Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, ANIM_Bobomb_BurnStill)
                    Set(LVar2, EXEC_DEATH_NO_SPINNING)
                    ExecWait(EVS_Enemy_Death)
                    Return
                CaseDefault
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, ANIM_Bobomb_HurtLit)
                    ExecWait(EVS_Enemy_Hit)
                    Call(GetLastDamage, ACTOR_SELF, LVar0)
                    IfGt(LVar0, 0)
                        ExecWait(EVS_Explode)
                        Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
                        SetConst(LVar0, PRT_MAIN)
                        SetConst(LVar1, ANIM_Bobomb_BurnStill)
                        Set(LVar2, EXEC_DEATH_NO_SPINNING)
                        ExecWait(EVS_Enemy_Death)
                        Return
                    EndIf
            EndSwitch
        CaseOrEq(EVENT_BURN_HIT)
        CaseOrEq(EVENT_BURN_DEATH)
        CaseOrEq(EVENT_SPIN_SMASH_HIT)
        CaseOrEq(EVENT_SPIN_SMASH_DEATH)
        CaseOrEq(EVENT_EXPLODE_TRIGGER)
            ExecWait(EVS_Explode)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnStill)
            Set(LVar2, EXEC_DEATH_NO_SPINNING)
            ExecWait(EVS_Enemy_Death)
            Return
        EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_IdleLit)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            Call(GetLastElement, LVarE)
            IfFlag(LVarE, DAMAGE_TYPE_WATER)
                ExecWait(EVS_Defuse)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_Hurt)
            Else
                ExecWait(EVS_Explode)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_BurnStill)
                Set(LVar2, EXEC_DEATH_NO_SPINNING)
            EndIf
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_SHOCK_HIT)
        CaseOrEq(EVENT_SHOCK_DEATH)
            ExecWait(EVS_Explode)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_BurnStill)
            Set(LVar2, EXEC_DEATH_NO_SPINNING)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            ExecWait(EVS_Enemy_Death)
            Return
        EndCaseGroup
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_IdleLit)
            ExecWait(EVS_Enemy_Recover)
        CaseEq(EVENT_SCARE_AWAY)
            ExecWait(EVS_Cleanup)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_Run)
            SetConst(LVar2, ANIM_Bobomb_Hurt)
            ExecWait(EVS_Enemy_ScareAway)
            Return
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Bobomb_DizzyLit)
            ExecWait(EVS_Enemy_AirLift)
        CaseEq(EVENT_BLOW_AWAY)
            Call(GetDamageSource, LVar0)
            IfEq(LVar0, DMG_SRC_HURRICANE)
                ExecWait(EVS_Defuse)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_Dizzy)
            Else
                ChildThread
                    Wait(100)
                    ExecWait(EVS_Explode)
                EndChildThread
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_Bobomb_DizzyLit)
            EndIf
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_UP_AND_AWAY)
            ExecWait(EVS_Defuse)
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_Blast = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Call(PlayLoopingSoundAtActor, ACTOR_SELF, 0, SOUND_LOOP_BOBOMB_FUSE)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Set(LVarA, LVar0)
            Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_RunLit)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(AddGoalPos, ACTOR_SELF, 80, 0, 0)
            Call(SetActorSpeed, ACTOR_SELF, Float(5.0))
            Call(RunToGoal, ACTOR_SELF, 0, FALSE)
            Call(SetActorRotationOffset, ACTOR_SELF, -1, 14, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 90)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_RunLit)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(3.0))
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 30)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            ExecWait(EVS_Explode)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_BurnStill)
            Wait(10)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Wait(10)
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, -1)
            Set(LVar2, EXEC_DEATH_NO_SPINNING)
            ExecWait(EVS_Enemy_Death)
            Return
        EndCaseGroup
    EndSwitch
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_RunLit)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(AddGoalPos, ACTOR_SELF, 10, 0, 0)
    Call(SetActorSpeed, ACTOR_SELF, Float(5.0))
    Call(RunToGoal, ACTOR_SELF, 0, FALSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_Buildup)
    Wait(15)
    Set(LVarA, 1)
    ExecWait(EVS_Explode)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Bobomb_BurnStill)
    Wait(2)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_BLAST | DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgExplosion, BS_FLAGS1_TRIGGER_EVENTS)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Wait(15)
    Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_SummonedBobomb, FALSE)
    SetConst(LVar0, PRT_MAIN)
    SetConst(LVar1, ANIM_Bobomb_BurnStill)
    Set(LVar2, EXEC_DEATH_NO_SPINNING)
    ExecWait(EVS_Enemy_Death)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_BobOmbIgnited, LVar0)
    IfFalse(LVar0)
        ExecWait(EVS_Ignite)
    Else
        ExecWait(EVS_Attack_Blast)
        Return
    EndIf
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    // Call(GetBattlePhase, LVar0)
    // Switch(LVar0)
    //     CaseEq(PHASE_ENEMY_BEGIN)
    Call(GetActorVar, ACTOR_SELF, AVAR_IgnitedOnce, LVar0)
    IfEq(LVar0, FALSE)
        Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_BobOmbIgnited, LVar0)
        IfEq(LVar0, TRUE)
            ExecWait(EVS_Ignite)
        EndIf
    EndIf
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace bob_omb

ActorBlueprint BobOmb = {
    .flags = 0,
    .maxHP = bob_omb::hp,
    .type = ACTOR_TYPE_BOB_OMB_RED_PHASE,
    .level = ACTOR_LEVEL_BOB_OMB_RED_PHASE,
    .partCount = ARRAY_COUNT(bob_omb::ActorParts),
    .partsData = bob_omb::ActorParts,
    .initScript = &bob_omb::EVS_Init,
    .statusTable = bob_omb::StatusTable,
    .escapeChance = 70,
    .airLiftChance = 90,
    .hurricaneChance = 90,
    .spookChance = 90,
    .upAndAwayChance = 95,
    .spinSmashReq = 0,
    .powerBounceChance = 100,
    .coinReward = 1,
    .size = { 34, 35 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
