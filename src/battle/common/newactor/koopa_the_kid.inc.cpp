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
#include "dx/debug_menu.h"

namespace battle::actor {

namespace koopa_the_kid {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_TakeTurn_Inner;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TestPhase;
extern EvtScript EVS_Attack_KoopaGangSpit;
extern EvtScript EVS_Attack_SteelyDrop;
extern EvtScript EVS_UseAttack;
extern EvtScript EVS_Hit;
extern EvtScript EVS_Death;
extern EvtScript EVS_NoDamageHit;
extern EvtScript EVS_BasicHit;
extern EvtScript EVS_BurnHit;
extern EvtScript EVS_ReturnHome;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_STEELY      = 2,
};

// Actor Stats
constexpr s32 hp = 30;
constexpr s32 dmgKoopaGangSpit = 1;
constexpr s32 dmgSteelyDrop = 1;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaTheKid_Idle,
    STATUS_KEY_STONE,     ANIM_KoopaTheKid_Still,
    STATUS_KEY_SLEEP,     ANIM_KoopaTheKid_Idle,
    STATUS_KEY_POISON,    ANIM_KoopaTheKid_Idle,
    STATUS_KEY_STOP,      ANIM_KoopaTheKid_Still,
    STATUS_KEY_STATIC,    ANIM_KoopaTheKid_Still,
    STATUS_KEY_PARALYZE,  ANIM_KoopaTheKid_Still,
    STATUS_KEY_PARALYZE,  ANIM_KoopaTheKid_Still,
    STATUS_KEY_DIZZY,     ANIM_KoopaTheKid_Idle,
    STATUS_KEY_DIZZY,     ANIM_KoopaTheKid_Idle,
    STATUS_END,
};

s32 SteelyAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaTheKid_BigSteely,
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
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_BowserPhase_KoopaGangSpitAttack, true)
    ExecWait(EVS_TestPhase)
    Return
    End
};

EvtScript EVS_TestPhase = {
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Koopa Kid Actor ID: (%d)\n", LVar9)
    // Wait(30)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, false)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, false)

    // Call(SetActorFlagBits, ACTOR_ENEMY1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, false)
    // Call(SetPartFlagBits, ACTOR_ENEMY1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, false)

    // Call(SetActorFlagBits, ACTOR_ENEMY2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, false)
    // Call(SetPartFlagBits, ACTOR_ENEMY2, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, false)
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
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_BEGIN_FIRST_STRIKE)
        CaseEq(EVENT_HIT_COMBO)
            Set(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Hit)
            Return
        CaseEq(EVENT_HIT)
            Set(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_BasicHit)
            Return
        CaseEq(EVENT_DEATH)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Hit)
            Wait(10)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Death)
            Return
        CaseEq(EVENT_BURN_HIT)
            Set(LVar0, PRT_MAIN)
            Set(LVar1, ANIM_KoopaTheKid_Hurt)
            Set(LVar2, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_BurnHit)
            Return
        CaseEq(EVENT_BURN_DEATH)
            Set(LVar0, PRT_MAIN)
            Set(LVar1, ANIM_KoopaTheKid_Hurt)
            Set(LVar2, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Hurt)
            ExecWait(EVS_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Set(LVar1, ANIM_KoopaTheKid_Idle)
            ExecWait(EVS_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            Set(LVar1, ANIM_KoopaTheKid_Idle)
            ExecWait(EVS_Enemy_NoDamageHit)
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaTheKid_Idle)
            ExecWait(EVS_Enemy_Recover)
        CaseDefault
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

EvtScript EVS_BasicHit = {
    ExecWait(EVS_Hit)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
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
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(ActorExists, ACTOR_KOOPA_GANG, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_KOOPA_GANG, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_KOOPA_GANG)
                Call(EnableIdleScript, ACTOR_KOOPA_GANG, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_KOOPA_GANG, false)
                // Call(SetAnimation, ACTOR_KOOPA_GANG, PRT_MAIN, ANIM_ChainChomp_Hurt)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_KOOPA_GANG, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_KOOPA_GANG, 0)
                Call(GetActorPos, ACTOR_KOOPA_GANG, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                Call(PlaySoundAtActor, ACTOR_KOOPA_GANG, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_KOOPA_GANG, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_KOOPA_GANG)
            EndThread
        EndIf
    EndIf
    // Call(ActorExists, ACTOR_GREEN_HAMMER_BRO, LVar2)
    // IfNe(LVar2, false)
    //     Call(GetActorHP, ACTOR_GREEN_HAMMER_BRO, LVar2)
    //     IfNe(LVar2, 0)
    //         Thread
    //             Call(HideHealthBar, ACTOR_GREEN_HAMMER_BRO)
    //             Call(EnableIdleScript, ACTOR_GREEN_HAMMER_BRO, IDLE_SCRIPT_DISABLE)
    //             Call(UseIdleAnimation, ACTOR_GREEN_HAMMER_BRO, false)
    //             Call(SetAnimation, ACTOR_GREEN_HAMMER_BRO, PRT_MAIN, ANIM_HammerBrosSMB3_Anim_0E)
    //             Wait(10)
    //             Set(LVar2, 0)
    //             Loop(24)
    //                 Call(SetActorYaw, ACTOR_GREEN_HAMMER_BRO, LVar2)
    //                 Add(LVar2, 30)
    //                 Wait(1)
    //             EndLoop
    //             Call(SetActorYaw, ACTOR_GREEN_HAMMER_BRO, 0)
    //             Call(GetActorPos, ACTOR_GREEN_HAMMER_BRO, LVar0, LVar1, LVar2)
    //             Add(LVar1, 10)
    //             PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
    //             Call(PlaySoundAtActor, ACTOR_GREEN_HAMMER_BRO, SOUND_ACTOR_DEATH)
    //             Set(LVar3, 0)
    //             Loop(12)
    //                 Call(SetActorRotation, ACTOR_GREEN_HAMMER_BRO, LVar3, 0, 0)
    //                 Add(LVar3, 8)
    //                 Wait(1)
    //             EndLoop
    //             Call(RemoveActor, ACTOR_GREEN_HAMMER_BRO)
    //         EndThread
    //     EndIf
    // EndIf
    // ExecWait(EVS_Enemy_DeathWithoutRemove)
    Label(0)
        Call(ActorExists, ACTOR_KOOPA_GANG, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
        // Call(ActorExists, ACTOR_GREEN_HAMMER_BRO, LVar0)
        // IfNe(LVar0, false)
        //     Wait(1)
        //     Goto(0)
        // EndIf
    Set(LVar2, EXEC_DEATH_NO_SPINNING)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_DEATH)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, true)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_SHADOW, true)
    Wait(30)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    // Call(RemoveActor, ACTOR_SELF)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_APPLY, true)
    Call(SetBattleFlagBits, BS_FLAGS1_DISABLE_CELEBRATION | BS_FLAGS1_BATTLE_FLED, true)
    Call(SetEndBattleFadeOutRate, 20)
    Return
    End
};

