#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/BattleTubba.h"
#include "sprite/npc/TubbasHeart.h"
#include "sprite/player.h"

namespace battle::actor {

namespace clubbeard {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_PlayFootstepQuaking;
extern EvtScript EVS_Attack_SlamFist;
extern EvtScript EVS_Attack_BodySlam;
extern EvtScript EVS_Death;

enum ActorPartIDs {
    PRT_MAIN        = 1,
};

enum ActorVars {
    AVAR_Unused     = 8,
};

// Clubbeard Actor Stats
constexpr s32 hp = 10;
constexpr s32 dmgFistPound = 5;
constexpr s32 dmgBodySlam = 5;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BattleTubba_Anim01,
    STATUS_KEY_STONE,     ANIM_BattleTubba_Anim00,
    STATUS_KEY_SLEEP,     ANIM_BattleTubba_Anim18,
    STATUS_KEY_POISON,    ANIM_BattleTubba_Anim01,
    STATUS_KEY_STOP,      ANIM_BattleTubba_Anim00,
    STATUS_KEY_STATIC,    ANIM_BattleTubba_Anim00,
    STATUS_KEY_PARALYZE,  ANIM_BattleTubba_Anim00,
    STATUS_KEY_PARALYZE,  ANIM_BattleTubba_Anim00,
    STATUS_KEY_DIZZY,     ANIM_BattleTubba_Anim18,
    STATUS_KEY_FEAR,      ANIM_BattleTubba_Anim00,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              80,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              80,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,           80,
    STATUS_KEY_SHRINK,             90,
    STATUS_KEY_STOP,               80,
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
        .targetOffset = { -15, 75 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -10 },
    },
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(SetActorVar, ACTOR_SELF, AVAR_Unused, 0)
    Exec(EVS_PlayFootstepQuaking)
    Return
    End
};

