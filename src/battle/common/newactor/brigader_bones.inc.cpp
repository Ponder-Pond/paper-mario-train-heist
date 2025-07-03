#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/BrigaderBones.h"
#include "sprite/npc/HowitzerHal.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace brigader_bones {

extern s32 DefaultAnims[];
extern s32 HalAnims[];
extern s32 BulletAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_Move_Command;
extern EvtScript EVS_Attack_FireBullet;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_HAL         = 2,
    PRT_BULLET      = 3,
};

enum ActorVars {
    AVAR_CollapseTurns     = 3,
    AVAL_CollapseTurnZero  = 0,
    AVAL_CollapseTurnOne   = 1,
    AVAL_CollapseTurnTwo   = 2,
    AVAR_Collapsed         = 4,
};

// Actor Stats
constexpr s32 hp = 1;
constexpr s32 dmgImpact = 1;

#define BASE_COLLAPSE_DURATION  2

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BrigaderBones_Idle,
    STATUS_KEY_STONE,     ANIM_BrigaderBones_Still,
    STATUS_KEY_POISON,    ANIM_BrigaderBones_Idle,
    STATUS_KEY_STOP,      ANIM_BrigaderBones_Still,
    STATUS_KEY_STATIC,    ANIM_BrigaderBones_Idle,
    STATUS_KEY_PARALYZE,  ANIM_BrigaderBones_Still,
    STATUS_END,
};

s32 CollapsedAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BrigaderBones_StillDead,
    STATUS_END,
};

s32 HalAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HowitzerHal_Idle,
    STATUS_END,
};

s32 DisableAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HowitzerHal_Disable,
    STATUS_END,
};

s32 BulletAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HowitzerHal_BulletBill,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 CollapsedDefense[] = {
    ELEMENT_NORMAL,  99,
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
    STATUS_KEY_SHRINK,            100,
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
    STATUS_TURN_MOD_STOP,          -1,
    STATUS_END,
};

s32 CollapsedStatusTable[] = {
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
        .targetOffset = { -8, 30 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
    },
    {
        .flags = ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_DAMAGE_IMMUNE,
        .index = PRT_HAL,
        .posOffset = { -28, 0, -5 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = HalAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -9 },
    },
    {
        .flags = ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_BULLET,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = BulletAnims,
        .defenseTable = DefenseTable,
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
    Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_BrigaderCommand, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_Collapsed, FALSE)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(GetOriginalActorType, ACTOR_SELF, LVarA)
    Call(CreateHomeTargetList, TARGET_FLAG_2 | TARGET_FLAG_PRIMARY_ONLY)
    Call(InitTargetIterator)
    Label(0)
        Call(GetOwnerTarget, LVar0, LVar1)
        Call(GetOriginalActorType, LVar0, LVar2)
        IfNe(LVar2, LVarA)
            Call(GetActorHP, LVar0, LVar3)
            IfNe(LVar3, 0)
                Return
            EndIf
        EndIf
        Call(ChooseNextTarget, ITER_NEXT, LVar0)
        IfNe(LVar0, ITER_NO_MORE)
            Goto(0)
        EndIf
    Call(CreateHomeTargetList, TARGET_FLAG_2 | TARGET_FLAG_PRIMARY_ONLY)
    Call(InitTargetIterator)
    Label(1)
        Call(GetOwnerTarget, LVar0, LVar1)
        Call(GetActorVar, LVar0, AVAR_Collapsed, LVar3)
        IfNe(LVar3, TRUE)
            Return
        EndIf
        Call(ChooseNextTarget, ITER_NEXT, LVar0)
        IfNe(LVar0, ITER_NO_MORE)
            Goto(1)
        EndIf
    Call(CreateHomeTargetList, TARGET_FLAG_2 | TARGET_FLAG_PRIMARY_ONLY)
    Call(InitTargetIterator)
    Label(2)
        Call(GetOwnerTarget, LVar0, LVar1)
        Call(SetActorFlagBits, LVar0, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_NO_DMG_APPLY, TRUE)
        Call(ChooseNextTarget, ITER_NEXT, LVar0)
        IfNe(LVar0, ITER_NO_MORE)
            Goto(2)
        EndIf
    Return
    End
};

