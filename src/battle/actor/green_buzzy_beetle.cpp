#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/BuzzyBeetle.h"
#include "sprite/npc/KoopaGang.h"
#include "boss.hpp"
#include "dx/debug_menu.h"

extern s32 ShellSpinAnims[];
extern s32 DefaultAnims[];
extern s32 ToppledAnims[];
extern s32 CannonAnims[];
extern s32 CannonSlowAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;

enum ActorPartIDs {
    PRT_BUZZY            = 1,
    PRT_CANNON           = 2,
};

enum ActorVars {
    AVAR_ToppleState        = 3,
    AVAL_State_Ground       = 1,
    AVAL_State_Toppled      = 2,
    AVAR_ToppleTurns        = 4,
    AVAL_ToppleTurnZero     = 0,
    AVAL_ToppleTurnOne      = 1,
};

// Actor Stats
constexpr s32 hp = 5;
constexpr s32 dmgShellToss = 2;

s32 UprightDefense[] = {
    ELEMENT_NORMAL,   4,
    ELEMENT_FIRE,    99,
    ELEMENT_BLAST,   99,
    ELEMENT_END,
};

s32 ToppledDefense[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              90,
    STATUS_KEY_POISON,             50,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              75,
    STATUS_KEY_UNUSED,                0,
    STATUS_KEY_STATIC,             50,
    STATUS_KEY_PARALYZE,           75,
    STATUS_KEY_SHRINK,             90,
    STATUS_KEY_STOP,               90,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,          0,
    STATUS_TURN_MOD_UNUSED,           0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,       0,
    STATUS_TURN_MOD_SHRINK,         0,
    STATUS_TURN_MOD_STOP,           0,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET | ACTOR_PART_FLAG_IGNORE_BELOW_CHECK,
        .index = PRT_BUZZY,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 16 },
        .opacity = 255,
        .idleAnimations = ShellSpinAnims,
        .defenseTable = UprightDefense,
        .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -9 },
    },
};

extern "C" export ActorBlueprint blueprint = {
    .flags = 0,
    .maxHP = hp,
    .type = ACTOR_TYPE_BUZZY_BEETLE_GREEN_PHASE,
    .level = ACTOR_LEVEL_BUZZY_BEETLE_GREEN_PHASE,
    .partCount = ARRAY_COUNT(ActorParts),
    .partsData = ActorParts,
    .initScript = &EVS_Init,
    .statusTable = StatusTable,
    .escapeChance = 60,
    .airLiftChance = 75,
    .hurricaneChance = 75,
    .spookChance = 75,
    .upAndAwayChance = 95,
    .spinSmashReq = 0,
    .powerBounceChance = 90,
    .coinReward = 1,
    .size = { 22, 18 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -8, 16 },
    .statusTextOffset = { 8, 13 },
};

s32 ShellSpinAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_ShellSpin,
    STATUS_END,
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Idle,
    STATUS_KEY_STONE,     ANIM_BuzzyBeetle_Still,
    STATUS_KEY_SLEEP,     ANIM_BuzzyBeetle_Sleep,
    STATUS_KEY_POISON,    ANIM_BuzzyBeetle_Idle,
    STATUS_KEY_STOP,      ANIM_BuzzyBeetle_Still,
    STATUS_KEY_STATIC,    ANIM_BuzzyBeetle_Idle,
    STATUS_KEY_PARALYZE,  ANIM_BuzzyBeetle_Still,
    STATUS_KEY_DIZZY,     ANIM_BuzzyBeetle_Dizzy,
    STATUS_KEY_UNUSED,    ANIM_BuzzyBeetle_Dizzy,
    STATUS_END,
};

s32 ToppledAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_ToppleIdle,
    STATUS_KEY_STONE,     ANIM_BuzzyBeetle_ToppleStill,
    STATUS_KEY_SLEEP,     ANIM_BuzzyBeetle_ToppleSleep,
    STATUS_KEY_POISON,    ANIM_BuzzyBeetle_ToppleIdle,
    STATUS_KEY_STOP,      ANIM_BuzzyBeetle_ToppleStill,
    STATUS_KEY_STATIC,    ANIM_BuzzyBeetle_ToppleIdle,
    STATUS_KEY_DIZZY,     ANIM_BuzzyBeetle_ToppleStruggle,
    STATUS_KEY_UNUSED,    ANIM_BuzzyBeetle_ToppleStruggle,
    STATUS_END,
};