EvtScript EVS_PlayFootstepQuaking = {
    Label(0)
        Wait(1)
        Call(ActorExists, ACTOR_SELF, LVar0)
        IfEq(LVar0, FALSE)
            Return
        EndIf
        Call(GetStatusFlags, ACTOR_SELF, LVar0)
        IfFlag(LVar0, STATUS_FLAG_SHRINK)
            Goto(0)
        EndIf
        Call(GetAnimation, ACTOR_SELF, PRT_MAIN, LVar0)
        IfEq(LVar0, ANIM_BattleTubba_Anim06)
            Goto(1)
        EndIf
        IfEq(LVar0, ANIM_BattleTubba_Anim07)
            Goto(1)
        EndIf
        Goto(0)
        Label(1)
        Call(StartRumble, BTL_RUMBLE_LONG)
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(0.5))
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_Idle = {
    Label(0)
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_DummyCheck = {
    Call(GetActorVar, ACTOR_SELF, AVAR_Unused, LVar0)
    IfEq(LVar0, 0)
        Return
    EndIf
    Return
    End
};

EvtScript EVS_ReturnHome = {
    Set(LVar1, ANIM_BattleTubba_Anim01)
    ExecWait(EVS_DummyCheck)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
    Call(SetGoalToHome, ACTOR_SELF)
    Call(SetActorSpeed, ACTOR_SELF, Float(6.0))
    Set(LVar1, ANIM_BattleTubba_Anim06)
    ExecWait(EVS_DummyCheck)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
    Call(RunToGoal, ACTOR_SELF, 0, FALSE)
    Set(LVar1, ANIM_BattleTubba_Anim01)
    ExecWait(EVS_DummyCheck)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
    Call(SetActorYaw, ACTOR_SELF, 0)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVarA)
    Switch(LVarA)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            SetConst(LVar2, -1)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            SetConst(LVar2, -1)
            ExecWait(EVS_Enemy_BurnHit)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SHOCK_HIT)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(MoveBattleCamOver, 20)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_Knockback)
            ExecWait(EVS_ReturnHome)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim01)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_SHOCK_DEATH)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(MoveBattleCamOver, 20)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_ShockHit)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_Hit)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim14)
            ExecWait(EVS_Enemy_SpinSmashHit)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleTubba_Anim01)
            ExecWait(EVS_Enemy_Recover)
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(RandInt, 100, LVar0)
    IfLt(LVar0, 60)
        ExecWait(EVS_Attack_SlamFist)
    Else
        ExecWait(EVS_Attack_BodySlam)
    EndIf
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_SlamFist = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
    Call(SetBattleCamDist, 150)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Set(LVar1, ANIM_BattleTubba_Anim06)
    ExecWait(EVS_DummyCheck)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetStatusFlags, ACTOR_SELF, LVar5)
    IfNotFlag(LVar5, STATUS_FLAG_SHRINK)
        Call(AddGoalPos, ACTOR_SELF, 50, 0, 0)
    Else
        Call(AddGoalPos, ACTOR_SELF, 20, 0, 0)
    EndIf
    Call(SetActorSpeed, ACTOR_SELF, Float(4.0))
    Call(RunToGoal, ACTOR_SELF, 0, FALSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim15)
    Wait(8)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_HEAVY_NPC_SWIPE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim16)
    Wait(20)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim17)
    Wait(3)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfNotFlag(LVar0, STATUS_FLAG_SHRINK)
        Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        Set(LVar1, 0)
        PlayEffect(EFFECT_LANDING_DUST, 4, LVar0, LVar1, LVar2, 0, 0)
        Thread
            Call(StartRumble, BTL_RUMBLE_LONG)
            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(15.0))
        EndThread
        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_HEAVY_NPC_LANDING)
    EndIf
    Call(EnemyTestTarget, ACTOR_SELF, LVarA, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVarA)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            PlayEffect(EFFECT_SHOCKWAVE, 0, LVar0, 0, LVar2, 0)
            Wait(30)
            Set(LVar1, ANIM_BattleTubba_Anim01)
            ExecWait(EVS_DummyCheck)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(MoveBattleCamOver, 20)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            ExecWait(EVS_ReturnHome)
            Return
        EndCaseGroup
        CaseDefault
    EndSwitch
    Call(GetStatusFlags, ACTOR_SELF, LVar5)
    IfNotFlag(LVar5, STATUS_FLAG_SHRINK)
        Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        PlayEffect(EFFECT_SHOCKWAVE, 1, LVar0, 0, LVar2, 0)
    EndIf
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    IfNotFlag(LVar5, STATUS_FLAG_SHRINK)
        Call(SetDamageSource, DMG_SRC_TUBBA_SMASH)
    EndIf
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, 0, 0, 0, dmgFistPound, BS_FLAGS1_TRIGGER_EVENTS)
    Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
    Call(SetBattleCamDist, 200)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Call(MoveBattleCamOver, 30)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Wait(30)
            Set(LVar1, ANIM_BattleTubba_Anim01)
            ExecWait(EVS_DummyCheck)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(MoveBattleCamOver, 20)
            Call(YieldTurn)
            ExecWait(EVS_ReturnHome)
        EndCaseGroup
    EndSwitch
    Return
    End
};

