#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "mapfs/trn_bt00_shape.h"
#include "sprite/npc/KoopaGang.h"
#include "sprite/npc/PyroGuy.h"
#include "sprite/npc/Bobomb.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace red_bandit_koopa {

// Red Bandit Params
enum ThisBanditsParams {
    // THIS_ACTOR_ID               = ACTOR_ENEMY3,
    THIS_ACTOR_TYPE             = ACTOR_TYPE_RED_BANDIT,
    THIS_LEVEL                  = ACTOR_LEVEL_RED_BANDIT,
    THIS_SLEEP_CHANCE           = 0,
    THIS_DIZZY_CHANCE           = 0,
    THIS_PARALYZE_CHANCE        = 0,
    THIS_ANIM_IDLE              = ANIM_KoopaGang_Red_Digging,
    THIS_ANIM_STILL             = ANIM_KoopaGang_Red_Still,
    THIS_ANIM_RUN               = ANIM_KoopaGang_Red_Run,
    THIS_ANIM_HURT              = ANIM_KoopaGang_Red_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaGang_Red_HurtStill,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaGang_Red_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaGang_Red_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaGang_Red_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaGang_Red_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaGang_Red_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaGang_Red_PointForward,
};

extern s32 DefaultAnims[];
extern s32 BombAnims[];
extern s32 PokeyAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_ManageFourthPhase;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_FifthPhaseTransition;
extern EvtScript EVS_Attack_LitBomb;
extern EvtScript EVS_Attack_UnlitBomb;
extern EvtScript EVS_Summon_LitBobomb;
extern EvtScript EVS_Summon_UnlitBobomb;
extern EvtScript EVS_Attack_Pokey;

enum ActorPartIDs {
    PRT_MAIN            = 1,
    PRT_BOMB            = 2,
    PRT_POKEY           = 3,
};

// Actor Stats
constexpr s32 hp = 1;
constexpr s32 dmgLitBomb = 5;
constexpr s32 dmgUnlitBomb = 3;
constexpr s32 dmgPokey = 2;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   1,
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
        .flags = ACTOR_PART_FLAG_IGNORE_BELOW_CHECK | ACTOR_PART_FLAG_PRIMARY_TARGET,
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
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION | ACTOR_PART_FLAG_NO_STATUS_ANIMS,
        .index = PRT_BOMB,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = BombAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION | ACTOR_PART_FLAG_NO_STATUS_ANIMS,
        .index = PRT_POKEY,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = PokeyAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
    STATUS_END,
};

s32 BombAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Red_BombLit,
    STATUS_END,
};

s32 PokeyAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Red_Pokey,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

BSS PlayerStatus DummyPlayerStatus;

API_CALLABLE((SpawnSpinEffect)) {
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

API_CALLABLE((FadeScreenToBlack)) {
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

API_CALLABLE((FadeScreenFromBlack)) {
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
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_PyroDefeated, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_SummonedBobomb, FALSE)
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(HPBarToHome, ACTOR_SELF)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_ActorsSpawned, FALSE)
    Exec(EVS_ManageFourthPhase)
    Return
    End
};

EvtScript EVS_ManageFourthPhase = {
    // Call(EnableModel, MODEL_BombBox, TRUE)
    // Call(EnableModel, MODEL_BombPile, TRUE)
    // Call(EnableModel, MODEL_BarrelRed, TRUE)
    // Call((FadeScreenFromBlack))
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Return
    End
};