s32 CannonAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Green_CannonIdle,
    STATUS_END,
};

s32 CannonSlowAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Green_CannonIdleSlow,
    STATUS_END,
};

#include "battle/common/SetAbsoluteStatusOffsets.inc.c"

EvtScript EVS_Init = {
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Ground)
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(HPBarToHome, ACTOR_SELF)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_BUZZY, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, true)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

// switch the anim on LVar1 if toppled
EvtScript EVS_CheckToppleAnim = {
    Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar3)
    IfEq(LVar3, AVAL_State_Toppled)
        Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
        Set(LVar1, LVar2)
    EndIf
    Return
    End
};

s32 FlipPosOffsets[] = { 7, 13, 17, 21, 23, 24, 23, 21, 17, 13, 7, 0,  4,  7,  6,  4,  0,  2,  0 };

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            SetConst(LVar2, ANIM_BuzzyBeetle_ToppleHurt)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfNe(LVar0, AVAL_State_Toppled)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_BurnHurt)
                SetConst(LVar2, ANIM_BuzzyBeetle_BurnHurt)
                ExecWait(EVS_Enemy_BurnHit)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleBurnHurt)
                SetConst(LVar2, ANIM_BuzzyBeetle_ToppleBurnHurt)
                ExecWait(EVS_Enemy_BurnHit)
            EndIf
        CaseEq(EVENT_BURN_DEATH)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfNe(LVar0, AVAL_State_Toppled)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_BurnHurt)
                SetConst(LVar2, ANIM_BuzzyBeetle_BurnHurt)
                ExecWait(EVS_Enemy_BurnHit)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_BurnHurt)
                ExecWait(EVS_Enemy_Death)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleBurnHurt)
                SetConst(LVar2, ANIM_BuzzyBeetle_ToppleBurnHurt)
                ExecWait(EVS_Enemy_BurnHit)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleBurnHurt)
                ExecWait(EVS_Enemy_Death)
            EndIf
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            SetConst(LVar2, ANIM_BuzzyBeetle_ToppleHurt)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_FLIP_TRIGGER)
            Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
            Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Toppled)
            Call(SetTargetOffset, ACTOR_SELF, PRT_BUZZY, 0, 16)
            Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_BUZZY, -1, -9)
            Call(SetActorVar, ACTOR_SELF, AVAR_ToppleTurns, AVAL_ToppleTurnOne)
            Call(SetDefenseTable, ACTOR_SELF, PRT_BUZZY, Ref(ToppledDefense))
            Call(SetIdleAnimations, ACTOR_SELF, PRT_BUZZY, Ref(ToppledAnims))
            Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_FLIPPED, true)
            Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Hurt)
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
            Thread
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -45)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -90)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -135)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -180)
                Wait(1)
            EndThread
            UseBuf(Ref(FlipPosOffsets))
            Loop(19)
                BufRead1(LVar0)
                Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
                Wait(1)
            EndLoop
            Call(SetActorDispOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
            Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_ToppleHurt)
            Call(EnableIdleScript, ACTOR_GREEN_BANDIT, IDLE_SCRIPT_DISABLE)
            Call(UseIdleAnimation, ACTOR_GREEN_BANDIT, false)
            Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, AVAL_GreenPhase_SlowCannonAttack)
            Call(SetAnimation, ACTOR_GREEN_BANDIT, PRT_CANNON, ANIM_KoopaGang_Green_CannonSlowDown)
            Wait(30)
            Call(SetIdleAnimations, ACTOR_GREEN_BANDIT, PRT_CANNON, Ref(CannonSlowAnims))
            Call(EnableIdleScript, ACTOR_GREEN_BANDIT, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_GREEN_BANDIT, true)
        CaseEq(EVENT_SHOCK_HIT)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            ExecWait(EVS_Enemy_Knockback)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Run)
            ExecWait(EVS_Enemy_ReturnHome)
        CaseEq(EVENT_SHOCK_DEATH)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_ZERO_DAMAGE)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ShellEnter)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_CeilingShellExit)
                Wait(8)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleIdle)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ShellEnter)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_ShellExit)
                Wait(8)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleIdle)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseEq(EVENT_SPIKE_TAUNT)
            Wait(10)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetStatusFlags, ACTOR_SELF, LVar3)
            IfFlag(LVar3, STATUS_FLAG_SHRINK)
                Add(LVar1, 9)
            Else
                Add(LVar1, 24)
            EndIf
            PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 20, 0)
            Wait(20)
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            SetConst(LVar2, ANIM_BuzzyBeetle_ToppleHurt)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            SetConst(LVar2, ANIM_BuzzyBeetle_ToppleHurt)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            SetConst(LVar2, ANIM_BuzzyBeetle_ToppleHurt)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            SetConst(LVar2, ANIM_BuzzyBeetle_ToppleHurt)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Idle)
                ExecWait(EVS_Enemy_Recover)
            EndIf
        CaseEq(EVENT_SCARE_AWAY)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Run)
                SetConst(LVar2, ANIM_BuzzyBeetle_Hurt)
                ExecWait(EVS_Enemy_ScareAway)
                Return
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleIdle)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Run)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleIdle)
            EndIf
            ExecWait(EVS_Enemy_AirLift)
        CaseEq(EVENT_BLOW_AWAY)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Hurt)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_FallDown)
            EndIf
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_AIR_LIFT_FAILED)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ShellEnter)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_ShellExit)
                Wait(8)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleIdle)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar3)
    IfEq(LVar3, AVAL_State_Toppled)
        Call(GetActorVar, ACTOR_SELF, AVAR_ToppleTurns, LVar0)
        Switch(LVar0)
            CaseEq(AVAL_ToppleTurnOne)
                Call(SetActorVar, ACTOR_SELF, AVAR_ToppleTurns, AVAL_ToppleTurnZero)
                Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
                Call(AddActorDecoration, ACTOR_SELF, PRT_BUZZY, 0, ACTOR_DECORATION_SWEAT)
                Wait(30)
                Call(RemoveActorDecoration, ACTOR_SELF, PRT_BUZZY, 0)
                Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
                Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, AVAL_GreenPhase_SlowCannonAttack)
            CaseEq(AVAL_ToppleTurnZero)
                Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
                Call(AddActorDecoration, ACTOR_SELF, PRT_BUZZY, 0, ACTOR_DECORATION_SWEAT)
                Wait(20)
                Call(RemoveActorDecoration, ACTOR_SELF, PRT_BUZZY, 0)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_ToppleIdle)
                SetConst(LVar2, ANIM_BuzzyBeetle_Idle)
                ExecWait(EVS_Enemy_FlipBackUp)
                Call(SetActorYaw, ACTOR_SELF, 0)
                Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Ground)
                Call(SetDefenseTable, ACTOR_SELF, PRT_BUZZY, Ref(UprightDefense))
                Call(SetIdleAnimations, ACTOR_SELF, PRT_BUZZY, Ref(DefaultAnims))
                Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_FLIPPED, false)
                Goto(0)
        EndSwitch
    Else
        Label(0)
        Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, AVAL_GreenPhase_FastCannonAttack)
        Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_ShellEnter)
        Wait(5)
        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SHELL_SPIN)
        Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(4.0))
        Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_ShellSpin)
        Call(SetIdleAnimations, ACTOR_SELF, PRT_BUZZY, Ref(ShellSpinAnims))
        Call(SetAnimation, ACTOR_GREEN_BANDIT, PRT_CANNON, ANIM_KoopaGang_Green_CannonIdle)
        Wait(10)
        Call(SetIdleAnimations, ACTOR_GREEN_BANDIT, PRT_CANNON, Ref(CannonAnims))
        Goto(1)
    EndIf
    Label(1)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};
