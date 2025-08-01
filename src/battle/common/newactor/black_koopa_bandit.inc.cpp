#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "mapfs/trn_bt00_shape.h"
#include "sprite/npc/KoopaGang.h"
#include "sprite/npc/ShyGuyRider.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace black_bandit_koopa {

// Black Bandit Params
enum ThisBanditsParams {
    // THIS_ACTOR_ID               = ACTOR_ENEMY2,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_BLACK_BANDIT,
    THIS_LEVEL                  = ACTOR_LEVEL_BLACK_BANDIT,
    THIS_SLEEP_CHANCE           = 0,
    THIS_DIZZY_CHANCE           = 0,
    THIS_PARALYZE_CHANCE        = 0,
    THIS_ANIM_IDLE              = ANIM_KoopaGang_Black_BlackIdle,
    THIS_ANIM_STILL             = ANIM_KoopaGang_Black_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang_Black_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang_Black_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang_Black_HurtStill,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang_Black_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang_Black_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang_Black_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang_Black_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang_Black_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang_Black_PointForward,
};

extern s32 DefaultAnims[];
extern s32 ParaBeetleAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_ManageThirdPhase;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_FourthPhaseTransition;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Attack_SniperShot;

enum ActorPartIDs {
    PRT_MAIN            = 1,
    PRT_PARABEETLE      = 2,
};

// Actor Stats
constexpr s32 hp = 1;
constexpr s32 dmgParaBeetleShot = 4;

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
    STATUS_KEY_SHRINK,            100,
    STATUS_KEY_STOP,               75,
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
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_PARABEETLE,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = ParaBeetleAnims,
        .defenseTable = ToppledDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_END,
};

s32 ParaBeetleAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Black_ParaBeetle,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

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
    Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_Scene_BlackPhase)
    Call(SetActorVar, ACTOR_SELF, AVAR_BlackPhase_ActorsSpawned, false)
    Exec(EVS_ManageThirdPhase)
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Black Bandit Actor ID: (%d)\n", LVar9)
    Return
    End
};