EvtScript EVS_Idle = {
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
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(EnableBattleStatusBar, FALSE)
    Call(ActorExists, ACTOR_PYRO_GUY, LVar2)
    IfNe(LVar2, FALSE)
        Call(GetActorHP, ACTOR_PYRO_GUY, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_PYRO_GUY)
                Call(EnableIdleScript, ACTOR_PYRO_GUY, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_PYRO_GUY, FALSE)
                Call(SetAnimation, ACTOR_PYRO_GUY, PRT_MAIN, ANIM_PyroGuy_Anim06)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_PYRO_GUY, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_PYRO_GUY, 0)
                Call(GetActorPos, ACTOR_PYRO_GUY, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                Call(PlaySoundAtActor, ACTOR_PYRO_GUY, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_PYRO_GUY, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_PYRO_GUY)
            EndThread
        EndIf
    EndIf
    Call(ActorExists, ACTOR_BOB_OMB, LVar2)
    IfNe(LVar2, FALSE)
        Call(GetActorHP, ACTOR_BOB_OMB, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_BOB_OMB)
                Call(EnableIdleScript, ACTOR_BOB_OMB, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_BOB_OMB, FALSE)
                Call(StopLoopingSoundAtActor, ACTOR_BOB_OMB, 0)
                Call(EnableActorPaletteEffects, ACTOR_BOB_OMB, PRT_MAIN, FALSE)
                Call(StartRumble, BTL_RUMBLE_PLAYER_MAX)
                Thread
                    Call(ShakeCam, CAM_BATTLE, 0, 2, Float(0.75))
                    Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
                    Call(ShakeCam, CAM_BATTLE, 0, 10, Float(4.5))
                    Call(ShakeCam, CAM_BATTLE, 0, 5, Float(3.0))
                EndThread
                Call(GetActorPos, ACTOR_BOB_OMB, LVar0, LVar1, LVar2)
                Add(LVar2, 2)
                PlayEffect(EFFECT_SMOKE_RING, 0, LVar0, LVar1, LVar2, 0)
                Add(LVar1, 20)
                Add(LVar2, 2)
                PlayEffect(EFFECT_EXPLOSION, 0, LVar0, LVar1, LVar2, 0)
                Call(PlaySoundAtActor, ACTOR_BOB_OMB, SOUND_BOMB_BLAST)
                Call(SetAnimation, ACTOR_BOB_OMB, PRT_MAIN, ANIM_Bobomb_BurnStill)
                Wait(10)
                Set(LVar2, 0)
                Loop(24)
                    Call(SetActorYaw, ACTOR_BOB_OMB, LVar2)
                    Add(LVar2, 30)
                    Wait(1)
                EndLoop
                Call(SetActorYaw, ACTOR_BOB_OMB, 0)
                Call(GetActorPos, ACTOR_BOB_OMB, LVar0, LVar1, LVar2)
                Add(LVar1, 10)
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
                Call(PlaySoundAtActor, ACTOR_BOB_OMB, SOUND_ACTOR_DEATH)
                Set(LVar3, 0)
                Loop(12)
                    Call(SetActorRotation, ACTOR_BOB_OMB, LVar3, 0, 0)
                    Add(LVar3, 8)
                    Wait(1)
                EndLoop
                Call(RemoveActor, ACTOR_BOB_OMB)
            EndThread
        EndIf
    EndIf
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(SetActorFlagBits, ACTOR_RED_BANDIT, ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
    Wait(10)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 50)
    Add(LVar2, 2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(25)
    Label(0)
        Call(ActorExists, ACTOR_PYRO_GUY, LVar0)
        IfNe(LVar0, FALSE)
            Wait(1)
            Goto(0)
        EndIf
        Call(ActorExists, ACTOR_BOB_OMB, LVar0)
        IfNe(LVar0, FALSE)
            Wait(1)
            Goto(0)
        EndIf
    ExecWait(EVS_FifthPhaseTransition)
    Return
    End
};

Vec3i BowsertheKidSpawnPos = { 80, 45, 20 };

Formation SpawnKoopatheKid = {
    ACTOR_BY_POS(KoopaTheKid, BowsertheKidSpawnPos, 75),
};

Vec3i KoopaGangSpawnPos = { 20, 0, 20 };

Formation SpawnKoopaGang = {
    ACTOR_BY_POS(KoopaGang, KoopaGangSpawnPos, 100),
};

Vec3i GreenHammerBroSpawnPos = { 130, 0, 20 };

Formation SpawnGreenHammerBro = {
    ACTOR_BY_POS(GreenHammerBro, GreenHammerBroSpawnPos, 50),
};

EvtScript EVS_FifthPhaseTransition = {
    Call(CancelEnemyTurn, 1)
    Call(EnableModel, MODEL_Tunnel, TRUE)
    Set(LVarA, 0)
    Loop(0)
        Add(LVarA, 10) // Increment LVarA by 10
        IfGt(LVarA, 2250)
            Set(LVarA, 0) // Reset LVarA back to 0 when it exceeds 2250
            BreakLoop
        EndIf
        IfGt(LVarA, 1000)
            Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_ActorsSpawned, LVarB)
            IfEq(LVarB, FALSE)
                Call(EnableModel, MODEL_BombBox, FALSE)
                Call(EnableModel, MODEL_BombPile, FALSE)
                Call(EnableModel, MODEL_BarrelRed, FALSE)
                Call(SummonEnemy, Ref(SpawnKoopatheKid), FALSE)
                Call(SummonEnemy, Ref(SpawnKoopaGang), FALSE)
                Call(SummonEnemy, Ref(SpawnGreenHammerBro), FALSE)
                Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_INVISIBLE, TRUE)
                Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_ActorsSpawned, TRUE)
            EndIf
        EndIf
        Call(TranslateModel, MODEL_Tunnel, LVarA, 0, 0)
        Wait(1)
    EndLoop
    Call(TranslateModel, MODEL_Tunnel, LVarA, 0, 0)
    Call(EnableModel, MODEL_Tunnel, FALSE)
    Call(EnableBattleStatusBar, TRUE)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

