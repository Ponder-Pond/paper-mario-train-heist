#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "mapfs/trn_bt00_shape.h"
#include "sprite/npc/KoopaGang.h"
#include "sprite/npc/ChainChomp.h"
#include "sprite/npc/HammerBrosSMB3.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace yellow_bandit_koopa {

// Yellow Bandit Params
enum ThisBanditsParams {
    // THIS_ACTOR_ID               = ACTOR_ENEMY0,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_YELLOW_BANDIT,
    THIS_LEVEL                  = ACTOR_LEVEL_YELLOW_BANDIT,
    THIS_SLEEP_CHANCE           = 0,
    THIS_DIZZY_CHANCE           = 100,
    THIS_PARALYZE_CHANCE        = 0,
    THIS_ANIM_IDLE              = ANIM_KoopaGang_Yellow_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaGang_Yellow_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang_Yellow_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang_Yellow_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang_Yellow_HurtStill,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang_Yellow_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang_Yellow_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang_Yellow_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang_Yellow_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang_Yellow_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang_Yellow_PointForward,
};

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_ThirdPhaseTransition;
extern EvtScript EVS_Move_Cheer;
extern EvtScript EVS_Attack_ShellToss;

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

// Actor Stats
constexpr s32 hp = 1;
constexpr s32 dmgShellToss = 2;
constexpr s32 maxAttackBoost = 3;
constexpr s32 amtAttackBoost = 1;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   1,
    ELEMENT_END,
};

s32 ToppledDefense[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,               THIS_SLEEP_CHANCE,
    STATUS_KEY_POISON,              0,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,               THIS_DIZZY_CHANCE,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,            THIS_PARALYZE_CHANCE,
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
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET | ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
    STATUS_END,
};


#include "common/StartRumbleWithParams.inc.c"

BSS PlayerStatus DummyPlayerStatus;

API_CALLABLE(SpawnSpinEffect) {
    Bytecode* args = script->ptrReadPos;
    s32 posX = evt_get_variable(script, *args++);
    s32 posY = evt_get_variable(script, *args++);
    s32 posZ = evt_get_variable(script, *args++);
    s32 duration = evt_get_variable(script, *args++);

    DummyPlayerStatus.pos.x = posX;
    DummyPlayerStatus.pos.y = posY - 10.0f;
    DummyPlayerStatus.pos.z = posZ;

    fx_effect_46(6, &DummyPlayerStatus, 1.0f, duration);
    return ApiStatus_DONE2;
}

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(HPBarToHome, ACTOR_SELF)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, true)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, true)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_Scene_YellowPhase)
    Call(SetActorVar, ACTOR_SELF, AVAR_YellowPhase_ActorsSpawned, false)
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Yellow Bandit Actor ID: (%d)\n", LVar9)
    Return
    End
};