EvtScript EVS_ManageThirdPhase = {
    Call(EnableModel, MODEL_SnipingCrate, true)
    Call(EnableModel, MODEL_BarrelBlack, true)
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
    Call(ActorExists, ACTOR_SHY_GUY_RIDER_1, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_SHY_GUY_RIDER_1, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_SHY_GUY_RIDER_1)
                Call(EnableIdleScript, ACTOR_SHY_GUY_RIDER_1, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_SHY_GUY_RIDER_1, false)
                // Call(SetAnimation, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ANIM_ShyGuyRider_Hurt)
                Wait(10)
                // Set(LVar2, 0)
                // Loop(24)
                //     Call(SetActorYaw, ACTOR_SHY_GUY_RIDER_1, LVar2)
                //     Add(LVar2, 30)
                //     Wait(1)
                // EndLoop
                // Call(SetActorYaw, ACTOR_SHY_GUY_RIDER_1, 0)
                // Call(GetActorPos, ACTOR_SHY_GUY_RIDER_1, LVar0, LVar1, LVar2)
                // Add(LVar1, 10)
                // PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                // Call(PlaySoundAtActor, ACTOR_SHY_GUY_RIDER_1, SOUND_ACTOR_DEATH)
                // Set(LVar3, 0)
                // Loop(12)
                //     Call(SetActorRotation, ACTOR_SHY_GUY_RIDER_1, LVar3, 0, 0)
                //     Add(LVar3, 8)
                //     Wait(1)
                // EndLoop
                Call(RemoveActor, ACTOR_SHY_GUY_RIDER_1)
            EndThread
        EndIf
    EndIf
    Call(ActorExists, ACTOR_SHY_GUY_RIDER_2, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_SHY_GUY_RIDER_2, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_SHY_GUY_RIDER_2)
                Call(EnableIdleScript, ACTOR_SHY_GUY_RIDER_2, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_SHY_GUY_RIDER_2, false)
                // Call(SetAnimation, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ANIM_HammerBrosSMB3_Alt_Anim_0E)
                Wait(10)
                // Set(LVar2, 0)
                // Loop(24)
                //     Call(SetActorYaw, ACTOR_SHY_GUY_RIDER_2, LVar2)
                //     Add(LVar2, 30)
                //     Wait(1)
                // EndLoop
                // Call(SetActorYaw, ACTOR_SHY_GUY_RIDER_2, 0)
                // Call(GetActorPos, ACTOR_SHY_GUY_RIDER_2, LVar0, LVar1, LVar2)
                // Add(LVar1, 10)
                // PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                // Call(PlaySoundAtActor, ACTOR_SHY_GUY_RIDER_2, SOUND_ACTOR_DEATH)
                // Set(LVar3, 0)
                // Loop(12)
                //     Call(SetActorRotation, ACTOR_SHY_GUY_RIDER_2, LVar3, 0, 0)
                //     Add(LVar3, 8)
                //     Wait(1)
                // EndLoop
                Call(RemoveActor, ACTOR_SHY_GUY_RIDER_2)
            EndThread
        EndIf
    EndIf
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_HEALTH_BAR, true)
    Wait(10)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    Add(LVar2, 2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(25)
    Label(0)
        Call(ActorExists, ACTOR_SHY_GUY_RIDER_1, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
        Call(ActorExists, ACTOR_SHY_GUY_RIDER_2, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
    ExecWait(EVS_FourthPhaseTransition)
    Return
    End
};

Vec3i RedBanditSpawnPos = { 115, 25, 20 };

Formation SpawnRedBandit = {
    ACTOR_BY_POS(RedBanditKoopa, RedBanditSpawnPos, 100),
};

Vec3i PyroGuySpawnPos = { 145, 55, 25 };

Formation SpawnPyroGuy = {
    ACTOR_BY_POS(PyroGuy, PyroGuySpawnPos, 75),
};

// Vec3i PositionFourthGroup[] = {
//     { 105, 0, 10 },
//     { 25, 0, 15 },
// };

// Formation FormationFourthGroup = {
//     ACTOR_BY_POS(RedBanditKoopa, PositionFourthGroup[0], 10),
//     ACTOR_BY_POS(PyroGuy, PositionFourthGroup[1], 9),
// };

EvtScript EVS_FourthPhaseTransition = {
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
                    Call(GetActorVar, ACTOR_SELF, AVAR_BlackPhase_ActorsSpawned, LVarB)
                    IfEq(LVarB, false)
                        Call(EnableModel, MODEL_BarrelBlack, false)
                        Call(EnableModel, MODEL_SnipingCrate, false)
                        Call(EnableModel, MODEL_BombBox, true)
                        Call(EnableModel, MODEL_BombPile, true)
                        Call(EnableModel, MODEL_BarrelRed, true)
                        Call(SummonEnemy, Ref(SpawnRedBandit), false)
                        Call(SummonEnemy, Ref(SpawnPyroGuy), false)
                        // Call(SummonEnemy, Ref(FormationFourthGroup), false)
                        Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_INVISIBLE, true)
                        Call(SetActorVar, ACTOR_SELF, AVAR_BlackPhase_ActorsSpawned, true)
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
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, false)
    ExecWait(EVS_Attack_SniperShot)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Attack_SniperShot = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR_FAR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 25)
    Call(SetBattleCamDist, 250)
    Thread
        Wait(20)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 20)
    EndThread
    // Wait(20)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Black_BlackAim)
    Wait(20)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 35)
    Add(LVar1, 28)
    Add(LVar2, 5)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Black_BlackFire)
    PlayEffect(EFFECT_BLAST, 0, LVar0, LVar1, LVar2, Float(0.75), 30, 0)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_PARABEETLE, SOUND_BOMB_BLAST)
    // Wait(3)
    Call(SetPartPos, ACTOR_SELF, PRT_PARABEETLE, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_PARABEETLE, ANIM_KoopaGang_Black_ParaBeetle)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_PARABEETLE, ACTOR_PART_FLAG_INVISIBLE, false)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 35)
    Add(LVar1, 28)
    Add(LVar2, 5)
    Call(SetPartPos, ACTOR_SELF, PRT_PARABEETLE, LVar0, LVar1, LVar2)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_PARABEETLE, ACTOR_PART_FLAG_INVISIBLE, false)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 4, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseEq(HIT_RESULT_MISS)
        CaseEq(HIT_RESULT_LUCKY)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Black_BlackIdle)
            Call(PlaySoundAtPart, ACTOR_SELF, PRT_PARABEETLE, SOUND_TOSS)
            Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, 0)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_PARABEETLE, Float(14.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_PARABEETLE, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_PARABEETLE, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
            Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Sub(LVar0, 65)
            Set(LVar1, 0)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_PARABEETLE, Float(6.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_PARABEETLE, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_PARABEETLE, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_PARABEETLE, ACTOR_PART_FLAG_INVISIBLE, true)
            // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_GeneralGuy_Anim02)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, true)
            Return
    EndSwitch
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Black_BlackIdle)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_PARABEETLE, SOUND_TOSS)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 15)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_PARABEETLE, Float(20.0))
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_PARABEETLE, Float(0.1))
    Call(FlyPartTo, ACTOR_SELF, PRT_PARABEETLE, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Add(LVar1, 20)
    // PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
    // Call(PlaySoundAtPart, ACTOR_SELF, PRT_PARABEETLE, SOUND_BOMB_BLAST)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_PARABEETLE, ACTOR_PART_FLAG_INVISIBLE, true)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgParaBeetleShot, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
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
            IfEq(LVar0, AVAL_Scene_BlackPhase)
                Call(EnableBattleStatusBar, false)
                Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
                Call(SetBattleCamTarget, 115, 20, 20)
                Call(SetBattleCamDist, 250)
                Call(SetBattleCamYaw, 0)
                Call(SetBattleCamOffsetY, 15)
                Call(MoveBattleCamOver, 20)
                Wait(20)
                Call(ActorSpeak, MSG_TrainHeist_BlackBattleStart, ACTOR_BLACK_BANDIT, PRT_MAIN, ANIM_KoopaGang_Black_BlackTalk, ANIM_KoopaGang_Black_BlackIdle)
                Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_Scene_RedPhase)
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

}; // namespace black_bandit_koopa

ActorBlueprint BlackBanditKoopa = {
    .flags = 0, // ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = black_bandit_koopa::hp,
    .type = black_bandit_koopa::THIS_ACTOR_TYPE,
    .level = black_bandit_koopa::THIS_LEVEL,
    .partCount = ARRAY_COUNT(black_bandit_koopa::ActorParts),
    .partsData = black_bandit_koopa::ActorParts,
    .initScript = &black_bandit_koopa::EVS_Init,
    .statusTable = black_bandit_koopa::StatusTable,
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
