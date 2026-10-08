#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "battle/stage/trn_00/stage.xml.h"
#include "sprite/npc/KoopaGang.h"
#include "sprite/npc/BuzzyBeetle.h"
#include "sprite/npc/BrigaderBones.h"
#include "boss.hpp"
#include "dx/debug_menu.h"

enum ThisBanditsParams {
    // THIS_ACTOR_ID               = ACTOR_ENEMY0,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_GREEN_BANDIT,
    THIS_LEVEL                  = ACTOR_LEVEL_GREEN_BANDIT,
    THIS_SLEEP_CHANCE           = 0,
    THIS_DIZZY_CHANCE           = 0,
    THIS_PARALYZE_CHANCE        = 0,
    THIS_ANIM_IDLE              = ANIM_KoopaGang_Green_CannonSit,
    THIS_ANIM_STILL             = ANIM_KoopaGang_Green_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang_Green_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang_Green_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang_Green_HurtStill,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang_Green_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang_Green_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang_Green_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang_Green_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang_Green_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang_Green_PointForward,
};

extern s32 DefaultAnims[];
extern s32 CannonAnims[];
extern s32 CannonSlowAnims[];
extern s32 BulletAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_ManageFirstPhase;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_SecondPhaseTransition;
// extern ActorBlueprint YellowBanditKoopa;
// extern ActorBlueprint GiantChainChomp;
// extern ActorBlueprint YellowHammerBro;
extern EvtScript EVS_Attack_BulletBiff_Fast;
extern EvtScript EVS_Attack_BulletBiff_Slow;

enum ActorPartIDs {
    PRT_MAIN            = 1,
    PRT_CANNON          = 2,
    PRT_BIFF            = 3,
};

// Actor Stats
constexpr s32 hp = 10;
constexpr s32 dmgSlowImpact = 1;
constexpr s32 dmgFastImpact = 2;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 ToppledDefense[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,                              0,
    STATUS_KEY_DEFAULT,                             0,
    STATUS_KEY_SLEEP,               THIS_SLEEP_CHANCE,
    STATUS_KEY_POISON,                              0,
    STATUS_KEY_FROZEN,                              0,
    STATUS_KEY_DIZZY,               THIS_DIZZY_CHANCE,
    STATUS_KEY_UNUSED,                              0,
    STATUS_KEY_STATIC,                              0,
    STATUS_KEY_PARALYZE,         THIS_PARALYZE_CHANCE,
    STATUS_KEY_SHRINK,                              0,
    STATUS_KEY_STOP,                                0,
    STATUS_TURN_MOD_DEFAULT,                        0,
    STATUS_TURN_MOD_SLEEP,                          0,
    STATUS_TURN_MOD_POISON,                         0,
    STATUS_TURN_MOD_FROZEN,                         0,
    STATUS_TURN_MOD_DIZZY,                          0,
    STATUS_TURN_MOD_UNUSED,                         0,
    STATUS_TURN_MOD_STATIC,                         0,
    STATUS_TURN_MOD_PARALYZE,                       0,
    STATUS_TURN_MOD_SHRINK,                         0,
    STATUS_TURN_MOD_STOP,                           0,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 40, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
    {
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_CANNON,
        .posOffset = { -5, 0, 1 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = CannonAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
    {
        .flags = ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_BIFF,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = BulletAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

extern "C" export ActorBlueprint blueprint = {
    .flags = 0, // ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = hp,
    .type = THIS_ACTOR_TYPE,
    .level = THIS_LEVEL,
    .partCount = ARRAY_COUNT(ActorParts),
    .partsData = ActorParts,
    .initScript = &EVS_Init,
    .statusTable = StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 70,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 100 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
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

s32 BulletAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Green_BulletBiff,
    STATUS_END,
};

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

API_CALLABLE(FadeScreenToBlack) {
    if (isInitialCall) {
        script->functionTemp[1] = 0;
    }

    script->functionTemp[1] += 16;

    if (script->functionTemp[1] > 255) {
        script->functionTemp[1] = 255;
    }

    set_screen_overlay_params_front(OVERLAY_SCREEN_COLOR, script->functionTemp[1]);

    if (script->functionTemp[1] == 255) {
        return ApiStatus_DONE2;
    }

    return ApiStatus_BLOCK;
}

API_CALLABLE(FadeScreenFromBlack) {
    if (isInitialCall) {
        script->functionTemp[1] = 255;
    }

    script->functionTemp[1] -= 16;
    if (script->functionTemp[1] <= 0) {
        script->functionTemp[1] = 0;
        return ApiStatus_DONE2;
    }

    set_screen_overlay_params_front(OVERLAY_SCREEN_COLOR, script->functionTemp[1]);
    return ApiStatus_BLOCK;
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
    Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_Scene_GreenPhase)
    Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_ActorsSpawned, false)
    Exec(EVS_ManageFirstPhase)
    Return
    End
};

EvtScript EVS_ManageFirstPhase = {
    Call(EnableModel, MODEL_BombBox, true)
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
            Wait(5)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Defeat)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_IDLE)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(EnableBattleStatusBar, false)
    Call(ActorExists, ACTOR_BUZZY_BEETLE, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_BUZZY_BEETLE, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_BUZZY_BEETLE)
                Call(EnableIdleScript, ACTOR_BUZZY_BEETLE, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_BUZZY_BEETLE, false)
                Call(SetAnimation, ACTOR_BUZZY_BEETLE, PRT_MAIN, ANIM_BuzzyBeetle_Hurt)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_BUZZY_BEETLE, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_BUZZY_BEETLE, 0)
                Call(GetActorPos, ACTOR_BUZZY_BEETLE, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2)
                Call(PlaySoundAtActor, ACTOR_BUZZY_BEETLE, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_BUZZY_BEETLE, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_BUZZY_BEETLE)
            EndThread
        EndIf
    EndIf
    Call(ActorExists, ACTOR_BRIGADER_BONES, LVar2)
    IfNe(LVar2, false)
        Call(GetActorHP, ACTOR_BRIGADER_BONES, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_BRIGADER_BONES)
                Call(EnableIdleScript, ACTOR_BRIGADER_BONES, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_BRIGADER_BONES, false)
                Call(SetAnimation, ACTOR_BRIGADER_BONES, PRT_MAIN, ANIM_BrigaderBones_Hurt)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_BRIGADER_BONES, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_BRIGADER_BONES, 0)
                Call(GetActorPos, ACTOR_BRIGADER_BONES, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2)
                Call(PlaySoundAtActor, ACTOR_BRIGADER_BONES, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_BRIGADER_BONES, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_BRIGADER_BONES)
            EndThread
        EndIf
    EndIf
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Green_Hurt)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_HEALTH_BAR, true)
    Wait(10)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 60)
    Add(LVar2, 2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0)
    Wait(25)
    Label(0)
        Call(ActorExists, ACTOR_BUZZY_BEETLE, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
        Call(ActorExists, ACTOR_BRIGADER_BONES, LVar0)
        IfNe(LVar0, false)
            Wait(1)
            Goto(0)
        EndIf
    ExecWait(EVS_SecondPhaseTransition)
    // Call(SetBattleFlagBits, BS_FLAGS1_DISABLE_CELEBRATION | BS_FLAGS1_BATTLE_FLED, true)
    // Call(SetBattleFlagBits2, BS_FLAGS2_DONT_STOP_MUSIC, true)
    // Call(SetEndBattleFadeOutRate, 20)
    Return
    End
};