EvtScript EVS_Attack_BodySlam = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
    Call(SetBattleCamDist, 150)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Set(LVar1, ANIM_BattleTubba_Anim06)
    ExecWait(EVS_DummyCheck)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, LVar1)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(AddGoalPos, ACTOR_SELF, 70, 0, 0)
    Call(SetActorSpeed, ACTOR_SELF, Float(4.0))
    Call(RunToGoal, ACTOR_SELF, 0, FALSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0A)
    Wait(8)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0B)
    Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_LARGE_ACTOR_JUMP, 0)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Set(LVarA, LVar0)
            Thread
                Wait(12)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0C)
            EndThread
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 15)
            Set(LVar1, 0)
            Add(LVar2, 5)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.0))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 23, FALSE, TRUE, FALSE)
            Call(PlaySoundAtActor, ACTOR_SELF, SOUND_HEAVY_NPC_LANDING)
            PlayEffect(EFFECT_SHOCKWAVE, 0, LVar0, 0, LVar2, 0)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0D)
            Thread
                Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                PlayEffect(EFFECT_SMOKE_IMPACT, 0, LVar0, LVar1, LVar2, 60, 8, 10, 20, 0)
            EndThread
            Thread
                Call(StartRumble, BTL_RUMBLE_LONG)
                Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
            EndThread
            Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 20)
            Set(LVar1, 0)
            Add(LVar2, 5)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.0))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
            Call(StartRumble, BTL_RUMBLE_LONG)
            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            PlayEffect(EFFECT_SMOKE_IMPACT, 0, LVar0, LVar1, LVar2, 60, 8, 10, 20, 0)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(MoveBattleCamOver, 20)
            Wait(8)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0E)
            Wait(4)
            ExecWait(EVS_ReturnHome)
            Return
        EndCaseGroup
        CaseDefault
            Thread
                Wait(9)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0C)
            EndThread
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.0))
            Call(JumpToGoal, ACTOR_SELF, 16, FALSE, TRUE, FALSE)
    EndSwitch
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetStatusFlags, ACTOR_SELF, LVar5)
    IfNotFlag(LVar5, STATUS_FLAG_SHRINK)
        Call(SetDamageSource, DMG_SRC_CRUSH)
        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_HEAVY_NPC_LANDING)
    EndIf
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, 0, 0, 0, dmgBodySlam, BS_FLAGS1_TRIGGER_EVENTS)
    Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
    Call(SetBattleCamDist, 200)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Call(MoveBattleCamOver, 30)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(GetStatusFlags, ACTOR_SELF, LVar5)
            IfFlag(LVar5, STATUS_FLAG_SHRINK)
                Goto(1)
            EndIf
            Call(GetBattleFlags, LVar0)
            IfNotFlag(LVar0, BS_FLAGS1_ATK_BLOCKED)
                Call(GetStatusFlags, ACTOR_PLAYER, LVar0)
                IfNotFlag(LVar0, STATUS_FLAG_STONE)
                    Call(LandJump, ACTOR_SELF)
                Else
                    Goto(1)
                EndIf
            Else
                Goto(1)
            EndIf
            Label(0)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            PlayEffect(EFFECT_SHOCKWAVE, 0, LVar0, 0, LVar2, 0)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0D)
            Thread
                Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                PlayEffect(EFFECT_SMOKE_IMPACT, 0, LVar0, LVar1, LVar2, 60, 8, 10, 20, 0)
            EndThread
            Thread
                Call(StartRumble, BTL_RUMBLE_LONG)
                Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
            EndThread
            Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 20)
            Set(LVar1, 0)
            Add(LVar2, 5)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.0))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
            Call(StartRumble, BTL_RUMBLE_LONG)
            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            PlayEffect(EFFECT_SMOKE_IMPACT, 0, LVar0, LVar1, LVar2, 60, 8, 10, 20, 0)
            Goto(2)
            Label(1)
            Call(GetStatusFlags, ACTOR_SELF, LVar5)
            IfNotFlag(LVar5, STATUS_FLAG_SHRINK)
                Wait(3)
                Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                PlayEffect(EFFECT_SHOCKWAVE, 1, LVar0, 0, LVar2, 0)
                Call(StartRumble, BTL_RUMBLE_LONG)
                Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
            EndIf
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Add(LVar0, 40)
            Set(LVar1, 0)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.8))
            Thread
                Wait(8)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim0E)
            EndThread
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
            IfNotFlag(LVar5, STATUS_FLAG_SHRINK)
                Call(StartRumble, BTL_RUMBLE_LONG)
                Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
                Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                PlayEffect(EFFECT_SMOKE_IMPACT, 0, LVar0, LVar1, LVar2, 60, 8, 10, 20, 0)
                Wait(8)
            EndIf
            Label(2)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(MoveBattleCamOver, 20)
            Call(YieldTurn)
            ExecWait(EVS_ReturnHome)
        EndCaseGroup
    EndSwitch
    Return
    End
};

EvtScript EVS_Death = {
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim14)
    // Call(ActorSpeak, MSG_CH3_00C9, ACTOR_SELF, PRT_MAIN, ANIM_BattleTubba_Anim14, ANIM_BattleTubba_Anim14)
    Set(LVar0, 1)
    Set(LVar1, ANIM_BattleTubba_Anim14)
    ExecWait(EVS_Enemy_Death)
    Return
    End
};

}; // namespace clubbeard

namespace clubbeards_heart {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_Move_Charge;
extern EvtScript EVS_Attack_Leap;
extern EvtScript EVS_Attack_DarkSwarm;
extern EvtScript EVS_FleeFromBattle;
extern EvtScript EVS_ReturnHome;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_TARGET      = 2,
};

enum ActorVars {
    AVAR_NextMove           = 0,
    AVAL_NextMove_Charge    = 0,
    AVAL_NextMove_Swarm     = 1,
    AVAL_NextMove_Leap      = 2,
    AVAR_ChargeLevel        = 1,
    AVAR_Flags              = 2,
    AVAL_Flag_TauntBow      = 1,
    AVAL_Flag_HidStatusBar  = 2,
    AVAR_ChargeTaunt        = 3,
    AVAR_ChargedEffectID    = 4,
    AVAR_SwarmCount         = 5,
};

