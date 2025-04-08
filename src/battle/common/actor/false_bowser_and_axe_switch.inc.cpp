#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/BattleBowser.h"
#include "entity.h"
#include "mapfs/smb_bt00_shape.h"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace false_bowser {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_TakeTurn_Inner;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_Attack_FireBall;
extern EvtScript EVS_Attack_Jump;
extern EvtScript EVS_UseAttack;
extern EvtScript EVS_Hit;
extern EvtScript EVS_Death;
extern EvtScript EVS_NoDamageHit;
extern EvtScript EVS_BasicHit;
extern EvtScript EVS_BurnHit;
extern EvtScript EVS_ShockReaction;
extern EvtScript EVS_ReturnHome;
extern EvtScript EVS_PlayFootstepSounds;

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

enum ActorVars {
    AVAR_TurnCount = 0,
    AVAR_AxeSwitch = 1,
};

// Actor Stats
constexpr s32 hp = 10;
constexpr s32 dmgJump = 1;
constexpr s32 dmgFireBall = 1;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BattleBowser_Idle,
    STATUS_KEY_STONE,     ANIM_BattleBowser_Still,
    STATUS_KEY_SLEEP,     ANIM_BattleBowser_Idle,
    STATUS_KEY_POISON,    ANIM_BattleBowser_Idle,
    STATUS_KEY_STOP,      ANIM_BattleBowser_Still,
    STATUS_KEY_STATIC,    ANIM_BattleBowser_Still,
    STATUS_KEY_PARALYZE,  ANIM_BattleBowser_Still,
    STATUS_KEY_PARALYZE,  ANIM_BattleBowser_Still,
    STATUS_KEY_DIZZY,     ANIM_BattleBowser_Idle,
    STATUS_KEY_DIZZY,     ANIM_BattleBowser_Idle,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              30,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              30,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,           30,
    STATUS_KEY_SHRINK,             50,
    STATUS_KEY_STOP,               30,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,         -1,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,         -1,
    STATUS_TURN_MOD_FEAR,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,      -1,
    STATUS_TURN_MOD_SHRINK,        -1,
    STATUS_TURN_MOD_STOP,          -2,
    STATUS_END,
};

s32 BoostedStatusTable[] = {
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
    STATUS_TURN_MOD_SLEEP,         -1,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,         -1,
    STATUS_TURN_MOD_FEAR,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,      -1,
    STATUS_TURN_MOD_SHRINK,        -1,
    STATUS_TURN_MOD_STOP,          -2,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -18, 72 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -8 },
    },
};

#include "common/StartRumbleWithParams.inc.c"

EvtScript EVS_Init = {
    Call(SetActorVar, ACTOR_SELF, AVAR_TurnCount, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_AxeSwitch, FALSE)
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Return
    End
};