// EvtScript EVS_TakeTurn = {
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, FALSE)
//     Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_PyroDefeated, LVar3)
//     // Set(LVar3, FALSE) // Set Bomb Attack/Move to be Lit or Unlit
//     Switch(LVar3)
//         CaseEq(FALSE)
//             Call(ActorExists, ACTOR_BOB_OMB, LVar4)
//             IfEq(LVar4, FALSE)
//                 Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_SummonedBobomb, LVar5)
//                 IfEq(LVar5, FALSE) // If Bobomb is not summoned
//                     // DebugPrintf("Bobomb is not summoned")
//                     Call(RandInt, 100, LVar6)
//                     Switch(LVar6)
//                         CaseLt(10)
//                             Set(LVar7, AVAL_RedPhase_LitBombAttack)
//                         CaseLt(30)
//                             Set(LVar7, AVAL_RedPhase_LitBobombSummon)
//                         CaseDefault
//                             Set(LVar7, AVAL_RedPhase_PokeyAttack)
//                     EndSwitch
//                     Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_RandomAttack, LVar7)
//                     // Set(LVar0, AVAL_RedPhase_LitBobombSummon) // Set Attack/Move
//                     Switch(LVar7)
//                         CaseEq(AVAL_RedPhase_LitBombAttack)
//                             ExecWait(EVS_Attack_LitBomb)
//                             Return
//                         CaseEq(AVAL_RedPhase_LitBobombSummon)
//                             ExecWait(EVS_Summon_LitBobomb)
//                             Return
//                         CaseEq(AVAL_RedPhase_PokeyAttack)
//                             ExecWait(EVS_Attack_Pokey)
//                             Return
//                     EndSwitch
//                 Else
//                     // DebugPrintf("Bobomb is summoned")
//                     Call(RandInt, 100, LVar6)
//                     Switch(LVar6)
//                         CaseLt(60)
//                             Set(LVar7, AVAL_RedPhase_LitBombAttack)
//                         CaseDefault
//                             Set(LVar7, AVAL_RedPhase_PokeyAttack)
//                     EndSwitch
//                     Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_RandomAttack, LVar7)
//                     // Set(LVar0, AVAL_RedPhase_PokeyAttack) // Set Attack/Move
//                     Switch(LVar7)
//                         CaseEq(AVAL_RedPhase_LitBombAttack)
//                             ExecWait(EVS_Attack_LitBomb)
//                             Return
//                         CaseEq(AVAL_RedPhase_PokeyAttack)
//                             ExecWait(EVS_Attack_Pokey)
//                             Return
//                     EndSwitch
//                 EndIf
//             Else
//                 // DebugPrintf("Bobomb is summoned")
//                 Call(RandInt, 100, LVar6)
//                 Switch(LVar6)
//                     CaseLt(60)
//                         Set(LVar7, AVAL_RedPhase_LitBombAttack)
//                     CaseDefault
//                         Set(LVar7, AVAL_RedPhase_PokeyAttack)
//                 EndSwitch
//                 Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_RandomAttack, LVar7)
//                 // Set(LVar0, AVAL_RedPhase_PokeyAttack) // Set Attack/Move
//                 Switch(LVar7)
//                     CaseEq(AVAL_RedPhase_LitBombAttack)
//                         ExecWait(EVS_Attack_LitBomb)
//                         Return
//                     CaseEq(AVAL_RedPhase_PokeyAttack)
//                         ExecWait(EVS_Attack_Pokey)
//                         Return
//                 EndSwitch
//             EndIf
//         CaseEq(TRUE)
//             Call(ActorExists, ACTOR_BOB_OMB, LVar4)
//             IfEq(LVar4, FALSE)
//                 Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_SummonedBobomb, LVar5)
//                 IfEq(LVar5, FALSE)
//                     // DebugPrintf("Bobomb is not summoned")
//                     Call(RandInt, 100, LVar6)
//                     Switch(LVar6)
//                         CaseLt(40)
//                             Set(LVar7, AVAL_RedPhase_UnlitBombAttack)
//                         CaseLt(40)
//                             Set(LVar7, AVAL_RedPhase_UnlitBobombSummon)
//                         CaseDefault
//                             Set(LVar7, AVAL_RedPhase_PokeyAttack)
//                     EndSwitch
//                     Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_RandomAttack, LVar7)
//                     // Set(LVar0, AVAL_RedPhase_UnlitBobombSummon) // Set Attack/Move
//                     Switch(LVar7)
//                         CaseEq(AVAL_RedPhase_UnlitBombAttack)
//                             ExecWait(EVS_Attack_UnlitBomb)
//                             Return
//                         CaseEq(AVAL_RedPhase_UnlitBobombSummon)
//                             ExecWait(EVS_Summon_UnlitBobomb)
//                             Return
//                         CaseEq(AVAL_RedPhase_PokeyAttack)
//                             ExecWait(EVS_Attack_Pokey)
//                             Return
//                     EndSwitch
//                 Else
//                     // DebugPrintf("Bobomb is not summoned")
//                     Call(RandInt, 100, LVar6)
//                     Switch(LVar6)
//                         CaseLt(60)
//                             Set(LVar7, AVAL_RedPhase_UnlitBombAttack)
//                         CaseDefault
//                             Set(LVar7, AVAL_RedPhase_PokeyAttack)
//                     EndSwitch
//                     Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_RandomAttack, LVar7)
//                     // Set(LVar0, AVAL_RedPhase_PokeyAttack) // Set Attack/Move
//                     Switch(LVar7)
//                         CaseEq(AVAL_RedPhase_UnlitBombAttack)
//                             ExecWait(EVS_Attack_UnlitBomb)
//                             Return
//                         CaseEq(AVAL_RedPhase_PokeyAttack)
//                             ExecWait(EVS_Attack_Pokey)
//                             Return
//                     EndSwitch
//                 EndIf
//             Else
//                 // DebugPrintf("Bobomb is summoned")
//                 Call(RandInt, 100, LVar6)
//                 Switch(LVar6)
//                     CaseLt(60)
//                         Set(LVar7, AVAL_RedPhase_UnlitBombAttack)
//                     CaseDefault
//                         Set(LVar7, AVAL_RedPhase_PokeyAttack)
//                 EndSwitch
//                 Call(GetActorVar, ACTOR_SELF, AVAR_RedPhase_RandomAttack, LVar7)
//                 // Set(LVar0, AVAL_RedPhase_PokeyAttack) // Set Attack/Move
//                 Switch(LVar7)
//                     CaseEq(AVAL_RedPhase_UnlitBombAttack)
//                         ExecWait(EVS_Attack_UnlitBomb)
//                         Return
//                     CaseEq(AVAL_RedPhase_PokeyAttack)
//                         ExecWait(EVS_Attack_Pokey)
//                         Return
//                 EndSwitch
//             EndIf
//     EndSwitch
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, TRUE)
//     Return
//     End
// };