EvtScript EVS_Idle = {
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
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfEq(LVar0, 0)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                Wait(10)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Defeat)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            // Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            // IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_IDLE)
                ExecWait(EVS_Enemy_Recover)
            // EndIf
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(EnableBattleStatusBar, false)
    Call(ActorExists, ACTOR_GIANT_CHOMP, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_GIANT_CHOMP, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_GIANT_CHOMP)
                Call(EnableIdleScript, ACTOR_GIANT_CHOMP, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_GIANT_CHOMP, false)
                Call(SetAnimation, ACTOR_GIANT_CHOMP, PRT_MAIN, ANIM_ChainChomp_Hurt)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_GIANT_CHOMP, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_GIANT_CHOMP, 0)
                Call(GetActorPos, ACTOR_GIANT_CHOMP, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                Call(PlaySoundAtActor, ACTOR_GIANT_CHOMP, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_GIANT_CHOMP, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_GIANT_CHOMP)
            EndThread
        EndIf
    EndIf
    Call(ActorExists, ACTOR_YELLOW_HAMMER_BRO, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_YELLOW_HAMMER_BRO, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_YELLOW_HAMMER_BRO)
                Call(EnableIdleScript, ACTOR_YELLOW_HAMMER_BRO, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_YELLOW_HAMMER_BRO, false)
                Call(SetAnimation, ACTOR_YELLOW_HAMMER_BRO, PRT_MAIN, ANIM_HammerBrosSMB3_Alt_Anim_0E)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_YELLOW_HAMMER_BRO, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_YELLOW_HAMMER_BRO, 0)
                Call(GetActorPos, ACTOR_YELLOW_HAMMER_BRO, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                Call(PlaySoundAtActor, ACTOR_YELLOW_HAMMER_BRO, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_YELLOW_HAMMER_BRO, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_YELLOW_HAMMER_BRO)
            EndThread
        EndIf
    EndIf
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_HEALTH_BAR, true)
    Wait(10)
    Call(SetAnimation, ACTOR_YELLOW_BANDIT, PRT_MAIN, THIS_ANIM_HURT)
    Call(GetActorPos, ACTOR_YELLOW_BANDIT, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    Add(LVar2, 2)
    Call(PlaySoundAtActor, ACTOR_YELLOW_BANDIT, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(25)
    Label(0)
        Call(ActorExists, ACTOR_GIANT_CHOMP, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
        Call(ActorExists, ACTOR_YELLOW_HAMMER_BRO, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
    ExecWait(EVS_ThirdPhaseTransition)
    Return
    End
};

Vec3i BlackBanditSpawnPos = { 115, 10, 20 };

Formation SpawnBlackBandit = {
    ACTOR_BY_POS(BlackBanditKoopa, BlackBanditSpawnPos, 50),
};

Vec3i CrateSpawnPos = { 15, 0, 20 };

Formation SpawnCrate = {
    ACTOR_BY_POS(Crate, CrateSpawnPos, 100),
};

Vec3i DyanmiteCrateSpawnPos = { 55, 0, 15 };

Formation SpawnDyanmiteCrate = {
    ACTOR_BY_POS(DyanmiteCrate, DyanmiteCrateSpawnPos, 100),
};

Vec3i ShyGuyRider1SpawnPos = { 45, -25, -50 };

Formation SpawnShyGuyRider1 = {
    ACTOR_BY_POS(ShyGuyRider, ShyGuyRider1SpawnPos, 75),
};

Vec3i ShyGuyRider2SpawnPos = { -45, -25, -50 };

Formation SpawnShyGuyRider2 = {
    ACTOR_BY_POS(ShyGuyRider, ShyGuyRider2SpawnPos, 100),
};

// Vec3i PositionThirdGroup[] = {
//     { 115, 10, 20 },
//     { 15, 0, 20 },
//     { 55, 0, 15 },
//     { 45, -25, -50 },
//     { 45, -25, -50 },
// };

// Formation FormationThirdGroup = {
//     ACTOR_BY_POS(BlackBanditKoopa, PositionThirdGroup[0], 8),
//     ACTOR_BY_POS(Crate, PositionThirdGroup[1], 10),
//     ACTOR_BY_POS(DyanmiteCrate, PositionThirdGroup[2], 10),
//     ACTOR_BY_POS(ShyGuyRider, PositionThirdGroup[3], 9),
//     ACTOR_BY_POS(ShyGuyRider, PositionThirdGroup[4], 10),
// };

EvtScript EVS_ThirdPhaseTransition = {
    Call(CancelEnemyTurn, 1)
    Call(EnableModel, MODEL_Tunnel, true)
    Set(LFlag0, false)
    Set(LVarA, 0)
    Loop(0)
        Add(LVarA, 10)
        IfGt(LVarA, 2250)
            Set(LVarA, 0)
            BreakLoop
        EndIf
        IfGt(LVarA, 1000)
            IfFalse(LFlag0)
                Thread
                    Call(GetActorVar, ACTOR_SELF, AVAR_YellowPhase_ActorsSpawned, LVarB)
                    IfEq(LVarB, false)
                        Call(EnableModel, MODEL_BarrelBlack, true)
                        Call(EnableModel, MODEL_SnipingCrate, true)
                        Call(SummonEnemy, Ref(SpawnBlackBandit), false)
                        Call(SummonEnemy, Ref(SpawnCrate), false)
                        Call(SummonEnemy, Ref(SpawnDyanmiteCrate), false)
                        Call(SummonEnemy, Ref(SpawnShyGuyRider1), false)
                        Call(SummonEnemy, Ref(SpawnShyGuyRider2), false)
                        // Call(SummonEnemy, Ref(FormationThirdGroup), false)
                        Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_INVISIBLE, true)
                        Call(SetActorVar, ACTOR_SELF, AVAR_YellowPhase_ActorsSpawned, true)
                    EndIf
                EndThread
            EndIf
            Set(LFlag0, true)
        EndIf
        Call(TranslateModel, MODEL_Tunnel, LVarA, 0, 0)
        Wait(1)
    EndLoop
    Call(TranslateModel, MODEL_Tunnel, LVarA, 0, 0)
    Call(EnableModel, MODEL_Tunnel, false)
    Call(EnableBattleStatusBar, true)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(ActorExists, ACTOR_GIANT_CHOMP, LVar0)
    IfEq(LVar0, true)
        Call(GetStatusFlags, ACTOR_GIANT_CHOMP, LVar0)
        IfNotFlag(LVar0, STATUS_FLAG_DIZZY)
            Call(GetActorAttackBoost, ACTOR_YELLOW_HAMMER_BRO, LVar3)
            IfLt(LVar3, maxAttackBoost)
                ExecWait(EVS_Move_Cheer)
            Else
                ExecWait(EVS_Attack_ShellToss)
            EndIf
        Else
            ExecWait(EVS_Attack_ShellToss)
        EndIf
    Else
        ExecWait(EVS_Attack_ShellToss)
    EndIf
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

EvtScript EVS_Move_Cheer = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 20)
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Yellow_ThumbsUp)
    Wait(5)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 7)
    Add(LVar1, 28)
    Add(LVar2, 5)
    PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 30, 0)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SMALL_LENS_FLARE)
    Wait(30)
    Thread
        Wait(10)
        Call(PlaySoundAtActor, ACTOR_YELLOW_HAMMER_BRO, SOUND_MAGIKOOPA_POWER_UP)
    EndThread
    Thread
        Call(FreezeBattleState, true)
        Call(BoostAttack, ACTOR_YELLOW_HAMMER_BRO, amtAttackBoost, false)
        Call(FreezeBattleState, false)
    EndThread
    Call(WaitForBuffDone)
    Wait(5)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
    Wait(5)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(YieldTurn)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