// Clubbeards Heart Actor Stats
constexpr s32 hp = 50;
constexpr s32 dmgLeap = 6;
constexpr s32 dmgSwarm = 12;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_TubbasHeart_Anim01,
    STATUS_KEY_STONE,     ANIM_TubbasHeart_Anim00,
    STATUS_KEY_SLEEP,     ANIM_TubbasHeart_Anim04,
    STATUS_KEY_POISON,    ANIM_TubbasHeart_Anim01,
    STATUS_KEY_STOP,      ANIM_TubbasHeart_Anim00,
    STATUS_KEY_STATIC,    ANIM_TubbasHeart_Anim00,
    STATUS_KEY_PARALYZE,  ANIM_TubbasHeart_Anim00,
    STATUS_KEY_PARALYZE,  ANIM_TubbasHeart_Anim00,
    STATUS_KEY_DIZZY,     ANIM_TubbasHeart_Anim0D,
    STATUS_KEY_DIZZY,     ANIM_TubbasHeart_Anim0D,
    STATUS_KEY_FEAR,      ANIM_TubbasHeart_Anim0A,
    STATUS_END,
};

s32 ChargedAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_TubbasHeart_Anim0B,
    STATUS_KEY_STONE,     ANIM_TubbasHeart_Anim00,
    STATUS_KEY_SLEEP,     ANIM_TubbasHeart_Anim04,
    STATUS_KEY_POISON,    ANIM_TubbasHeart_Anim0B,
    STATUS_KEY_STOP,      ANIM_TubbasHeart_Anim00,
    STATUS_KEY_STATIC,    ANIM_TubbasHeart_Anim00,
    STATUS_KEY_PARALYZE,  ANIM_TubbasHeart_Anim00,
    STATUS_KEY_PARALYZE,  ANIM_TubbasHeart_Anim00,
    STATUS_KEY_DIZZY,     ANIM_TubbasHeart_Anim0D,
    STATUS_KEY_DIZZY,     ANIM_TubbasHeart_Anim0D,
    STATUS_KEY_FEAR,      ANIM_TubbasHeart_Anim0A,
    STATUS_END,
};

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
    STATUS_KEY_SHRINK,             90,
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
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 24 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -10 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_PRIMARY_TARGET | ACTOR_PART_FLAG_SKIP_MOVEMENT_ALLOC,
        .index = PRT_TARGET,
        .posOffset = { 0, 54, 0 },
        .targetOffset = { 0, -30 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -10 },
    },
};

EvtScript EVS_Init = {
    Call(SetActorVar, ACTOR_SELF, AVAR_NextMove, AVAL_NextMove_Charge)
    Call(SetActorVar, ACTOR_SELF, AVAR_ChargeLevel, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_Flags, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_ChargeTaunt, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_ChargedEffectID, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_SwarmCount, 0)
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_TARGET, ACTOR_PART_TARGET_NO_SMASH, TRUE)
    Return
    End
};

API_CALLABLE(UpdateChargeEffectPos) {
    Bytecode *args = script->ptrReadPos;
    EffectInstance* effect = (EffectInstance*) evt_get_variable(script, *args++);

    s32 x = evt_get_variable(script, *args++);
    s32 y = evt_get_variable(script, *args++);
    s32 z = evt_get_variable(script, *args++);
    f32 scale = evt_get_float_variable(script, *args++);

    effect->data.energyInOut->pos.x = x;
    effect->data.energyInOut->pos.y = y;
    effect->data.energyInOut->pos.z = z;
    effect->data.energyInOut->scale = scale;

    return ApiStatus_DONE2;
}