// EvtScript EVS_TakeTurn = {
//     //LVar0 = Reusable Storage
//     //LVar1 = Bomb Attack Script
//     //LVar2 = Bomb Summon Script
//     //LVar3 = Bomb Priority
//     //LVar4 = Pokey Priority
//     //LVar5 = Chosen Event Script
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, FALSE)
//     Call(ActorExists, ACTOR_BOB_OMB, LVar0)         //Check if Bomb exists
//     Switch(LVar0)   //Increase Pokey priority if Bomb Exists
//         CaseEq(FALSE)
//             Set(LVar4, 80)
//         CaseEq(TRUE)
//             Set(LVar4, 60)
//     EndSwitch
//     Call(RandInt, 100, LVar0)
//     IfLe(LVar0, LVar4)  //Attack Bomb
//         Call(ActorExists, ACTOR_PYRO_GUY, LVar0)    //Check if Bomb lit
//         Switch(LVar0)
//             CaseEq(FALSE)   //Use unlit Bomb
//                 Set(LVar1, EVS_Attack_UnlitBomb)
//                 Set(LVar2, EVS_Summon_UnlitBobomb)
//                 Set(LVar3, 50)  //Bomb Priority
//             CaseEq(TRUE)    //Use lit Bomb
//                 Set(LVar1, EVS_Attack_LitBomb)
//                 Set(LVar2, EVS_Summon_LitBobomb)
//                 Set(LVar3, 50)  //Bomb Priority
//         EndSwitch
//         Call(ActorExists, ACTOR_BOB_OMB, LVar0)     //Check if Bomb exists
//         Switch(LVar0)
//             CaseEq(FALSE)   //No Bomb (Allow Summon)
//                 Call(RandInt, 100, LVar0)
//                 IfLe(LVar0, LVar3)
//                     Set(LVar5, LVar1)
//                 Else
//                     Set(LVar5, LVar2)
//                 EndIf
//             CaseEq(TRUE)    //Bomb (No Summon)
//                 Set(LVar5, LVar1)
//         EndSwitch
//     Else     //Attack Pokey
//         Set(LVar5, EVS_Attack_Pokey)
//     EndIf
//     ExecWait(LVar5)
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, TRUE)
//     Return
//     End
// };