EvtScript EVS_Idle = {
    Label(0)
        Call(GetActorVar, ACTOR_SELF, AVAR_Collapsed, LVar0)
        Call(GetStatusFlags, ACTOR_SELF, LVar1)
        Switch(LVar0)
            CaseEq(0)
                Switch(LVar1)
                    CaseFlag(STATUS_FLAG_SLEEP)
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -6, 24)
                        Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, 1, -5)
                    CaseDefault
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -8, 30)
                        Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, -1, -10)
                EndSwitch
            CaseEq(1)
                Switch(LVar1)
                    CaseFlag(STATUS_FLAG_SLEEP)
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -1, 10)
                        Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 0)
                    CaseDefault
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -1, 10)
                        Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 0)
                EndSwitch
        EndSwitch
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_Collapse = {
    Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_BrigaderCommand, FALSE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_COLLAPSE)
    Call(SetAnimation, ACTOR_SELF, PRT_HAL, ANIM_HowitzerHal_Disable)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_Death)
    Wait(20)
    Call(SetActorVar, ACTOR_SELF, AVAR_Collapsed, TRUE)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_HAL, Ref(DisableAnims))
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(CollapsedAnims))
    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(CollapsedDefense))
    Call(SetStatusTable, ACTOR_SELF, Ref(CollapsedStatusTable))
    Call(ClearStatusEffects, ACTOR_SELF)
    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -1, 10)
    Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 0)
    Set(LVar0, BASE_COLLAPSE_DURATION)
    Call(SetActorVar, ACTOR_SELF, AVAR_CollapseTurns, LVar0)
    Call(GetOriginalActorType, ACTOR_SELF, LVarA)
    Call(CreateHomeTargetList, TARGET_FLAG_2 | TARGET_FLAG_PRIMARY_ONLY)
    Call(InitTargetIterator)
    Label(0)
        Call(GetOwnerTarget, LVar0, LVar1)
        Call(GetOriginalActorType, LVar0, LVar2)
        IfNe(LVar2, LVarA)
            Return
        EndIf
        Call(ChooseNextTarget, ITER_NEXT, LVar0)
        IfNe(LVar0, ITER_NO_MORE)
            Goto(0)
        EndIf
    Call(CreateHomeTargetList, TARGET_FLAG_2 | TARGET_FLAG_PRIMARY_ONLY)
    Call(InitTargetIterator)
    Label(1)
        Call(GetOwnerTarget, LVar0, LVar1)
        Call(GetActorVar, LVar0, AVAR_Collapsed, LVar3)
        IfNe(LVar3, TRUE)
            Return
        EndIf
        Call(ChooseNextTarget, ITER_NEXT, LVar0)
        IfNe(LVar0, ITER_NO_MORE)
            Goto(1)
        EndIf
    Call(CreateHomeTargetList, TARGET_FLAG_2 | TARGET_FLAG_PRIMARY_ONLY)
    Call(InitTargetIterator)
    Label(2)
        Call(GetOwnerTarget, LVar0, LVar1)
        Call(SetActorFlagBits, LVar0, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_NO_DMG_APPLY, TRUE)
        Call(ChooseNextTarget, ITER_NEXT, LVar0)
        IfNe(LVar0, ITER_NO_MORE)
            Goto(2)
        EndIf
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_HIT_COMBO)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BrigaderBones_Hurt)
            Exec(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_HAL)
            SetConst(LVar1, ANIM_HowitzerHal_Hurt)
            ExecWait(EVS_Enemy_Hit)
        CaseEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BrigaderBones_Hurt)
            Exec(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_HAL)
            SetConst(LVar1, ANIM_HowitzerHal_Hurt)
            ExecWait(EVS_Enemy_Hit)
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BrigaderBones_Hurt)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BrigaderBones_Hurt)
            ExecWait(EVS_Enemy_SpinSmashHit)
            ExecWait(EVS_Collapse)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Collapsed, LVar0)
            IfEq(LVar0, FALSE)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_BrigaderBones_Idle)
                Exec(EVS_Enemy_NoDamageHit)
                SetConst(LVar0, PRT_HAL)
                SetConst(LVar1, ANIM_HowitzerHal_Hurt)
                ExecWait(EVS_Enemy_NoDamageHit)
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_BrigaderBones_StillDead)
                Exec(EVS_Enemy_NoDamageHit)
                Call(SetActorVar, ACTOR_SELF, AVAR_CollapseTurns, AVAL_CollapseTurnTwo)
                SetConst(LVar0, PRT_HAL)
                SetConst(LVar1, ANIM_HowitzerHal_Hurt)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_BrigaderBones_Hurt)
            Exec(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_HAL)
            SetConst(LVar1, ANIM_HowitzerHal_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Call(GetActorVar, ACTOR_SELF, AVAR_Collapsed, LVar0)
            IfEq(LVar0, FALSE)
                Wait(10)
                ExecWait(EVS_Collapse)
            EndIf
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_Collapsed, LVar0)
            IfEq(LVar0, FALSE)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, ANIM_BrigaderBones_Idle)
                ExecWait(EVS_Enemy_Recover)
            EndIf
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
    // DebugPrintf("Turn Taken")
    Call(GetActorVar, ACTOR_SELF, AVAR_Collapsed, LVar0)
    IfEq(LVar0, TRUE)
        Call(GetActorVar, ACTOR_SELF, AVAR_CollapseTurns, LVar0)
        Switch(LVar0)
            CaseEq(AVAL_CollapseTurnTwo)
                Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_BrigaderCommand, FALSE)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Wait(10)
                Call(SetActorVar, ACTOR_SELF, AVAR_CollapseTurns, AVAL_CollapseTurnOne)
                // DebugPrintf("2 Turns Remaining...")
            CaseEq(AVAL_CollapseTurnOne)
                Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_BrigaderCommand, FALSE)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(4.0))
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Wait(10)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_StillDead)
                Wait(10)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Wait(10)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_StillDead)
                Wait(10)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Call(SetActorVar, ACTOR_SELF, AVAR_CollapseTurns, AVAL_CollapseTurnZero)
                Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(1.0))
                // DebugPrintf("1 Turn Remaining...")
            CaseEq(AVAL_CollapseTurnZero)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(8.0))
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Wait(10)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_StillDead)
                Wait(10)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Wait(10)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_StillDead)
                Wait(10)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Wait(10)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_StillDead)
                Wait(10)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_RATTLE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_DeathRattle)
                Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(1.0))
                Wait(10)
                Call(PlaySoundAtActor, ACTOR_SELF, SOUND_DRY_BONES_ARISE)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_Revive)
                Wait(20)
                Call(SetActorVar, ACTOR_SELF, AVAR_Collapsed, FALSE)
                Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefenseTable))
                Call(SetStatusTable, ACTOR_SELF, Ref(StatusTable))
                Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_DAMAGE_IMMUNE, FALSE)
                Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -8, 30)
                Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, -1, -10)
                ExecWait(EVS_Move_Command)
        EndSwitch
    Else
        ExecWait(EVS_Move_Command)
    EndIf
    Call(GetActorVar, ACTOR_BRIGADER_BONES, AVAR_GreenPhase_BrigaderCommand, LVar0)
    IfEq(LVar0, TRUE)
        Call(SetIdleAnimations, ACTOR_SELF, PRT_HAL, Ref(HalAnims))
        ExecWait(EVS_Attack_FireBullet)
    Else
        Call(SetIdleAnimations, ACTOR_SELF, PRT_HAL, Ref(DisableAnims))
    EndIf
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_Command = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_CommandStart)
    Wait(5)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BrigaderBones_CommandEnd)
    Wait(15)
    Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_BrigaderCommand, TRUE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_FireBullet = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(SetAnimation, ACTOR_SELF, PRT_HAL, ANIM_HowitzerHal_Shot)
    Wait(13)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 10, Float(1.0))
    EndThread
    Call(StartRumble, BTL_RUMBLE_PLAYER_HEAVY)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_HAL, SOUND_BULLET_BILL_FIRE)
    Call(GetPartPos, ACTOR_SELF, PRT_HAL, LVar0, LVar1, LVar2)
    Sub(LVar0, 33)
    Add(LVar1, 30)
    Add(LVar2, 3)
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 0, 2, 0)
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 2, 2, 0)
    Call(SetAnimation, ACTOR_SELF, PRT_BULLET, ANIM_HowitzerHal_BulletBill)
    Call(GetPartPos, ACTOR_SELF, PRT_HAL, LVar0, LVar1, LVar2)
    Add(LVar0, -30)
    Add(LVar1, 25)
    Add(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            // Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            // Wait(5)
            // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Sub(LVar0, 15)
            // Add(LVar1, 48)
            // Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BULLET, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BULLET, Float(10.0))
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(FlyPartTo, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
            Wait(2)
            IfEq(LVar0, HIT_RESULT_LUCKY)
                Call(SetGoalToTarget, ACTOR_SELF)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Call(YieldTurn)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            Return
        EndCaseGroup
    EndSwitch
    // Wait(5)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 15)
    // Add(LVar1, 48)
    // Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BULLET, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Add(LVar1, 25)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BULLET, Float(8.0))
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(FlyPartTo, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgImpact, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BULLET, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartPos, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace brigader_bones

ActorBlueprint BrigaderBones = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR,
    .maxHP = brigader_bones::hp,
    .type = ACTOR_TYPE_BRIGADER_BONES,
    .level = ACTOR_LEVEL_BRIGADER_BONES,
    .partCount = ARRAY_COUNT(brigader_bones::ActorParts),
    .partsData = brigader_bones::ActorParts,
    .initScript = &brigader_bones::EVS_Init,
    .statusTable = brigader_bones::StatusTable,
    .escapeChance = 50,
    .airLiftChance = 75,
    .hurricaneChance = 70,
    .spookChance = 10,
    .upAndAwayChance = 95,
    .spinSmashReq = 0,
    .powerBounceChance = 95,
    .coinReward = 1,
    .size = { 28, 32 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -15, 15 },
    .statusTextOffset = { 3, 27 },
};

}; // namespace battle::actor