EvtScript EVS_BurnHit = {
    Call(GetLastEvent, ACTOR_SELF, LVar3)
    IfEq(LVar3, 36)
        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 1)
        Call(SetPartEventBits, ACTOR_SELF, PRT_MAIN, ACTOR_EVENT_FLAG_ENCHANTED, false)
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
                    Call(JumpToGoal, ACTOR_SELF, 8, false, false, false)
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
                        Call(JumpToGoal, ACTOR_SELF, 15, false, true, false)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 10, false, true, false)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 5, false, true, false)
                    Else
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 15, false, false, false)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 10, false, false, false)
                        Call(SetGoalPos, ACTOR_SELF, LVar4, LVar5, LVar6)
                        Call(JumpToGoal, ACTOR_SELF, 5, false, false, false)
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
        Call(RunToGoal, ACTOR_SELF, 0, false)
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
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    ExecWait(EVS_TakeTurn_Inner)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

EvtScript EVS_TakeTurn_Inner = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    ExecWait(EVS_UseAttack)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

// #define LBL_JUMP_ATTACK 0
// #define LBL_FIRE_BALL_ATTACK 1
#define LBL_ENDTURN 0
EvtScript EVS_UseAttack = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetActorVar, ACTOR_SELF, AVAR_BowserPhase_KoopaGangSpitAttack, LVar0)
    IfEq(LVar0, true)
        ExecWait(EVS_Attack_KoopaGangSpit)
        Goto(LBL_ENDTURN)
    Else
        ExecWait(EVS_Attack_SteelyDrop)
        Goto(LBL_ENDTURN)
    EndIf
    Label(LBL_ENDTURN)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};
#undef LBL_ENDTURN