Vec3i YellowBanditSpawnPos = { 85, 0, 15 };

Formation SpawnYellowBandit = {
    OVL_ACTOR_BY_POS("yellow_bandit_koopa", YellowBanditSpawnPos, 50),
};

Vec3i GiantChainChompSpawnPos = { 25, 0, 20 };

Formation SpawnGiantChainChomp = {
    OVL_ACTOR_BY_POS("giant_chain_chomp", GiantChainChompSpawnPos, 100),
};

Vec3i HammerBroAltSpawnPos = { 125, 0, 10 };

Formation SpawnYellowHammerBro = {
    OVL_ACTOR_BY_POS("yellow_hammer_bro", HammerBroAltSpawnPos, 75),
};

// Vec3i PositionSecondGroup[] = {
//     { 105, 0, 10 },
//     { 25, 0, 15 },
//     { 145, 0, 10 },
// };

// Formation FormationSecondGroup = {
//     ACTOR_BY_POS(YellowBanditKoopa, PositionSecondGroup[0], 9),
//     ACTOR_BY_POS(GiantChainChomp, PositionSecondGroup[1], 10),
//     ACTOR_BY_POS(YellowHammerBro, PositionSecondGroup[2], 8),
// };

EvtScript EVS_SecondPhaseTransition = {
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
                    Call(GetActorVar, ACTOR_SELF, AVAR_GreenPhase_ActorsSpawned, LVarB)
                    IfEq(LVarB, false)
                        Call(EnableModel, MODEL_BombBox, false)
                        Call(SummonEnemy, Ref(SpawnYellowBandit), false)
                        Call(SummonEnemy, Ref(SpawnGiantChainChomp), false)
                        Call(SummonEnemy, Ref(SpawnYellowHammerBro), false)
                        // Call(SummonEnemy, Ref(FormationSecondGroup), false)
                        Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_INVISIBLE, true)
                        Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_ActorsSpawned, true)
                        Call(SetActorVar, ACTOR_SELF, AVAR_Phase, AVAL_YellowPhase)
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
    Call(GetActorVar, ACTOR_SELF, AVAR_GreenPhase_CannonAttacks, LVar0)
    Switch(LVar0)
        CaseEq(AVAL_GreenPhase_SlowCannonAttack)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_CANNON, Ref(CannonSlowAnims))
            ExecWait(EVS_Attack_BulletBiff_Slow)
        CaseEq(AVAL_GreenPhase_FastCannonAttack)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_CANNON, Ref(CannonAnims))
            ExecWait(EVS_Attack_BulletBiff_Fast)
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Attack_BulletBiff_Slow = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Green_CannonSitFire)
    Wait(5)
    Call(SetAnimation, ACTOR_SELF, PRT_CANNON, ANIM_KoopaGang_Green_ShotSlow)
    Wait(13)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 10, Float(1.0))
    EndThread
    Call(StartRumble, BTL_RUMBLE_PLAYER_HEAVY)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_CANNON, SOUND_BULLET_BILL_FIRE)
    Call(GetPartPos, ACTOR_SELF, PRT_CANNON, LVar0, LVar1, LVar2)
    Sub(LVar0, 33)
    Add(LVar1, 35)
    Add(LVar2, 3)
    PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2)
    PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_BIFF, ANIM_KoopaGang_Green_BulletBiff)
    Call(GetPartPos, ACTOR_SELF, PRT_CANNON, LVar0, LVar1, LVar2)
    Add(LVar0, -55)
    Add(LVar1, 35)
    Add(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2)
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
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, false)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(8.0))
            Call(SetGoalToTarget, ACTOR_SELF)
            // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
            // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN)
            // Wait(5)
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BIFF, Float(0.01))
            Call(JumpPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, true)
            Wait(2)
            IfEq(LVar0, HIT_RESULT_LUCKY)
                Call(SetGoalToTarget, ACTOR_SELF)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Call(YieldTurn)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, true)
            Return
        EndCaseGroup
    EndSwitch
    // Wait(5)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 15)
    // Add(LVar1, 48)
    // Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, false)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Add(LVar1, 25)
    Add(LVar2, 2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(8.0))
    Call(SetGoalToTarget, ACTOR_SELF)
    // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
    // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN)
    // Wait(5)
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_BIFF, Float(0.01))
    Call(JumpPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, true)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgSlowImpact, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, true)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartPos, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Return
    End
};