EvtScript EVS_Idle = {
    Label(0)
        Call(ActorExists, ACTOR_SELF, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorVar, ACTOR_SELF, AVAR_ChargedEffectID, LVar0)
            IfNe(LVar0, 0)
                Call(GetActorPos, ACTOR_SELF, LVar1, LVar2, LVar3)
                Call(GetStatusFlags, ACTOR_SELF, LVar4)
                IfNotFlag(LVar4, STATUS_FLAG_SHRINK)
                    Add(LVar2, 15)
                    SetF(LVar4, Float(1.0))
                Else
                    Add(LVar2, 6)
                    SetF(LVar4, Float(0.4))
                EndIf
                Call((UpdateChargeEffectPos), LVar0, LVar1, LVar2, LVar3, LVar4)
            EndIf
        EndIf
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_SelectAnimation = {
    Set(LVar1, ANIM_TubbasHeart_Anim0E)
    Call(GetActorVar, ACTOR_SELF, AVAR_ChargeLevel, LVar0)
    IfNe(LVar0, 0)
        Set(LVar1, ANIM_TubbasHeart_Anim0B)
    EndIf
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(SetActorScale, ACTOR_SELF, Float(1.0), Float(1.0), Float(1.0))
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            ExecWait(EVS_SelectAnimation)
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BEGIN_FIRST_STRIKE)
        CaseOrEq(EVENT_BURN_HIT)
        CaseOrEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_TubbasHeart_Anim10)
            SetConst(LVar2, ANIM_TubbasHeart_Anim11)
            ExecWait(EVS_Enemy_BurnHit)
        EndCaseGroup
        CaseEq(EVENT_SPIN_SMASH_HIT)
            ExecWait(EVS_SelectAnimation)
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseOrEq(EVENT_SHOCK_HIT)
        CaseOrEq(EVENT_SHOCK_DEATH)
            ExecWait(EVS_SelectAnimation)
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_ShockHit)
            ExecWait(EVS_SelectAnimation)
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_Knockback)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim03)
            ExecWait(EVS_ReturnHome)
        EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            Set(LVar1, ANIM_TubbasHeart_Anim01)
            Call(GetActorVar, ACTOR_SELF, AVAR_ChargeLevel, LVar0)
            IfNe(LVar0, 0)
                Set(LVar1, ANIM_TubbasHeart_Anim0B)
            EndIf
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseOrEq(EVENT_DEATH)
            ExecWait(EVS_SelectAnimation)
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            ExecWait(EVS_SelectAnimation)
            SetConst(LVar0, PRT_MAIN)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIKE_CONTACT)
        CaseEq(EVENT_BURN_CONTACT)
        CaseEq(EVENT_END_FIRST_STRIKE)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim03)
            ExecWait(EVS_ReturnHome)
            Call(HPBarToHome, ACTOR_SELF)
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_TubbasHeart_Anim01)
            ExecWait(EVS_Enemy_Recover)
        CaseDefault
    EndSwitch
    Call(GetActorHP, ACTOR_SELF, LVar0)
    IfLe(LVar0, 5)
        ExecWait(EVS_FleeFromBattle)
        Return
    Else
        Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    EndIf
    Return
    End
};

EvtScript EVS_FleeFromBattle = {
    Call(UseBattleCamPreset, BTL_CAM_ACTOR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 40)
    Wait(40)
    // Call(ActorSpeak, MSG_CH3_00C6, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim0D, ANIM_TubbasHeart_Anim0D)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(MoveBattleCamOver, 25)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_APPLY, TRUE)
    Call(SetBattleFlagBits, BS_FLAGS1_DISABLE_CELEBRATION | BS_FLAGS1_BATTLE_FLED, TRUE)
    Call(SetEndBattleFadeOutRate, 20)
    Return
    End
};