// Define macros for descriptive variable names
#define ReusableStorage  LVar0
#define BombAttackScript LVar1
#define BombSummonScript LVar2
#define BombPriority     LVar3
#define PokeyPriority    LVar4
#define ChosenEventScript LVar5

EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(ActorExists, ACTOR_BOB_OMB, ReusableStorage)
    Switch(ReusableStorage)
        CaseEq(FALSE)
            Set(PokeyPriority, 80)
        CaseEq(TRUE)
            Set(PokeyPriority, 60)
    EndSwitch
    Call(RandInt, 100, ReusableStorage)
    IfLe(ReusableStorage, PokeyPriority)
        Call(ActorExists, ACTOR_PYRO_GUY, ReusableStorage)
        Switch(ReusableStorage)
            CaseEq(FALSE)
                Set(BombAttackScript, EVS_Attack_UnlitBomb)
                Set(BombSummonScript, EVS_Summon_UnlitBobomb)
                Set(BombPriority, 50)
            CaseEq(TRUE)
                Set(BombAttackScript, EVS_Attack_LitBomb)
                Set(BombSummonScript, EVS_Summon_LitBobomb)
                Set(BombPriority, 50)
        EndSwitch
        Call(ActorExists, ACTOR_BOB_OMB, ReusableStorage)
        Switch(ReusableStorage)
            CaseEq(FALSE)
                Call(RandInt, 100, ReusableStorage)
                IfLe(ReusableStorage, BombPriority)
                    Set(ChosenEventScript, BombAttackScript)
                Else
                    Set(ChosenEventScript, BombSummonScript)
                EndIf
            CaseEq(TRUE)
                Set(ChosenEventScript, BombAttackScript)
        EndSwitch
    Else
        Set(ChosenEventScript, EVS_Attack_Pokey)
    EndIf
    ExecWait(ChosenEventScript)
    // BufRead3(LVar0, LVar1, LVar2)
    // SetF(*tmp_0, *arg_0)
    // SetF(*tmp_1, *arg_0)
    // MulF(*tmp_0, *arg_1)
    // MulF(*tmp_1, *arg_2)
    // AddF(LVar0, *tmp_0)
    // AddF(LVar1, *tmp_1)
    Call(SetPartScale, ACTOR_SELF, 1, LVar0, LVar1, LVar2 )
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

// Undefine macros
#undef ReusableStorage
#undef BombAttackScript
#undef BombSummonScript
#undef BombPriority
#undef PokeyPriority
#undef ChosenEventScript

// EvtScript EVS_LitBomb_Hit = {
//     Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, FALSE)
//     Call(SetPartSounds, ACTOR_SELF, LVar0, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
//     Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_TOSS)
//     Call(SetGoalToTarget, ACTOR_SELF)
//     Call(GetGoalPos, ACTOR_SELF, LVar1, LVar2, LVar3)
//     Call(SetPartJumpGravity, ACTOR_SELF, LVar0, Float(1.3))
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 15, TRUE)
//     Sub(LVar1, 50)
//     Set(LVar2, 10)
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 20, TRUE)
//     Sub(LVar1, 30)
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 10, TRUE)
//     Sub(LVar1, 20)
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 5, TRUE)
//     Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, TRUE)
//     Return
//     End
// };

// EvtScript EVS_UnlitBomb_Hit = {
//     Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, FALSE)
//     Call(SetPartSounds, ACTOR_SELF, PRT_BOMB, ACTOR_SOUND_JUMP, SOUND_NONE, SOUND_NONE)
//     Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_TOSS)
//     Call(SetGoalToTarget, ACTOR_SELF)
//     Call(GetGoalPos, ACTOR_SELF, LVar1, LVar2, LVar3)
//     Call(SetPartJumpGravity, ACTOR_SELF, LVar0, Float(1.3))
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 15, TRUE)
//     Sub(LVar1, 50)
//     Set(LVar2, 10)
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 20, TRUE)
//     Sub(LVar1, 30)
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 10, TRUE)
//     Sub(LVar1, 20)
//     Call(JumpPartTo, ACTOR_SELF, LVar0, LVar1, LVar2, LVar3, 5, TRUE)
//     Call(SetPartFlagBits, ACTOR_SELF, LVar0, ACTOR_PART_FLAG_INVISIBLE, TRUE)
//     Return
//     End
// };