EvtScript EVS_Attack_KoopaGangSpit = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Hide)
    Wait(24)
    // Call(ActorExists, ACTOR_ENEMY1, LVar3)
    // IfEq(LVar3, true)
    //     Call(UseIdleAnimation, ACTOR_ENEMY1, false)
    //     Call(EnableIdleScript, ACTOR_ENEMY1, IDLE_SCRIPT_DISABLE)
    //     Call(GetActorPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //     Add(LVar0, 65)
    //     Add(LVar1, 60)
    //     Sub(LVar2, 2)
    //     Call(SetGoalPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //     Call(SetAnimation, ACTOR_ENEMY1, PRT_MAIN, ANIM_KoopaGang_Red_ShellSpin)
    //     // Call(SetActorJumpGravity, ACTOR_ENEMY1, Float(1.6))
    //     Call(SetActorJumpGravity, ACTOR_ENEMY1, Float(2.0))
    //     Call(JumpToGoal, ACTOR_ENEMY1, 10, false, true, false)
    // EndIf
    // Wait(10)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Close)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_SuckUp)
    // Wait(20)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Shake)
    // Wait(20)
    // Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_IGNORE_DEFENSE, 0, 0, BS_FLAGS1_INCLUDE_POWER_UPS)
    // Switch(LVar0)
    //     CaseEq(HIT_RESULT_LUCKY)
    //         Call(GetActorPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //         Sub(LVar0, 15)
    //         Add(LVar2, 2)
    //         Call(SetActorPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //         Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_SpitOut)
    //         Call(SetGoalToTarget, ACTOR_ENEMY1)
    //         Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    //         Add(LVar0, -5)
    //         Add(LVar1, 20)
    //         // Add(LVar2, 0)
    //         Call(SetGoalPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //         Call(SetActorJumpGravity, ACTOR_ENEMY1, Float(0.01))
    //         Call(SetActorSpeed, ACTOR_ENEMY1, Float(12.0))
    //         Call(SetAnimation, ACTOR_ENEMY1, PRT_MAIN, ANIM_KoopaGang_Red_ShellSpin)
    //         Call(FlyToGoal, ACTOR_ENEMY1, 0, 0, EASING_COS_IN_OUT)
    //         Wait(2)
    //         Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
    //         Return
    //     CaseEq(HIT_RESULT_MISS)
    // EndSwitch
    // Call(GetActorPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    // Sub(LVar0, 15)
    // Add(LVar2, 2)
    // Call(SetActorPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_SpitOut)
    // Call(SetGoalToTarget, ACTOR_ENEMY1)
    // Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    // Add(LVar1, 25)
    // // Add(LVar2, 0)
    // Call(SetGoalPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    // Call(SetActorJumpGravity, ACTOR_ENEMY1, Float(0.01))
    // Call(SetActorSpeed, ACTOR_ENEMY1, Float(12.0))
    // Call(SetAnimation, ACTOR_ENEMY1, PRT_MAIN, ANIM_KoopaGang_Red_ShellSpin)
    // Call(FlyToGoal, ACTOR_ENEMY1, 0, 0, EASING_COS_IN_OUT)
    // Wait(2)
    // Call(EnemyDamageTarget, ACTOR_SELF, LVar0, 0, SUPPRESS_EVENT_ALL, 0, dmgKoopaGangSpit, BS_FLAGS1_TRIGGER_EVENTS)
    // Switch(LVar0)
    //     CaseOrEq(HIT_RESULT_HIT)
    //     CaseOrEq(HIT_RESULT_NO_DAMAGE)
    //         Call(GetActorPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //         Call(SetGoalToHome, ACTOR_ENEMY1)
    //         Call(GetGoalPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //         Call(SetGoalPos, ACTOR_ENEMY1, LVar0, LVar1, LVar2)
    //         Call(SetActorJumpGravity, ACTOR_ENEMY1, Float(2.0))
    //         Call(JumpToGoal, ACTOR_ENEMY1, 10, false, true, false)
    //         Wait(10)
    //         Call(YieldTurn)
    //     EndCaseGroup
    // EndSwitch
    // Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    // Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_LAUNCH_SHELL)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

EvtScript EVS_Attack_SteelyDrop = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_Hide)
    Wait(24)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaTheKid_ClownCarStill)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 6)
    Add(LVar1, 24)
    Call(SetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
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
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Add(LVar0, 6)
            Sub(LVar1, 24)
            Call(SetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
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

// ActorBlueprint KoopaGang = {
//     .flags = ACTOR_FLAG_NO_DMG_POPUP | ACTOR_FLAG_NO_HEALTH_BAR,
//     .maxHP = koopa_gang::hp,
//     .type = ACTOR_TYPE_KOOPA_GANG,
//     .level = ACTOR_LEVEL_KOOPA_GANG,
//     .partCount = ARRAY_COUNT(koopa_gang::ActorParts),
//     .partsData = koopa_gang::ActorParts,
//     .initScript = &koopa_gang::EVS_Init,
//     .statusTable = koopa_gang::StatusTable,
//     .escapeChance = 0,
//     .airLiftChance = 0,
//     .hurricaneChance = 0,
//     .spookChance = 0,
//     .upAndAwayChance = 0,
//     .spinSmashReq = 0,
//     .powerBounceChance = 70,
//     .coinReward = 0,
//     .size = { 38, 42 },
//     .healthBarOffset = { 0, 0 },
//     .statusIconOffset = { -10, 20 },
//     .statusTextOffset = { 10, 20 },
// };

}; // namespace battle::actor