EvtScript EVS_ReturnHome = {
    Call(SetActorSpeed, ACTOR_SELF, Float(6.0))
    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.8))
    Call(SetGoalToHome, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 45)
    Set(LVar1, 0)
    ExecWait(EVS_Enemy_HopToPos)
    Call(SetGoalToHome, ACTOR_SELF)
    Call(JumpToGoal, ACTOR_SELF, 12, FALSE, TRUE, FALSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim01)
    Call(SetActorYaw, ACTOR_SELF, 0)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    // Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    // IfNe(LVar0, HIT_RESULT_MISS)
    //     Call(GetActorVar, ACTOR_SELF, AVAR_Flags, LVar0)
    //     IfNotFlag(LVar0, AVAL_Flag_TauntBow)
    //         Call(GetCurrentPartnerID, LVar0)
    //         IfEq(LVar0, PARTNER_BOW)
    //             Call(EnableBattleStatusBar, FALSE)
    //             Call(GetActorVar, ACTOR_SELF, AVAR_Flags, LVar0)
    //             BitwiseOrConst(LVar0, AVAL_Flag_TauntBow | AVAL_Flag_HidStatusBar)
    //             Call(SetActorVar, ACTOR_SELF, AVAR_Flags, LVar0)
    //             Call(UseBattleCamPreset, BTL_CAM_ACTOR)
    //             Call(BattleCamTargetActor, ACTOR_SELF)
    //             Call(MoveBattleCamOver, 40)
    //             Wait(40)
    //             Call(ActorSpeak, MSG_CH3_00C3, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim0B, ANIM_TubbasHeart_Anim01)
    //             Call(UseBattleCamPreset, BTL_CAM_ACTOR)
    //             Call(BattleCamTargetActor, ACTOR_PARTNER)
    //             Call(MoveBattleCamOver, 25)
    //             Wait(25)
    //             Call(UseIdleAnimation, ACTOR_PARTNER, FALSE)
    //             Call(SetAnimation, ACTOR_PARTNER, 1, ANIM_BattleBow_Talk)
    //             Call(ActorSpeak, MSG_CH3_00C4, ACTOR_PARTNER, 1, ANIM_BattleBow_Talk, ANIM_BattleBow_Idle)
    //             Call(SetAnimation, ACTOR_PARTNER, 1, ANIM_BattleBow_Laugh)
    //             Call(EndActorSpeech, ACTOR_PARTNER, 1, -1, -1)
    //             Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    //             Call(UseIdleAnimation, ACTOR_PARTNER, TRUE)
    //             Call(MoveBattleCamOver, 30)
    //             Call(GetActorVar, ACTOR_SELF, AVAR_NextMove, LVar0)
    //             IfEq(LVar0, AVAL_NextMove_Charge)
    //                 Call(GetActorVar, ACTOR_SELF, AVAR_ChargeTaunt, LVar0)
    //                 IfTrue(LVar0)
    //                     Call(EnableBattleStatusBar, TRUE)
    //                 EndIf
    //             Else
    //                 Call(EnableBattleStatusBar, TRUE)
    //             EndIf
    //         EndIf
    //     EndIf
    // EndIf
    Call(GetActorVar, ACTOR_SELF, AVAR_NextMove, LVar0)
    Switch(LVar0)
        CaseEq(AVAL_NextMove_Charge)
            // Call(GetActorVar, ACTOR_SELF, AVAR_ChargeTaunt, LVar0)
            // IfEq(LVar0, 0)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_ChargeTaunt, TRUE)
            //     Call(UseBattleCamPreset, BTL_CAM_ACTOR)
            //     Call(BattleCamTargetActor, ACTOR_SELF)
            //     Call(MoveBattleCamOver, 40)
            //     Wait(40)
            //     Call(ActorSpeak, MSG_CH3_00C5, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim0B, ANIM_TubbasHeart_Anim01)
            // EndIf
            ExecWait(EVS_Move_Charge)
            Call(SetActorVar, ACTOR_SELF, AVAR_NextMove, AVAL_NextMove_Swarm)
            // Call(GetActorVar, ACTOR_SELF, AVAR_Flags, LVar0)
            // IfFlag(LVar0, AVAL_Flag_HidStatusBar)
            //     Call(EnableBattleStatusBar, TRUE)
            // EndIf
        CaseEq(AVAL_NextMove_Swarm)
            ExecWait(EVS_Attack_DarkSwarm)
            Call(GetActorVar, ACTOR_SELF, AVAR_ChargeLevel, LVar0)
            IfEq(LVar0, 0)
                Call(GetActorVar, ACTOR_SELF, AVAR_SwarmCount, LVar1)
                Add(LVar1, 1)
                Call(SetActorVar, ACTOR_SELF, AVAR_SwarmCount, LVar1)
                IfGe(LVar1, 2)
                    Call(SetActorVar, ACTOR_SELF, AVAR_NextMove, AVAL_NextMove_Leap)
                Else
                    Call(SetActorVar, ACTOR_SELF, AVAR_NextMove, AVAL_NextMove_Charge)
                EndIf
            EndIf
        CaseEq(AVAL_NextMove_Leap)
            Call(SetActorVar, ACTOR_SELF, AVAR_NextMove, AVAL_NextMove_Charge)
            ExecWait(EVS_Attack_Leap)
    EndSwitch
    // Call(GetActorVar, ACTOR_SELF, AVAR_Flags, LVar0)
    // BitwiseAndConst(LVar0, ~AVAL_Flag_HidStatusBar)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Flags, LVar0)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_Leap = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim03)
    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.8))
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 45)
    Set(LVar1, 0)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(JumpToGoal, ACTOR_SELF, 12, FALSE, TRUE, FALSE)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 40)
    Set(LVar1, 0)
    Call(SetActorSpeed, ACTOR_SELF, Float(6.0))
    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.8))
    ExecWait(EVS_Enemy_HopToPos)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim01)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim16)
    Wait(5)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim15)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Set(LVarA, LVar0)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 30)
            Set(LVar1, 0)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.5))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 20)
            Set(LVar1, 0)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(2.0))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 6, FALSE, TRUE, FALSE)
            Sub(LVar0, 10)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 4, FALSE, TRUE, FALSE)
            Wait(20)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim03)
            ExecWait(EVS_ReturnHome)
            Return
        EndCaseGroup
        CaseDefault
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(0.3))
            Call(JumpToGoal, ACTOR_SELF, 4, FALSE, TRUE, FALSE)
    EndSwitch
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, 0, 0, 0, dmgLeap, BS_FLAGS1_TRIGGER_EVENTS)
    Set(LVarF, LVar0)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
        CaseOrEq(HIT_RESULT_10)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim01)
            Call(SetActorScale, ACTOR_SELF, Float(1.1), Float(0.8), Float(1.0))
            Wait(1)
            Call(SetActorScale, ACTOR_SELF, Float(1.0), Float(1.0), Float(1.0))
            Wait(1)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Add(LVar0, 30)
            Set(LVar1, 0)
            Call(SetActorJumpGravity, ACTOR_SELF, Float(1.8))
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
            Add(LVar0, 20)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 6, FALSE, TRUE, FALSE)
            Add(LVar0, 10)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 4, FALSE, TRUE, FALSE)
            Wait(8)
            IfEq(LVarF, HIT_RESULT_10)
                Return
            EndIf
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(YieldTurn)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim03)
            ExecWait(EVS_ReturnHome)
        EndCaseGroup
    EndSwitch
    Return
    End
};