EvtScript EVS_Idle = {
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetAnimation, ACTOR_SELF, PRT_MAIN, LVar3)
    Label(0)
        Call(ActorExists, ACTOR_SELF, LVarB)
        IfEq(LVarB, 0)
            Return
        EndIf
        Call(GetStatusFlags, ACTOR_SELF, LVarB)
        IfNotFlag(LVarB, STATUS_FLAG_SHRINK)
            Call(ModifyActorDecoration, ACTOR_SELF, PRT_MAIN, 1, 100, 0, 0, 0)
        Else
            Call(ModifyActorDecoration, ACTOR_SELF, PRT_MAIN, 1, 40, 0, 0, 0)
        EndIf
        ExecWait(EVS_PlayFootstepSounds)
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_PlayFootstepSounds = {
    Call(GetAnimation, ACTOR_SELF, PRT_MAIN, LVar7)
    IfNe(LVar7, ANIM_BattleBowser_Walk)
        IfEq(LVar3, ANIM_BattleBowser_Walk)
            Call(PlaySoundAtActor, ACTOR_SELF, SOUND_GOOMBA_STEP)
            Call(PlaySoundAtActor, ACTOR_SELF, SOUND_GOOMBA_STEP)
        EndIf
        Set(LVar3, LVar7)
        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        Return
    EndIf
    Call(GetActorPos, ACTOR_SELF, LVar5, LVar6, LVar7)
    Sub(LVar5, LVar0)
    IfGt(LVar5, -30)
        IfLt(LVar5, 30)
            Set(LVar3, LVar7)
            Return
        EndIf
    EndIf
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_GOOMBA_STEP)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_GOOMBA_STEP)
    Set(LVar3, LVar7)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_BEGIN_FIRST_STRIKE)
        CaseEq(EVENT_HIT_COMBO)
            Set(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Hit)
            Return
        CaseEq(EVENT_HIT)
            Set(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_BasicHit)
            Return
        CaseEq(EVENT_DEATH)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_BURN_HIT)
            Set(LVar0, PRT_MAIN)
            Set(LVar1, ANIM_BattleBowser_BurnHurt)
            Set(LVar2, ANIM_BattleBowser_BurnStill)
            ExecWait(EVS_BurnHit)
            Return
        CaseEq(EVENT_BURN_DEATH)
            Set(LVar0, PRT_MAIN)
            Set(LVar1, ANIM_BattleBowser_BurnHurt)
            Set(LVar2, ANIM_BattleBowser_BurnStill)
            ExecWait(EVS_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_BurnStill)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            Set(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_BasicHit)
            Return
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_SHOCK_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_ShockReaction)
            Call(SetActorSpeed, ACTOR_SELF, Float(6.0))
            Set(LVar1, ANIM_BattleBowser_Walk)
            ExecWait(EVS_ReturnHome)
        CaseEq(EVENT_SHOCK_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_ShockReaction)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Hurt)
            ExecWait(EVS_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Set(LVar1, ANIM_BattleBowser_Idle)
            ExecWait(EVS_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            Set(LVar1, ANIM_BattleBowser_Idle)
            ExecWait(EVS_Enemy_NoDamageHit)
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BattleBowser_Jump)
            ExecWait(EVS_Enemy_Recover)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PostJump)
            Call(ShakeCam, CAM_BATTLE, 0, 4, Float(3.0))
        CaseDefault
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_BasicHit = {
    ExecWait(EVS_Hit)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Hit = {
    SetConst(LVar0, PRT_MAIN)
    ExecWait(EVS_Enemy_Hit)
    Return
    End
};

EvtScript EVS_Death = {
    Call(HideHealthBar, ACTOR_SELF)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Set(LVar2, EXEC_DEATH_NO_SPINNING)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Set(LVar3, 1000)
    Set(LVar4, 1000)
    PlayEffect(EFFECT_SHAPE_SPELL, 2, LVar0, LVar1, LVar2, LVar3, LVar4, LVar5, 30, 0)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 10)
    Add(LVar2, 10)
    PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_DEATH)
    Call(DropStarPoints, ACTOR_SELF)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_SHADOW, TRUE)
    Wait(30)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    // ExecWait(EVS_ForceNextTarget)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