// Define macros for readability
#define GetPosX() LVar0
#define SetPosX(value) Set(LVar0, value)
#define AddPosX(value) Add(LVar0, value)

EvtScript EVS_Attack_LitBomb = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR_FAR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 30)
    Call(SetBattleCamDist, 250)
    Thread
        Wait(45)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 30)
    EndThread
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedPickup)
    Wait(15)
    Call(GetActorPos, ACTOR_SELF, GetPosX(), LVar1, LVar2)
    AddPosX(5)
    Add(LVar1, 35)
    Add(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_BOMB, ANIM_KoopaGang_Red_Bomb)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Wait(15)
    Thread
        Loop(12)
            Call(GetActorYaw, ACTOR_SELF, LVar3)
            Add(LVar3, 15)
            Call(SetActorYaw, ACTOR_SELF, LVar3)
            Call(GetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
            Add(LVar3, 15)
            Call(SetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
            Wait(1)
        EndLoop
        Call(GetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
        Add(LVar0, 17)
        Sub(LVar1, 10)
        Call(SetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedHold)
    Wait(10)
    Call(SetAnimation, ACTOR_SELF, PRT_BOMB, ANIM_KoopaGang_Red_BombLit)
    Wait(15)
    Thread
        Loop(6)
            Call(GetActorYaw, ACTOR_SELF, LVar3)
            Sub(LVar3, 30)
            Call(SetActorYaw, ACTOR_SELF, LVar3)
            Call(GetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
            Add(LVar3, 30)
            Call(SetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
            Wait(1)
        EndLoop
        Call(GetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
        Sub(LVar0, 17)
        Add(LVar1, 10)
        Call(SetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedThrow)
    // Wait(5)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 4, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseEq(HIT_RESULT_MISS)
        CaseEq(HIT_RESULT_LUCKY)
            Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_TOSS)
            Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, -5)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(18.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 0, 30, EASING_LINEAR)
            Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Sub(LVar0, 65)
            Set(LVar1, -5)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(6.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 0, 15, EASING_LINEAR)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_DigIdle)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            Return
    EndSwitch
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_TOSS)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(18.0))
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(0.1))
    Call(FlyPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 0, 15, EASING_LINEAR)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_BOMB_BLAST)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, TRUE)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, DAMAGE_TYPE_BLAST | DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgLitBomb, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_DigIdle)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

// Undefine macros
#undef GetPosX
#undef SetPosX
#undef AddPosX