EvtScript EVS_Move_Charge = {
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(SetBattleCamTarget, 85, 50, 0)
    Call(SetBattleCamDist, 280)
    Call(SetBattleCamOffsetY, 0)
    Call(MoveBattleCamOver, 40)
    Wait(40)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim0B)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetStatusFlags, ACTOR_SELF, LVar4)
    IfNotFlag(LVar4, STATUS_FLAG_SHRINK)
        Add(LVar1, 15)
        SetF(LVar3, Float(1.0))
    Else
        Add(LVar1, 6)
        SetF(LVar3, Float(0.4))
    EndIf
    PlayEffect(EFFECT_GATHER_MAGIC, 1, LVar0, LVar1, LVar2, LVar3, 60, 0)
    PlayEffect(EFFECT_ENERGY_IN_OUT, 6, LVar0, LVar1, LVar2, LVar3, 60, 0)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_TUBBAS_HEART_CHARGE)
    Wait(60)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(MoveBattleCamOver, 20)
    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.0))
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(JumpToGoal, ACTOR_SELF, 15, FALSE, TRUE, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_ChargeLevel, 1)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetStatusFlags, ACTOR_SELF, LVar4)
    IfNotFlag(LVar4, STATUS_FLAG_SHRINK)
        Add(LVar1, 15)
        SetF(LVar3, Float(1.0))
    Else
        Add(LVar1, 6)
        SetF(LVar3, Float(0.4))
    EndIf
    PlayEffect(EFFECT_ENERGY_IN_OUT, 3, LVar0, LVar1, LVar2, LVar3, 0, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_ChargedEffectID, LVarF)
    Call(EnableActorGlow, ACTOR_SELF, TRUE)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ChargedAnims))
    Return
    End
};