EvtScript EVS_BurnHit = {
    Call(GetLastEvent, ACTOR_SELF, LVar3)
    IfEq(LVar3, 36)
        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 1)
        Call(SetPartEventBits, ACTOR_SELF, PRT_MAIN, ACTOR_EVENT_FLAG_ENCHANTED, FALSE)
    EndIf
    Call(SetAnimation, ACTOR_SELF, LVar0, LVar1)
    Call(GetDamageSource, LVar3)
    Switch(LVar3)
        CaseEq(DMG_SRC_FIRE_SHELL)
            Call(GetOriginalActorType, ACTOR_SELF, LVar7)
            Switch(LVar7)
                CaseOrEq(ACTOR_TYPE_MONTY_MOLE)
                CaseOrEq(ACTOR_TYPE_MONTY_MOLE_BOSS)
                EndCaseGroup
                CaseDefault
                    Call(GetActorPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(0.01))
                    Add(LVar5, 55)
                    Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                    Call(JumpToGoal, ACTOR_SELF, 8, FALSE, FALSE, FALSE)
            EndSwitch
            Set(LVar7, 0)
            Loop(30)
                Add(LVar7, 48)
                Call(SetActorYaw, ACTOR_SELF, LVar7)
                Wait(1)
            EndLoop
            Call(GetOriginalActorType, ACTOR_SELF, LVar7)
            Switch(LVar7)
                CaseOrEq(ACTOR_TYPE_MONTY_MOLE)
                CaseOrEq(ACTOR_TYPE_MONTY_MOLE_BOSS)
                    Wait(30)
                EndCaseGroup
                CaseDefault
                    Sub(LVar5, 55)
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(0.8))
                    IfEq(LVar5, 0)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 15, FALSE, TRUE, FALSE)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Else
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 15, FALSE, FALSE, FALSE)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 10, FALSE, FALSE, FALSE)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 5, FALSE, FALSE, FALSE)
                    EndIf
            EndSwitch
        CaseDefault
            Wait(20)
    EndSwitch
    IfNe(LVar2, -1)
        Call(SetAnimation, ACTOR_SELF, LVar0, LVar2)
    EndIf
    Wait(10)
    Call(GetLastEvent, ACTOR_SELF, LVar1)
    Switch(LVar1)
        CaseEq(EVENT_BURN_HIT)
            Call(GetPartEventFlags, ACTOR_SELF, LVar0, LVar1)
            IfNotFlag(LVar1, ACTOR_EVENT_FLAG_FIREY | ACTOR_EVENT_FLAG_EXPLODE_ON_IGNITION)
                Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                Call(GetActorSize, ACTOR_SELF, LVar3, LVar4)
                Call(GetStatusFlags, ACTOR_SELF, LVar5)
                IfFlag(LVar5, STATUS_FLAG_SHRINK)
                    MulF(LVar3, Float(0.4))
                    MulF(LVar4, Float(0.4))
                EndIf
                DivF(LVar3, Float(2.0))
                Call(GetActorFlags, ACTOR_SELF, LVar5)
                IfFlag(LVar5, ACTOR_FLAG_UPSIDE_DOWN)
                    SubF(LVar1, LVar3)
                Else
                    AddF(LVar1, LVar3)
                EndIf
                AddF(LVar2, Float(5.0))
                DivF(LVar3, Float(10.0))
                Call(GetStatusFlags, ACTOR_SELF, LVar5)
                IfFlag(LVar5, STATUS_FLAG_SHRINK)
                    MulF(LVar3, Float(0.4))
                    IfLt(LVar3, 1)
                        SetF(LVar3, Float(1.0))
                    EndIf
                EndIf
                PlayEffect(EFFECT_SMOKE_BURST, 0, LVar0, LVar1, LVar2, LVar3, 10, 0)
            EndIf
        CaseEq(EVENT_BURN_DEATH)
            // do nothing
    EndSwitch
    Return
    End
};

EvtScript EVS_ShockReaction = {
    IfNe(LVar1, -1)
        Call(SetAnimation, ACTOR_SELF, LVar0, LVar1)
    EndIf
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.4))
    Call(SetGoalToHome, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar2, LVar3, LVar4)
    Call(GetActorPos, ACTOR_SELF, LVar5, LVar6, LVar7)
    Set(LVar8, LVar2)
    Add(LVar8, LVar5)
    Div(LVar8, 2)
    Set(LVar9, LVar4)
    Add(LVar9, LVar7)
    Div(LVar9, 2)
    Call(SetGoalPos, ACTOR_SELF, LVar8, 0, LVar9)
    Call(JumpToGoal, ACTOR_SELF, 15, FALSE, TRUE, FALSE)
    Add(LVar8, LVar2)
    Div(LVar8, 2)
    Add(LVar9, LVar4)
    Div(LVar9, 2)
    Call(SetGoalPos, ACTOR_SELF, LVar8, 0, LVar9)
    Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
    Call(SetGoalPos, ACTOR_SELF, LVar2, LVar3, LVar4)
    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
    Return
    End
};

EvtScript EVS_NoDamageHit = {
    SetConst(LVar0, PRT_MAIN)
    ExecWait(EVS_Enemy_NoDamageHit)
    Return
    End
};