EvtScript EVS_Attack_BulletBiff_Fast = {
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Green_CannonSitFire)
    Wait(5)
    Call(SetAnimation, ACTOR_SELF, PRT_CANNON, ANIM_KoopaGang_Green_Shot)
    Wait(13)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 10, Float(1.0))
    EndThread
    Call(StartRumble, BTL_RUMBLE_PLAYER_HEAVY)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_CANNON, SOUND_BULLET_BILL_FIRE)
    Call(GetPartPos, ACTOR_SELF, PRT_CANNON, LVar0, LVar1, LVar2)
    Sub(LVar0, 33)
    Add(LVar1, 35)
    Add(LVar2, 3)
    PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2)
    PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_BIFF, ANIM_KoopaGang_Green_BulletBiff)
    Call(GetPartPos, ACTOR_SELF, PRT_CANNON, LVar0, LVar1, LVar2)
    Add(LVar0, -55)
    Add(LVar1, 35)
    Add(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2)
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
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, false)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(12.0))
            Call(SetGoalToTarget, ACTOR_SELF)
            // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
            // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
            // Wait(5)
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BIFF, Float(0.01))
            Call(JumpPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, true)
            Wait(2)
            IfEq(LVar0, HIT_RESULT_LUCKY)
                Call(SetGoalToTarget, ACTOR_SELF)
                Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            EndIf
            Call(YieldTurn)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, true)
            Return
        EndCaseGroup
    EndSwitch
    // Wait(5)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 15)
    // Add(LVar1, 48)
    // Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, false)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Add(LVar1, 25)
    Add(LVar2, 2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(12.0))
    Call(SetGoalToTarget, ACTOR_SELF)
    // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
    // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
    // Wait(5)
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_BIFF, Float(0.01))
    Call(JumpPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, true)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgFastImpact, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, true)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartPos, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
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
            IfEq(LVar0, AVAL_Scene_GreenPhase)
                Call(EnableBattleStatusBar, false)
                Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
                Call(SetBattleCamTarget, 70, 75, 20)
                Call(SetBattleCamDist, 250)
                Call(SetBattleCamYaw, 0)
                Call(SetBattleCamOffsetY, 15)
                Call(MoveBattleCamOver, 20)
                Wait(20)
                Call(ActorSpeak, MSG_TrainHeist_GreenBattleStart, ACTOR_GREEN_BANDIT, PRT_MAIN, ANIM_KoopaGang_Green_CannonTalk, ANIM_KoopaGang_Green_CannonSit)
                Call(SetActorVar, ACTOR_SELF, AVAR_Scene_BeginBattle, AVAL_Scene_YellowPhase)
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