EvtScript EVS_Attack_UnlitBomb = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR_FAR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 30)
    Call(SetBattleCamDist, 250)
    Thread
        Wait(45)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 30)
    EndThread
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedPickup)
    Wait(15)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 5)
    Add(LVar1, 35)
    Add(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_BOMB, ANIM_KoopaGang_Red_Bomb)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Wait(15)
    // Thread
    //     Loop(12)
    //         Call(GetActorYaw, ACTOR_SELF, LVar3)
    //         Add(LVar3, 15)
    //         Call(SetActorYaw, ACTOR_SELF, LVar3)
    //         Call(GetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
    //         Add(LVar3, 15)
    //         Call(SetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
    //     EndLoop
    //     Call(GetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    //     Sub(LVar0, 10)
    //     Call(SetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    // EndThread
    // Call(SetAnimation, ACTOR_SELF, PRT_BOMB, ANIM_KoopaGang_Red_BombLit)
    // Wait(15)
    // Thread
    //     Loop(12)
    //         Call(GetActorYaw, ACTOR_SELF, LVar3)
    //         Sub(LVar3, 15)
    //         Call(SetActorYaw, ACTOR_SELF, LVar3)
    //         Call(GetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
    //         Add(LVar3, 15)
    //         Call(SetPartYaw, ACTOR_SELF, PRT_BOMB, LVar3)
    //     EndLoop
    //     Call(GetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    //     Add(LVar0, 10)
    //     Call(SetPartPos, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2)
    // EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedThrow)
    // Wait(5)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 4, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseEq(HIT_RESULT_MISS)
        CaseEq(HIT_RESULT_LUCKY)
            Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_TOSS)
            Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, -5)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(18.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 0, 30, EASING_LINEAR)
            Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Sub(LVar0, 65)
            Set(LVar1, -5)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(6.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 0, 15, EASING_LINEAR)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_DigIdle)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            Return
    EndSwitch
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_BOMB, SOUND_TOSS)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(18.0))
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(0.1))
    Call(FlyPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 0, 15, EASING_LINEAR)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgUnlitBomb, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, 0)
            Add(LVar2, 2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BOMB, Float(18.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_BOMB, Float(1.5))
            Call(JumpPartTo, ACTOR_SELF, PRT_BOMB, LVar0, LVar1, LVar2, 15, TRUE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Wait(10)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_DigIdle)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

Vec3i SummonedBobOmbPos = { 0.0f, -1000.0f, 0.0f };

Formation SummonedBobOmb = {
    ACTOR_BY_POS(BobOmb, SummonedBobOmbPos, 100),
};

EvtScript EVS_Summon_LitBobomb = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR_FAR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 30)
    Call(SetBattleCamDist, 250)
    Thread
        Wait(45)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 30)
    EndThread
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedPickup)
    Call(SummonEnemy, Ref(SummonedBobOmb), FALSE)
    Wait(15)
    Set(LVarB, LVar0)
    Call(UseIdleAnimation, LVarB, FALSE)
    Call(EnableIdleScript, LVarB, IDLE_SCRIPT_DISABLE)

    Set(LVar0, 0)
    // flip over
    Call(GetActorRotation, LVarB, LVar0, LVar1, LVar2)
    Set(LVar0, 180)
    Call(SetActorRotation, LVarB, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 5)
    Add(LVar1, 55)
    Add(LVar2, 2)
    Call(SetActorPos, LVarB, LVar0, LVar1, LVar2)
    Call(SetAnimation, LVarB, PRT_MAIN, ANIM_Bobomb_Hurt)
    Wait(15)
    Thread
        Loop(12)
            Call(GetActorYaw, ACTOR_SELF, LVar3)
            Add(LVar3, 15)
            Call(SetActorYaw, ACTOR_SELF, LVar3)
            Set(LVar3, 0)
            Call(GetActorYaw, LVarB, LVar3)
            Add(LVar3, 15)
            Call(SetActorYaw, LVarB, LVar3)
            Wait(1)
        EndLoop
        Call(GetActorPos, LVarB, LVar0, LVar1, LVar2)
        Add(LVar0, 15)
        Sub(LVar1, 25)
        Call(SetActorPos, LVarB, LVar0, LVar1, LVar2)
        PlayEffect(EFFECT_RING_BLAST, 0, LVar0, LVar1, LVar2, Float(0.5), 15)
    EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedHold)
    Call(SetAnimation, LVarB, PRT_MAIN, ANIM_Bobomb_Cower)
    Wait(15)
    Thread
        Loop(6)
            Call(GetActorYaw, ACTOR_SELF, LVar3)
            Sub(LVar3, 30)
            Call(SetActorYaw, ACTOR_SELF, LVar3)
            Set(LVar3, 0)
            Wait(1)
        EndLoop
    EndThread
    Call(SetActorSpeed, LVarB, Float(18.0))
    Call(SetActorJumpGravity, LVarB, Float(0.01))
    Call(GetActorPos, LVarB, LVar0, LVar1, LVar2)
    Set(LVar0, 0)
    Add(LVar1, 250)
    Set(LVar2, 15)
    Call(SetGoalPos, LVarB, LVar0, LVar1, LVar2)
    Call(FlyToGoal, LVarB, 0, 0, EASING_LINEAR)
    Wait(15)
    Call(GetActorYaw, LVarB, LVar3)
    Set(LVar3, 0)
    Call(SetActorYaw, LVarB, LVar3)
    Set(LVar0, 0)
    Call(GetActorRotation, LVarB, LVar0, LVar1, LVar2)
    Set(LVar0, 0)
    Call(SetActorRotation, LVarB, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Digging)
    Wait(15)
    Call(SetActorSpeed, LVarB, Float(15.0))
    Call(SetActorJumpGravity, LVarB, Float(1.5))
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, -120)
    Set(LVar1, 0)
    Set(LVar2, 15)
    Call(SetGoalPos, LVarB, LVar0, LVar1, LVar2)
    Call(JumpToGoal, LVarB, 15, FALSE, TRUE, FALSE)
    Call(GetActorPos, LVarB, LVar0, LVar1, LVar2)
    Call(SetActorPos, LVarB, LVar0, LVar1, LVar2)
    Call(ForceHomePos, LVarB, LVar0, LVar1, LVar2)
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_SummonedBobomb, TRUE)
    Call(SetActorVar, LVarB, AVAR_RedPhase_BobOmbIgnited, TRUE)
    Call(EnableIdleScript, LVarB, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, LVarB, TRUE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Summon_UnlitBobomb = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    // Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    // Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR_FAR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 30)
    Call(SetBattleCamDist, 250)
    Thread
        Wait(45)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 30)
    EndThread
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedPickup)
    Call(SummonEnemy, Ref(SummonedBobOmb), FALSE)
    Wait(15)
    Set(LVarB, LVar0)
    Call(UseIdleAnimation, LVarB, FALSE)
    Call(EnableIdleScript, LVarB, IDLE_SCRIPT_DISABLE)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 5)
    Add(LVar1, 35)
    Add(LVar2, 2)
    Call(SetActorPos, LVarB, LVar0, LVar1, LVar2)
    Call(SetAnimation, LVarB, PRT_MAIN, ANIM_Bobomb_Hurt)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_BOMB, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Wait(15)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedThrow)
    Call(SetActorSpeed, LVarB, Float(18.0))
    Call(SetActorJumpGravity, LVarB, Float(1.5))
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, -120)
    Set(LVar1, 0)
    Set(LVar2, 15)
    Call(SetGoalPos, LVarB, LVar0, LVar1, LVar2)
    Call(JumpToGoal, LVarB, 15, FALSE, TRUE, FALSE)
    Call(GetActorPos, LVarB, LVar0, LVar1, LVar2)
    Call(SetActorPos, LVarB, LVar0, LVar1, LVar2)
    Call(ForceHomePos, LVarB, LVar0, LVar1, LVar2)
    Call(SetActorVar, ACTOR_SELF, AVAR_RedPhase_SummonedBobomb, TRUE)
    Call(SetActorVar, LVarB, AVAR_RedPhase_BobOmbIgnited, FALSE)
    Call(EnableIdleScript, LVarB, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, LVarB, TRUE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_Pokey = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR_FAR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 30)
    Call(SetBattleCamDist, 250)
    Thread
        Wait(35)
        Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        Call(MoveBattleCamOver, 30)
    EndThread
    Wait(15)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    Add(LVar2, 2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 15, 0, 0)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedPickupPokey)
    Wait(15)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 15)
    Add(LVar2, 2)
    Call(SetPartPos, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2)
    Call(SetAnimation, ACTOR_SELF, PRT_POKEY, ANIM_KoopaGang_Red_Pokey)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_POKEY, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Add(LVar1, 50)
    // Add(LVar2, 2)
    // Call(SetPartMoveSpeed, ACTOR_SELF, PRT_POKEY, Float(15.0))
    // Call(SetPartJumpGravity, ACTOR_SELF, PRT_POKEY, Float(1.0))
    // Call(JumpPartTo, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2, 15, FALSE)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 5)
    Add(LVar1, 35)
    Add(LVar2, 2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_POKEY, Float(15.0))
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_POKEY, Float(1.5))
    Call(JumpPartTo, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2, 15, TRUE)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedPickup)
    // Wait(10)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_RedThrow)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 4, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseEq(HIT_RESULT_MISS)
        CaseEq(HIT_RESULT_LUCKY)
            Call(PlaySoundAtPart, ACTOR_SELF, PRT_POKEY, SOUND_TOSS)
            Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, -5)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_POKEY, Float(18.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_POKEY, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2, 0, 0, EASING_LINEAR)
            Call(EnemyTestTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_TRIGGER_LUCKY, 0, 0, 0)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Sub(LVar0, 65)
            Set(LVar1, -5)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_POKEY, Float(6.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_POKEY, Float(0.1))
            Call(FlyPartTo, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2, 0, 0, EASING_LINEAR)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_POKEY, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_DigIdle)
            Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            Return
    EndSwitch
    Call(PlaySoundAtPart, ACTOR_SELF, PRT_POKEY, SOUND_TOSS)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_POKEY, Float(30.0))
    Call(SetPartJumpGravity, ACTOR_SELF, PRT_POKEY, Float(0.1))
    Call(FlyPartTo, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2, 0, 0, EASING_LINEAR)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVarF, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgPokey, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 50)
            Set(LVar1, 0)
            Add(LVar2, 2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_POKEY, Float(20.0))
            Call(SetPartJumpGravity, ACTOR_SELF, PRT_POKEY, Float(1.5))
            Call(JumpPartTo, ACTOR_SELF, PRT_POKEY, LVar0, LVar1, LVar2, 15, TRUE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_POKEY, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Wait(10)
            Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_DigIdle)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace red_bandit_koopa

ActorBlueprint RedBanditKoopa = {
    .flags = ACTOR_FLAG_NO_SHADOW, //ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .type = red_bandit_koopa::THIS_ACTOR_TYPE,
    .level = red_bandit_koopa::THIS_LEVEL,
    .maxHP = red_bandit_koopa::hp,
    .partCount = ARRAY_COUNT(red_bandit_koopa::ActorParts),
    .partsData = red_bandit_koopa::ActorParts,
    .initScript = &red_bandit_koopa::EVS_Init,
    .statusTable = red_bandit_koopa::StatusTable,
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