EvtScript EVS_ReturnHome = {
    Set(LVar0, PRT_MAIN)
    Call(GetActorPos, ACTOR_SELF, LVarA, LVarB, LVarC)
    Call(SetGoalToHome, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVarD, LVarE, LVarF)
    Call(SetActorYaw, ACTOR_SELF, 0)
    Call(GetDist2D, LVar9, LVarA, LVarC, LVarD, LVarF)
    IfGe(LVar9, Float(5.0))
        Call(SetAnimation, ACTOR_SELF, LVar0, LVar1)
        Call(RunToGoal, ACTOR_SELF, 0, FALSE)
    EndIf
    IfEq(LVarB, 180)
        Loop(15)
            Sub(LVarB, 12)
            Call(SetActorYaw, ACTOR_SELF, LVarB)
            Wait(1)
        EndLoop
    EndIf
    Call(SetActorYaw, ACTOR_SELF, 0)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    ExecWait(EVS_TakeTurn_Inner)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn_Inner = {
    Call(AddActorVar, ACTOR_SELF, AVAR_TurnCount, 1)
    ExecWait(EVS_UseAttack)
    Return
    End
};

#define LBL_JUMP_ATTACK 0
#define LBL_FIRE_BALL_ATTACK 1
EvtScript EVS_UseAttack = {
    Call(GetActorVar, ACTOR_SELF, AVAR_AxeSwitch, LVar3)
    IfEq(LVar3, FALSE)
        Call(RandInt, 100, LVar1)
        Switch(LVar1)
            CaseLt(50)
                Set(LVar0, LBL_JUMP_ATTACK)
            CaseDefault
                Set(LVar0, LBL_FIRE_BALL_ATTACK)
        EndSwitch
        // Set(LVar0, LBL_FIRE_BALL_ATTACK)
        // IfEq(LVar0, LBL_JUMP_ATTACK)
        //     DebugPrintf("Attack: %s\n", "Jump")
        // Else
        //     DebugPrintf("Attack: %s\n", "Fire Ball")
        // EndIf
        Switch(LVar0)
            CaseEq(LBL_JUMP_ATTACK)
                ExecWait(EVS_Attack_Jump)
            CaseEq(LBL_FIRE_BALL_ATTACK)
                ExecWait(EVS_Attack_FireBall)
        EndSwitch
    Else
        Return
    EndIf
    Return
    End
};

EvtScript EVS_Attack_Jump = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfNotFlag(LVar0, STATUS_FLAG_SHRINK)
        Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
        Call(SetBattleCamDist, 300)
        Call(SetBattleCamOffsetY, 36)
        Call(BattleCamTargetActor, ACTOR_SELF)
        Call(MoveBattleCamOver, 40)
        Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    Else
        Call(UseBattleCamPreset, BTL_CAM_ENEMY_APPROACH)
        Call(SetBattleCamDist, 240)
        Call(SetBattleCamOffsetY, 14)
        Call(BattleCamTargetActor, ACTOR_SELF)
        Call(MoveBattleCamOver, 40)
        Call(SetBattleCamTargetingModes, BTL_CAM_YADJ_TARGET, BTL_CAM_XADJ_AVG, FALSE)
    EndIf
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Walk)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(AddGoalPos, ACTOR_SELF, 60, 0, 0)
    Call(SetActorSpeed, ACTOR_SELF, Float(4.5))
    Call(RunToGoal, ACTOR_SELF, 0, FALSE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Idle)
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PreJump)
    Wait(3)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Jump)
    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.2))
    Call(EnemyTestTarget, ACTOR_SELF, LVarA, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVarA)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 30)
            Set(LVar1, 0)
            Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 18, FALSE, TRUE, FALSE)
            Thread
                Call(N(StartRumbleWithParams), 80, 14)
                Call(ShakeCam, CAM_BATTLE, 0, 4, Float(3.0))
            EndThread
            Sub(LVar0, 35)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(JumpToGoal, ACTOR_SELF, 14, FALSE, TRUE, FALSE)
            Thread
                Call(N(StartRumbleWithParams), 80, 14)
                Call(ShakeCam, CAM_BATTLE, 0, 4, Float(2.0))
            EndThread
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PostJump)
            Wait(3)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Idle)
            Wait(25)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Call(SetActorSpeed, ACTOR_SELF, Float(6.0))
            Set(LVar1, ANIM_BattleBowser_Walk)
            ExecWait(EVS_ReturnHome)
            Return
        EndCaseGroup
        CaseDefault
    EndSwitch
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(JumpToGoal, ACTOR_SELF, 15, FALSE, TRUE, FALSE)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 4, Float(3.0))
    EndThread
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Land)
    // Call(SetDamageSource, DMG_SRC_CRUSH)
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, 0, 0, 0, dmgJump, BS_FLAGS1_TRIGGER_EVENTS)
    // Call(LandJump, ACTOR_SELF)
    Call(N(StartRumbleWithParams), 80, 14)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 40)
    Set(LVar1, 0)
    Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(JumpToGoal, ACTOR_SELF, 18, FALSE, TRUE, FALSE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_HEAVY_NPC_LANDING)
    Thread
        Call(N(StartRumbleWithParams), 80, 14)
        Call(ShakeCam, CAM_BATTLE, 0, 4, Float(3.0))
    EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PostJump)
    Wait(3)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Idle)
    Wait(15)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
        CaseOrEq(HIT_RESULT_10)
            IfEq(LVarF, HIT_RESULT_10)
                Return
            EndIf
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(SetActorSpeed, ACTOR_SELF, Float(6.0))
            Set(LVar1, ANIM_BattleBowser_Walk)
            ExecWait(EVS_ReturnHome)
        EndCaseGroup
    EndSwitch
    Return
    End
};