EvtScript EVS_Attack_DarkSwarm = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(SetBattleCamTarget, 85, 50, 0)
    Call(SetBattleCamDist, 280)
    Call(SetBattleCamOffsetY, 0)
    Call(MoveBattleCamOver, 20)
    Wait(20)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SPELL_CAST1)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim0C)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetStatusFlags, ACTOR_SELF, LVar3)
    IfNotFlag(LVar3, STATUS_FLAG_SHRINK)
        Add(LVar1, 16)
    Else
        Add(LVar1, 6)
    EndIf
    PlayEffect(EFFECT_GATHER_ENERGY_PINK, 0, LVar0, LVar1, LVar2, Float(1.0), 45, 0)
    Wait(60)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(MoveBattleCamOver, 20)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_TubbasHeart_Anim0B)
    Thread
        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        Call(SetActorJumpGravity, ACTOR_SELF, Float(0.8))
        Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        Call(JumpToGoal, ACTOR_SELF, 18, FALSE, TRUE, FALSE)
    EndThread
    Call(GetActorVar, ACTOR_SELF, AVAR_ChargeLevel, LVar0)
    Sub(LVar0, 1)
    Call(SetActorVar, ACTOR_SELF, AVAR_ChargeLevel, LVar0)
    IfEq(LVar0, 0)
        Call(EnableActorGlow, ACTOR_SELF, FALSE)
        Call(GetActorVar, ACTOR_SELF, AVAR_ChargedEffectID, LVar0)
        Call(RemoveEffect, LVar0)
        Call(SetActorVar, ACTOR_SELF, AVAR_ChargedEffectID, 0)
        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    EndIf
    Call(PlaySound, SOUND_TUBBAS_HEART_SWARM_ATTACK)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyTestTarget, ACTOR_SELF, LVarA, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVarA)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Add(LVar1, 5)
            PlayEffect(EFFECT_TUBBA_HEART_ATTACK, FX_HEART_SWARM_MISS, LVar0, LVar1, LVar2, Float(1.0), 200, 0)
            Wait(145)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            Else
            EndIf
            Return
        EndCaseGroup
    EndSwitch
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Add(LVar1, 5)
    PlayEffect(EFFECT_TUBBA_HEART_ATTACK, FX_HEART_SWARM_HIT, LVar0, LVar1, LVar2, Float(1.0), 200, 0)
    Thread
        Wait(160)
        Call(PlaySound, SOUND_TUBBAS_HEART_SWARM_VANISH)
    EndThread
    Call(UseIdleAnimation, ACTOR_PLAYER, FALSE)
    Call(SetAnimation, ACTOR_PLAYER, 0, ANIM_Mario1_Flail)
    Wait(30)
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Call(SetBattleCamTarget, LVar0, LVar1, LVar2)
    Call(SetBattleCamDist, 300)
    Call(SetBattleCamOffsetY, 35)
    Call(MoveBattleCamOver, 50)
    Wait(56)
    Call(PlaySoundAtActor, ACTOR_PLAYER, SOUND_SQUEEZE)
    Call(SetAnimation, ACTOR_PLAYER, 0, ANIM_MarioB1_Leeching)
    Wait(37)
    Call(PlaySoundAtActor, ACTOR_PLAYER, SOUND_SQUEEZE)
    Wait(22)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(MoveBattleCamOver, 10)
    Call(PlaySoundAtActor, ACTOR_PLAYER, SOUND_TUBBAS_HEART_SWARM_DISPERSE)
    Call(UseIdleAnimation, ACTOR_PLAYER, TRUE)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgSwarm, BS_FLAGS1_TRIGGER_EVENTS)
    Return
    End
};

}; // namespace clubbeards_heart

ActorBlueprint Clubbeard = {
    .flags = 0,
    .maxHP = clubbeard::hp,
    .type = ACTOR_TYPE_CLUBBEARD,
    .level = ACTOR_LEVEL_CLUBBEARD,
    .partCount = ARRAY_COUNT(clubbeard::ActorParts),
    .partsData = clubbeard::ActorParts,
    .initScript = &clubbeard::EVS_Init,
    .statusTable = clubbeard::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 4,
    .powerBounceChance = 75,
    .coinReward = 0,
    .size = { 110, 100 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -33, 78 },
    .statusTextOffset = { 10, 75 },
};

ActorBlueprint ClubbeardsHeart = {
    .flags = 0,
    .maxHP = clubbeards_heart::hp,
    .type = ACTOR_TYPE_TUBBA_HEART,
    .level = ACTOR_LEVEL_TUBBA_HEART,
    .partCount = ARRAY_COUNT(clubbeards_heart::ActorParts),
    .partsData = clubbeards_heart::ActorParts,
    .initScript = &clubbeards_heart::EVS_Init,
    .statusTable = clubbeards_heart::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 4,
    .powerBounceChance = 75,
    .coinReward = 0,
    .size = { 30, 30 },
    .healthBarOffset = { 5, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