EvtScript EVS_Attack_ShellToss = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SMALL_LENS_FLARE)
    Call(SetActorYaw, ACTOR_SELF, 0)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_POINT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 22)
    Add(LVar1, 19)
    PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 30, 0)
    Wait(30)
    Call(GetHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Set(LVar2, 15)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_RUN)
    Call(RunToGoal, ACTOR_SELF, 10, false)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_ENTER_SHELL)
    Wait(10)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_SPINUP)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SpawnSpinEffect, LVar0, LVar1, LVar2, 30)
    Wait(30)
    Thread
        Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_IGNORE_DEFENSE, 0, 0, BS_FLAGS1_INCLUDE_POWER_UPS)
        Switch(LVar0)
            CaseEq(HIT_RESULT_LUCKY)
                Wait(20)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
                Return
            CaseEq(HIT_RESULT_MISS)
                Return
        EndSwitch
        Label(0)
        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        Call(GetActorPos, ACTOR_PLAYER, LVar3, LVar4, LVar5)
        IfGt(LVar0, LVar3)
            Wait(1)
            Goto(0)
        EndIf
        Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
        Call(SetGoalToTarget, ACTOR_SELF)
        Call(EnemyDamageTarget, ACTOR_SELF, LVar0, 0, SUPPRESS_EVENT_ALL, 0, dmgShellToss, BS_FLAGS1_TRIGGER_EVENTS)
    EndThread
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_LAUNCH_SHELL)
    Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_WALK, SOUND_NONE, SOUND_NONE)
    Call(EnableActorBlur, ACTOR_SELF, ACTOR_BLUR_ENABLE)
    Call(SetActorSpeed, ACTOR_SELF, Float(16.0))
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetGoalPos, ACTOR_SELF, -160, LVar1, LVar2)
    Call(RunToGoal, ACTOR_SELF, 0, false)
    Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_WALK)
    Call(EnableActorBlur, ACTOR_SELF, ACTOR_BLUR_DISABLE)
    Thread
        Call(SetPartRotationOffset, ACTOR_SELF, PRT_MAIN, 0, 15, 0)
        Set(LVar0, 0)
        Loop(40)
            Sub(LVar0, 45)
            Call(SetPartRotation, ACTOR_SELF, PRT_MAIN, 0, 0, LVar0)
            Wait(1)
        EndLoop
        Call(SetPartRotationOffset, ACTOR_SELF, PRT_MAIN, 0, 0, 0)
        Call(SetPartRotation, ACTOR_SELF, PRT_MAIN, 0, 0, 0)
    EndThread
    Call(GetHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Set(LVar2, 15)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetActorJumpGravity, ACTOR_SELF, Float(0.5))
    Call(JumpToGoal, ACTOR_SELF, 40, false, true, false)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SpawnSpinEffect, LVar0, LVar1, LVar2, 30)
    Wait(30)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
    Wait(10)
    Call(SetAnimation, ACTOR_SELF, 1, ANIM_KoopaGang_Yellow_ThumbsUp)
    Wait(5)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 7)
    Add(LVar1, 28)
    Add(LVar2, 5)
    PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 30, 0)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SMALL_LENS_FLARE)
    Wait(30)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_RUN)
    Call(SetGoalToHome, ACTOR_SELF)
    Call(RunToGoal, ACTOR_SELF, 10, false)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetBattlePhase, LVar0)
    Switch(LVar0)
        CaseEq(PHASE_PLAYER_BEGIN)
            Call(GetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, LVar0)
            IfEq(LVar0, AVAL_Scene_YellowPhase)
                Call(EnableBattleStatusBar, false)
                Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
                Call(SetBattleCamTarget, 105, 15, 20)
                Call(SetBattleCamDist, 250)
                Call(SetBattleCamYaw, 0)
                Call(SetBattleCamOffsetY, 15)
                Call(MoveBattleCamOver, 20)
                Wait(20)
                Call(ActorSpeak, MSG_TrainHeist_YellowBattleStart, ACTOR_YELLOW_BANDIT, PRT_MAIN, ANIM_KoopaGang_Yellow_Talk, ANIM_KoopaGang_Yellow_Idle)
                Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_BlackPhase)
                Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
                Wait(20)
                Call(EnableBattleStatusBar, true)
            EndIf
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

}; // namespace yellow_bandit_koopa

ActorBlueprint YellowBanditKoopa = {
    .flags = 0, //ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = yellow_bandit_koopa::hp,
    .type = yellow_bandit_koopa::THIS_ACTOR_TYPE,
    .level = yellow_bandit_koopa::THIS_LEVEL,
    .partCount = ARRAY_COUNT(yellow_bandit_koopa::ActorParts),
    .partsData = yellow_bandit_koopa::ActorParts,
    .initScript = &yellow_bandit_koopa::EVS_Init,
    .statusTable = yellow_bandit_koopa::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 70,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