// EvtScript N(EVS_AttackMissed) = {
//     Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Tantrum)
//     Thread
//         Wait(5)
//         Loop(4)
//             Call(N(StartRumbleWithParams), 80, 14)
//             Call(ShakeCam, CAM_BATTLE, 0, 2, Float(2.0))
//             Wait(4)
//         EndLoop
//         Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Idle)
//     EndThread
//     Return
//     End
// };

EvtScript EVS_Attack_FireBall = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 30)
    Add(LVar1, 65)
    Call(SetBattleCamTarget, LVar0, LVar1, LVar2)
    Call(SetBattleCamDist, 400)
    Call(MoveBattleCamOver, 40)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PreFireBreath)
    Wait(30)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_FireBreathStill)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(MoveBattleCamOver, 55)
    Wait(15)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)

    Wait(15)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyTestTarget, ACTOR_SELF, LVarA, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVarA)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            // Wait(10)
            // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_FireBreathLoop)
            // Wait(30)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PostFireBreath)
            Wait(2)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Idle)
            // Wait(15)
            // ExecWait(N(EVS_AttackMissed))
            IfEq(LVarA, HIT_RESULT_LUCKY)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Wait(15)
            Return
        EndCaseGroup
        CaseDefault
    EndSwitch
    Wait(2)
    // DebugPrintf("Guard Now!\n")
    // DebugPrintf("Frame: %s\n", "1/3")
    // Wait(1)
    // DebugPrintf("Guard Now!\n")
    // DebugPrintf("Frame: %s\n", "2/3")
    // Wait(1)
    // DebugPrintf("Guard Now!\n")
    // DebugPrintf("Frame: %s\n", "3/3")
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, DAMAGE_TYPE_FIRE, SUPPRESS_EVENT_ALL, 0, dmgFireBall, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
        CaseOrEq(HIT_RESULT_10)
            // Wait(10)
            // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_FireBreathLoop)
            // Wait(30)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_PostFireBreath)
            Wait(15)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BattleBowser_Idle)
            IfEq(LVarF, HIT_RESULT_10)
                Return
            EndIf
        EndCaseGroup
    EndSwitch
    Return
    End
};

}; // namespace false_bowser

namespace axe_switch {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_ActivateSwitch;


enum ActorPartIDs {
    PRT_MAIN            = 1,
};

enum ActorVars {
    AVAR_TurnCount = 0,
};

// Actor Stats
constexpr s32 hp = 99;

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
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 20 },
        .opacity = 255,
        .idleAnimations = NULL,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

EvtScript EVS_Init = {
    Call(MakeEntity, Ref(Entity_AxeSwitch), 179, 19, 0, 0, MAKE_ENTITY_END)
    Call(SetActorVar, ACTOR_SELF, AVAR_TurnCount, 0)
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
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

API_CALLABLE((ForceTriggerAxeSwitchEntity)) {
    BattleStatus* battleStatus = &gBattleStatus;
    Actor* player = battleStatus->playerActor;
    CollisionStatus* collisionStatus = &gCollisionStatus;
    PlayerStatus* playerStatus = &gPlayerStatus;

    if (player->scalingFactor == 1.0) {
        s32 entityID = script->varTable[10];
        get_entity_by_index(entityID);
        collisionStatus->curFloor = entityID | COLLISION_WITH_ENTITY_BIT;
        playerStatus->flags |= PS_FLAG_JUMPING;
        update_entities();
        collisionStatus->curFloor = -1;
        playerStatus->flags &= ~PS_FLAG_JUMPING;
        return ApiStatus_DONE2;
    }

    return ApiStatus_DONE2;
}

EvtScript EVS_HandleEvent = {
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
        CaseOrEq(EVENT_DEATH)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Call(SetActorVar, ACTOR_ENEMY0, false_bowser::AVAR_AxeSwitch, TRUE)
            Call((ForceTriggerAxeSwitchEntity))
            ExecWait(EVS_ActivateSwitch)
            Return
        EndCaseGroup
    EndSwitch
    Return
    End
};

EvtScript EVS_ActivateSwitch = {
    // Call((ForceTriggerAxeSwitchEntity))
    Set(LVar0, 0)
    Loop(10)
        Add(LVar0, 18)
        Call(RotateGroup, MODEL_AxeGroup, LVar0, 0, 0, 1)
        Wait(1)
    EndLoop
    Call(GetModelCenter, MODEL_AxeChain)
    PlayEffect(EFFECT_SHATTERING_STONES, 0, LVar0, LVar1, LVar2)
    Loop(5)
        Call(TranslateModel, MODEL_AxeChain, 1, 1, 1)
        Wait(1)
        Call(TranslateModel, MODEL_AxeChain, 0, 0, 0)
        Wait(1)
        Call(TranslateModel, MODEL_AxeChain, -1, -1, -1)
        Wait(1)
        Call(TranslateModel, MODEL_AxeChain, 0, 0, 0)
        Wait(1)
    EndLoop
    Wait(15)
    Call(EnableModel, MODEL_AxeChain, FALSE)
    // Loop(5)
    //     Call(TranslateGroup, MODEL_Bridge10, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge9, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge8, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge7, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge6, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge5, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge4, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge3, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge2, 1, 1, 1)
    //     Call(TranslateGroup, MODEL_Bridge1, 1, 1, 1)
    //     Wait(1)
    //     Call(TranslateGroup, MODEL_Bridge10, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge9, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge8, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge7, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge6, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge5, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge4, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge3, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge2, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge1, 0, 0, 0)
    //     Wait(1)
    //     Call(TranslateGroup, MODEL_Bridge10, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge9, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge8, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge7, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge6, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge5, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge4, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge3, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge2, -1, -1, -1)
    //     Call(TranslateGroup, MODEL_Bridge1, -1, -1, -1)
    //     Wait(1)
    //     Call(TranslateGroup, MODEL_Bridge10, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge9, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge8, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge7, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge6, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge5, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge4, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge3, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge2, 0, 0, 0)
    //     Call(TranslateGroup, MODEL_Bridge1, 0, 0, 0)
    //     Wait(1)
    // EndLoop
    Call(EnableGroup, MODEL_Bridge10, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge9, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge8, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge7, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge6, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge5, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge4, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge3, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge2, FALSE)
    Wait(2)
    Call(EnableGroup, MODEL_Bridge1, FALSE)
    Wait(2)
    Call(SetActorJumpGravity, ACTOR_ENEMY0, Float(1.2))
    Call(SetGoalPos, ACTOR_ENEMY0, LVar0, -130, LVar2)
    Call(FallToGoal, ACTOR_ENEMY0, 10)
    Wait(5)
    Call(PlaySoundAtActor, ACTOR_ENEMY0, SOUND_ACTOR_DEATH)
    Call(DropStarPoints, ACTOR_ENEMY0)
    Call(RemoveActor, ACTOR_ENEMY0)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Return
    End
};

}; // namespace axe_switch

ActorBlueprint FalseBowser = {
    .flags = 0,
    .maxHP = false_bowser::hp,
    .type = ACTOR_TYPE_FALSE_BOWSER,
    .level = ACTOR_LEVEL_FALSE_BOWSER,
    .partCount = ARRAY_COUNT(false_bowser::ActorParts),
    .partsData = false_bowser::ActorParts,
    .initScript = &false_bowser::EVS_Init,
    .statusTable = false_bowser::StatusTable,
    .escapeChance = 70,
    .airLiftChance = 90,
    .hurricaneChance = 90,
    .spookChance = 90,
    .upAndAwayChance = 95,
    .spinSmashReq = 0,
    .powerBounceChance = 100,
    .coinReward = 1,
    .size = { 80, 82 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -15, 32 },
    .statusTextOffset = { 5, 32 },
};

ActorBlueprint AxeSwitch = {
    .flags = ACTOR_FLAG_INVISIBLE | ACTOR_FLAG_NO_SHADOW | ACTOR_FLAG_TARGET_ONLY | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_DMG_POPUP,
    .maxHP = axe_switch::hp,
    .type = ACTOR_TYPE_AXE_SWITCH,
    .level = ACTOR_LEVEL_AXE_SWITCH,
    .partCount = ARRAY_COUNT(axe_switch::ActorParts),
    .partsData = axe_switch::ActorParts,
    .initScript = &axe_switch::EVS_Init,
    .statusTable = axe_switch::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 0,
    .coinReward = 1,
    .size = { 24, 24 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { 0, 0 },
    .statusTextOffset = { 0, 0 },
};

}; // namespace battle::actor
