#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "koopa_gang_tower.hpp"
#include "train_heist_actors.hpp"
#include "sprite/npc/KoopaBros.h" //TODO: Change to KoopaGang2.h
#include "dx/debug_menu.h"

namespace battle::actor {

namespace green_bandit_tower {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleCommand;
extern EvtScript EVS_TestPhase;

// these are the only parameters that vary among koopa bros actors
enum ThisBanditsParams {
    THIS_ACTOR_ID               = GREEN_ACTOR,
    THIS_ANIM_IDLE              = ANIM_KoopaBros_Green_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaBros_Green_Still,
    THIS_ANIM_SLEEP             = ANIM_KoopaBros_Green_Sleep,
    THIS_ANIM_DIZZY             = ANIM_KoopaBros_Green_Dizzy,
    THIS_ANIM_RUN               = ANIM_KoopaBros_Green_Run,
    THIS_ANIM_HURT              = ANIM_KoopaBros_Green_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaBros_Green_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaBros_Green_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaBros_Green_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaBros_Green_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaBros_Green_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaBros_Green_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaBros_Green_StillToppled,
    THIS_ANIM_TOPPLE_DIZZY      = ANIM_KoopaBros_Green_DizzyToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaBros_Green_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaBros_Green_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaBros_Green_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaBros_Green_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaBros_Green_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaBros_Green_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaBros_Green_PointForward,
};

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

// Actor Stats
constexpr s32 hp = 1;

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
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_SLEEP,
    STATUS_KEY_POISON,    THIS_ANIM_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_END,
};

s32 TowerAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOWER_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 TippingAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TIPPING_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 ToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_END,
};

s32 BasicHurtAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

s32 BasicToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_END,
};

s32 ShellAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

Actor* (GetKoopaBrosWithState)(s32 state) {
    Actor* actor = get_actor(GREEN_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(YELLOW_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(BLACK_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(RED_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    return NULL;
}

API_CALLABLE((GetTowerFallPosition)) {
    Bytecode* args = script->ptrReadPos;
    Vec3f temp;
    Vec3f fallPositions[4];
    s32 height;
    s32 ownerState;
    Actor* enemy;
    Vec3f* iVec;
    Vec3f* jVec;
    s32 i, j;

    height = get_actor(BOSS_ACTOR)->state.varTable[AVAR_Boss_TowerHeight];
    switch (height) {
        case 2:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            break;
        case 3:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            break;
        case 4:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosD));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[3].x = enemy->homePos.x;
            fallPositions[3].y = enemy->homePos.y;
            fallPositions[3].z = enemy->homePos.z;
            break;
    }

    for (i = 0; i < height - 1; i++) {
        for (j = i; j < height; j++) {
            iVec = &fallPositions[i];
            jVec = &fallPositions[j];
            if (iVec->x < jVec->x) {
                temp = *iVec;
                *iVec = *jVec;
                *jVec = temp;
            }
        }
    }

    ownerState = get_actor(script->owner1.enemyID)->state.varTable[AVAR_Koopa_State];
    switch (height) {
        case 2:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
            }
            break;
        case 3:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
            }
            break;
        case 4:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosD:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[3].x);
                    evt_set_variable(script, *args++, fallPositions[3].y);
                    evt_set_variable(script, *args++, fallPositions[3].z);
                    break;
            }
            break;
    }
    return ApiStatus_DONE2;
}

API_CALLABLE((GetLastActorEventType)) {
    Bytecode* args = script->ptrReadPos;
    Actor* actor = get_actor(script->owner1.actorID);

    actor->lastEventType = evt_get_variable(script, *args++);
    return ApiStatus_DONE2;
}

// respond to commands issued from BOSS_ACTOR
// (in) LVarA : event
EvtScript EVS_HandleCommand = {
    Call(SetOwnerID, THIS_ACTOR_ID)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfNe(LVar0, 0)
        Return
    EndIf
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Switch(LVarA)
        CaseEq(BOSS_CMD_STABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TowerAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOWER_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_UNSTABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Wait(5)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_HURT)
                    ExecWait(EVS_Enemy_Hit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call((GetLastActorEventType), EVENT_BURN_HIT)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_BURN)
                    SetConst(LVar2, -1)
                    ExecWait(EVS_Enemy_BurnHit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_NO_DAMAGE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_EXIT_SHELL)
                EndCaseGroup
            EndSwitch
            Wait(15)
        CaseEq(BOSS_CMD_TOPPLE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TOPPLE_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_BURN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(GetActorSize, ACTOR_SELF, LVar3, LVar4)
                    DivF(LVar3, Float(2.0))
                    AddF(LVar1, LVar3)
                    AddF(LVar2, Float(5.0))
                    DivF(LVar3, Float(10.0))
                    PlayEffect(EFFECT_SMOKE_BURST, 0, LVar0, LVar1, LVar2, LVar3, 10, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TRY_GET_UP)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Toppled)
                    Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                    Sub(LVar0, 1)
                    IfGt(LVar0, 0)
                        // still has topple turns left, just struggle a bit
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(20)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                    Else
                        // topple turns are over, koopa bros can get up
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(12)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                        Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_JUMP)
                        Thread
                            Set(LVar0, 0)
                            Call(SetActorRotationOffset, ACTOR_SELF, 0, 18, 0)
                            Loop(4)
                                Add(LVar0, 22)
                                Call(SetActorRotation, ACTOR_SELF, 0, 0, LVar0)
                                Wait(1)
                            EndLoop
                        EndThread
                        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(SetActorJumpGravity, ACTOR_SELF, Float(3.0))
                        Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_STEP_A)
                        Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
                        Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -5, 36)
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_GotUp)
                        Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                        Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP)
                    EndIf
                CaseDefault
                    Wait(20)
            EndSwitch
        CaseEq(BOSS_CMD_GET_READY)
            // if koopa just got up, change its state to ready
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_GotUp)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
            EndSwitch
        CaseEq(BOSS_CMD_SHELL_SPIN)
            // if koopa is ready, change its state to shell spin
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Ready)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_ShellSpin)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ShellAnims))
            EndSwitch
    EndSwitch
    Return
    End
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    ExecWait(EVS_TestPhase)
    Return
    End
};

EvtScript EVS_TestPhase = {
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Green Actor ID: (%d)\n", LVar9)
    // Wait(30)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

s32 FlipPosOffsets[] = { 9, 16, 22, 26, 30, 32, 33, 32, 30, 26, 22, 16, 9, 0, 4, 6, 7, 6, 4, 0, 2, 0 };

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
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_FLIP_TRIGGER)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
            Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT)
                Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
                Thread
                    Wait(4)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -60)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -75)
                    Wait(1)
                EndThread
            Else
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
            EndIf
            UseBuf(Ref(FlipPosOffsets))
            Loop(ARRAY_COUNT(FlipPosOffsets))
                BufRead1(LVar0)
                Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
                Wait(1)
            EndLoop
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(GetStatusFlags, ACTOR_SELF, LVar0)
                IfEq(LVar0, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                    Wait(10)
                EndIf
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOPPLE_IDLE)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_IDLE)
                ExecWait(EVS_Enemy_Recover)
            EndIf
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace green_bandit_tower

namespace yellow_bandit_tower {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleCommand;
extern EvtScript EVS_TestPhase;

enum ThisBanditsParams {
    THIS_ACTOR_ID               = YELLOW_ACTOR,
    THIS_ANIM_IDLE              = ANIM_KoopaBros_Yellow_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaBros_Yellow_Still,
    THIS_ANIM_SLEEP             = ANIM_KoopaBros_Yellow_Sleep,
    THIS_ANIM_DIZZY             = ANIM_KoopaBros_Yellow_Dizzy,
    THIS_ANIM_RUN               = ANIM_KoopaBros_Yellow_Run,
    THIS_ANIM_HURT              = ANIM_KoopaBros_Yellow_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaBros_Yellow_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaBros_Yellow_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaBros_Yellow_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaBros_Yellow_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaBros_Yellow_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaBros_Yellow_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaBros_Yellow_StillToppled,
    THIS_ANIM_TOPPLE_DIZZY      = ANIM_KoopaBros_Yellow_DizzyToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaBros_Yellow_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaBros_Yellow_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaBros_Yellow_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaBros_Yellow_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaBros_Yellow_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaBros_Yellow_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaBros_Yellow_PointForward,
};

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

// Actor Stats
constexpr s32 hp = 1;

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
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_SLEEP,
    STATUS_KEY_POISON,    THIS_ANIM_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_END,
};

s32 TowerAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOWER_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 TippingAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TIPPING_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 ToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_END,
};

s32 BasicHurtAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

s32 BasicToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_END,
};

s32 ShellAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

Actor* (GetKoopaBrosWithState)(s32 state) {
    Actor* actor = get_actor(GREEN_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(YELLOW_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(BLACK_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(RED_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    return NULL;
}

API_CALLABLE((GetTowerFallPosition)) {
    Bytecode* args = script->ptrReadPos;
    Vec3f temp;
    Vec3f fallPositions[4];
    s32 height;
    s32 ownerState;
    Actor* enemy;
    Vec3f* iVec;
    Vec3f* jVec;
    s32 i, j;

    height = get_actor(BOSS_ACTOR)->state.varTable[AVAR_Boss_TowerHeight];
    switch (height) {
        case 2:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            break;
        case 3:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            break;
        case 4:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosD));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[3].x = enemy->homePos.x;
            fallPositions[3].y = enemy->homePos.y;
            fallPositions[3].z = enemy->homePos.z;
            break;
    }

    for (i = 0; i < height - 1; i++) {
        for (j = i; j < height; j++) {
            iVec = &fallPositions[i];
            jVec = &fallPositions[j];
            if (iVec->x < jVec->x) {
                temp = *iVec;
                *iVec = *jVec;
                *jVec = temp;
            }
        }
    }

    ownerState = get_actor(script->owner1.enemyID)->state.varTable[AVAR_Koopa_State];
    switch (height) {
        case 2:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
            }
            break;
        case 3:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
            }
            break;
        case 4:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosD:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[3].x);
                    evt_set_variable(script, *args++, fallPositions[3].y);
                    evt_set_variable(script, *args++, fallPositions[3].z);
                    break;
            }
            break;
    }
    return ApiStatus_DONE2;
}

API_CALLABLE((GetLastActorEventType)) {
    Bytecode* args = script->ptrReadPos;
    Actor* actor = get_actor(script->owner1.actorID);

    actor->lastEventType = evt_get_variable(script, *args++);
    return ApiStatus_DONE2;
}

// respond to commands issued from BOSS_ACTOR
// (in) LVarA : event
EvtScript EVS_HandleCommand = {
    Call(SetOwnerID, THIS_ACTOR_ID)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfNe(LVar0, 0)
        Return
    EndIf
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Switch(LVarA)
        CaseEq(BOSS_CMD_STABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TowerAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOWER_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_UNSTABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Wait(5)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_HURT)
                    ExecWait(EVS_Enemy_Hit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call((GetLastActorEventType), EVENT_BURN_HIT)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_BURN)
                    SetConst(LVar2, -1)
                    ExecWait(EVS_Enemy_BurnHit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_NO_DAMAGE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_EXIT_SHELL)
                EndCaseGroup
            EndSwitch
            Wait(15)
        CaseEq(BOSS_CMD_TOPPLE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TOPPLE_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_BURN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(GetActorSize, ACTOR_SELF, LVar3, LVar4)
                    DivF(LVar3, Float(2.0))
                    AddF(LVar1, LVar3)
                    AddF(LVar2, Float(5.0))
                    DivF(LVar3, Float(10.0))
                    PlayEffect(EFFECT_SMOKE_BURST, 0, LVar0, LVar1, LVar2, LVar3, 10, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TRY_GET_UP)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Toppled)
                    Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                    Sub(LVar0, 1)
                    IfGt(LVar0, 0)
                        // still has topple turns left, just struggle a bit
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(20)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                    Else
                        // topple turns are over, koopa bros can get up
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(12)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                        Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_JUMP)
                        Thread
                            Set(LVar0, 0)
                            Call(SetActorRotationOffset, ACTOR_SELF, 0, 18, 0)
                            Loop(4)
                                Add(LVar0, 22)
                                Call(SetActorRotation, ACTOR_SELF, 0, 0, LVar0)
                                Wait(1)
                            EndLoop
                        EndThread
                        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(SetActorJumpGravity, ACTOR_SELF, Float(3.0))
                        Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_STEP_A)
                        Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
                        Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -5, 36)
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_GotUp)
                        Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                        Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP)
                    EndIf
                CaseDefault
                    Wait(20)
            EndSwitch
        CaseEq(BOSS_CMD_GET_READY)
            // if koopa just got up, change its state to ready
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_GotUp)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
            EndSwitch
        CaseEq(BOSS_CMD_SHELL_SPIN)
            // if koopa is ready, change its state to shell spin
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Ready)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_ShellSpin)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ShellAnims))
            EndSwitch
    EndSwitch
    Return
    End
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    ExecWait(EVS_TestPhase)
    Return
    End
};

EvtScript EVS_TestPhase = {
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Yellow Actor ID: (%d)\n", LVar9)
    // Wait(30)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

s32 FlipPosOffsets[] = { 9, 16, 22, 26, 30, 32, 33, 32, 30, 26, 22, 16, 9, 0, 4, 6, 7, 6, 4, 0, 2, 0 };

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
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_FLIP_TRIGGER)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
            Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT)
                Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
                Thread
                    Wait(4)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -60)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -75)
                    Wait(1)
                EndThread
            Else
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
            EndIf
            UseBuf(Ref(FlipPosOffsets))
            Loop(ARRAY_COUNT(FlipPosOffsets))
                BufRead1(LVar0)
                Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
                Wait(1)
            EndLoop
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(GetStatusFlags, ACTOR_SELF, LVar0)
                IfEq(LVar0, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                    Wait(10)
                EndIf
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOPPLE_IDLE)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_IDLE)
                ExecWait(EVS_Enemy_Recover)
            EndIf
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace yellow_bandit_tower

namespace black_bandit_tower {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleCommand;
extern EvtScript EVS_TestPhase;

// these are the only parameters that vary among koopa bros actors
enum ThisBanditsParams {
    THIS_ACTOR_ID               = BLACK_ACTOR,
    THIS_ANIM_IDLE              = ANIM_KoopaBros_Black_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaBros_Black_Still,
    THIS_ANIM_SLEEP             = ANIM_KoopaBros_Black_Sleep,
    THIS_ANIM_DIZZY             = ANIM_KoopaBros_Black_Dizzy,
    THIS_ANIM_RUN               = ANIM_KoopaBros_Black_Run,
    THIS_ANIM_HURT              = ANIM_KoopaBros_Black_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaBros_Black_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaBros_Black_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaBros_Black_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaBros_Black_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaBros_Black_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaBros_Black_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaBros_Black_StillToppled,
    THIS_ANIM_TOPPLE_DIZZY      = ANIM_KoopaBros_Black_DizzyToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaBros_Black_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaBros_Black_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaBros_Black_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaBros_Black_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaBros_Black_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaBros_Black_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaBros_Black_PointForward,
};

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

// Actor Stats
constexpr s32 hp = 1;

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
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_SLEEP,
    STATUS_KEY_POISON,    THIS_ANIM_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_END,
};

s32 TowerAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOWER_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 TippingAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TIPPING_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 ToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_END,
};

s32 BasicHurtAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

s32 BasicToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_END,
};

s32 ShellAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

Actor* (GetKoopaBrosWithState)(s32 state) {
    Actor* actor = get_actor(GREEN_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(YELLOW_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(BLACK_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(RED_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    return NULL;
}

API_CALLABLE((GetTowerFallPosition)) {
    Bytecode* args = script->ptrReadPos;
    Vec3f temp;
    Vec3f fallPositions[4];
    s32 height;
    s32 ownerState;
    Actor* enemy;
    Vec3f* iVec;
    Vec3f* jVec;
    s32 i, j;

    height = get_actor(BOSS_ACTOR)->state.varTable[AVAR_Boss_TowerHeight];
    switch (height) {
        case 2:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            break;
        case 3:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            break;
        case 4:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosD));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[3].x = enemy->homePos.x;
            fallPositions[3].y = enemy->homePos.y;
            fallPositions[3].z = enemy->homePos.z;
            break;
    }

    for (i = 0; i < height - 1; i++) {
        for (j = i; j < height; j++) {
            iVec = &fallPositions[i];
            jVec = &fallPositions[j];
            if (iVec->x < jVec->x) {
                temp = *iVec;
                *iVec = *jVec;
                *jVec = temp;
            }
        }
    }

    ownerState = get_actor(script->owner1.enemyID)->state.varTable[AVAR_Koopa_State];
    switch (height) {
        case 2:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
            }
            break;
        case 3:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
            }
            break;
        case 4:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosD:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[3].x);
                    evt_set_variable(script, *args++, fallPositions[3].y);
                    evt_set_variable(script, *args++, fallPositions[3].z);
                    break;
            }
            break;
    }
    return ApiStatus_DONE2;
}

API_CALLABLE((GetLastActorEventType)) {
    Bytecode* args = script->ptrReadPos;
    Actor* actor = get_actor(script->owner1.actorID);

    actor->lastEventType = evt_get_variable(script, *args++);
    return ApiStatus_DONE2;
}

// respond to commands issued from BOSS_ACTOR
// (in) LVarA : event
EvtScript EVS_HandleCommand = {
    Call(SetOwnerID, THIS_ACTOR_ID)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfNe(LVar0, 0)
        Return
    EndIf
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Switch(LVarA)
        CaseEq(BOSS_CMD_STABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TowerAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOWER_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_UNSTABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Wait(5)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_HURT)
                    ExecWait(EVS_Enemy_Hit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call((GetLastActorEventType), EVENT_BURN_HIT)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_BURN)
                    SetConst(LVar2, -1)
                    ExecWait(EVS_Enemy_BurnHit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_NO_DAMAGE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_EXIT_SHELL)
                EndCaseGroup
            EndSwitch
            Wait(15)
        CaseEq(BOSS_CMD_TOPPLE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TOPPLE_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_BURN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(GetActorSize, ACTOR_SELF, LVar3, LVar4)
                    DivF(LVar3, Float(2.0))
                    AddF(LVar1, LVar3)
                    AddF(LVar2, Float(5.0))
                    DivF(LVar3, Float(10.0))
                    PlayEffect(EFFECT_SMOKE_BURST, 0, LVar0, LVar1, LVar2, LVar3, 10, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TRY_GET_UP)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Toppled)
                    Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                    Sub(LVar0, 1)
                    IfGt(LVar0, 0)
                        // still has topple turns left, just struggle a bit
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(20)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                    Else
                        // topple turns are over, koopa bros can get up
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(12)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                        Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_JUMP)
                        Thread
                            Set(LVar0, 0)
                            Call(SetActorRotationOffset, ACTOR_SELF, 0, 18, 0)
                            Loop(4)
                                Add(LVar0, 22)
                                Call(SetActorRotation, ACTOR_SELF, 0, 0, LVar0)
                                Wait(1)
                            EndLoop
                        EndThread
                        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(SetActorJumpGravity, ACTOR_SELF, Float(3.0))
                        Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_STEP_A)
                        Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
                        Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -5, 36)
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_GotUp)
                        Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                        Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP)
                    EndIf
                CaseDefault
                    Wait(20)
            EndSwitch
        CaseEq(BOSS_CMD_GET_READY)
            // if koopa just got up, change its state to ready
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_GotUp)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
            EndSwitch
        CaseEq(BOSS_CMD_SHELL_SPIN)
            // if koopa is ready, change its state to shell spin
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Ready)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_ShellSpin)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ShellAnims))
            EndSwitch
    EndSwitch
    Return
    End
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    ExecWait(EVS_TestPhase)
    Return
    End
};

EvtScript EVS_TestPhase = {
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Black Actor ID: (%d)\n", LVar9)
    // Wait(30)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

s32 FlipPosOffsets[] = { 9, 16, 22, 26, 30, 32, 33, 32, 30, 26, 22, 16, 9, 0, 4, 6, 7, 6, 4, 0, 2, 0 };

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
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_FLIP_TRIGGER)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
            Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT)
                Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
                Thread
                    Wait(4)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -60)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -75)
                    Wait(1)
                EndThread
            Else
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
            EndIf
            UseBuf(Ref(FlipPosOffsets))
            Loop(ARRAY_COUNT(FlipPosOffsets))
                BufRead1(LVar0)
                Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
                Wait(1)
            EndLoop
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(GetStatusFlags, ACTOR_SELF, LVar0)
                IfEq(LVar0, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                    Wait(10)
                EndIf
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOPPLE_IDLE)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_IDLE)
                ExecWait(EVS_Enemy_Recover)
            EndIf
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace black_bandit_tower

namespace red_bandit_tower {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleCommand;
extern EvtScript EVS_TestPhase;

// these are the only parameters that vary among koopa bros actors
enum ThisBanditsParams {
    THIS_ACTOR_ID               = RED_ACTOR,
    THIS_ANIM_IDLE              = ANIM_KoopaBros_Red_Idle,
    THIS_ANIM_STILL             = ANIM_KoopaBros_Red_Still,
    THIS_ANIM_SLEEP             = ANIM_KoopaBros_Red_Sleep,
    THIS_ANIM_DIZZY             = ANIM_KoopaBros_Red_Dizzy,
    THIS_ANIM_RUN               = ANIM_KoopaBros_Red_Run,
    THIS_ANIM_HURT              = ANIM_KoopaBros_Red_Hurt,
    THIS_ANIM_HURT_STILL        = ANIM_KoopaBros_Red_HurtStill,
    THIS_ANIM_BURN              = ANIM_KoopaBros_Red_BurnHurt,
    THIS_ANIM_BURN_STILL        = ANIM_KoopaBros_Red_BurnStill,
    THIS_ANIM_TOWER_IDLE        = ANIM_KoopaBros_Red_IdleCrouch,
    THIS_ANIM_TOWER_STILL       = ANIM_KoopaBros_Red_StillCrouch,
    THIS_ANIM_TOPPLE_IDLE       = ANIM_KoopaBros_Red_IdleToppled,
    THIS_ANIM_TOPPLE_STILL      = ANIM_KoopaBros_Red_StillToppled,
    THIS_ANIM_TOPPLE_DIZZY      = ANIM_KoopaBros_Red_DizzyToppled,
    THIS_ANIM_TIPPING_IDLE      = ANIM_KoopaBros_Red_IdleTipping,
    THIS_ANIM_TOP_ENTER_SHELL   = ANIM_KoopaBros_Red_TopEnterShell,
    THIS_ANIM_TOP_EXIT_SHELL    = ANIM_KoopaBros_Red_TopExitShell,
    THIS_ANIM_ENTER_SHELL       = ANIM_KoopaBros_Red_EnterShell,
    THIS_ANIM_EXIT_SHELL        = ANIM_KoopaBros_Red_ExitShell,
    THIS_ANIM_SHELL_SPIN        = ANIM_KoopaBros_Red_ShellSpin,
    THIS_ANIM_POINT             = ANIM_KoopaBros_Red_PointForward,
};

enum ActorPartIDs {
    PRT_MAIN            = 1,
};

// Actor Stats
constexpr s32 hp = 1;

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
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_SLEEP,
    STATUS_KEY_POISON,    THIS_ANIM_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_DIZZY,
    STATUS_END,
};

s32 TowerAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOWER_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 TippingAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TIPPING_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOWER_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOWER_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOWER_STILL,
    STATUS_END,
};

s32 ToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_KEY_STONE,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_SLEEP,     THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_POISON,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_TOPPLE_STILL,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_KEY_DIZZY,     THIS_ANIM_TOPPLE_DIZZY,
    STATUS_END,
};

s32 BasicHurtAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

s32 BasicToppledAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_TOPPLE_IDLE,
    STATUS_END,
};

s32 ShellAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_HURT_STILL,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

Actor* (GetKoopaBrosWithState)(s32 state) {
    Actor* actor = get_actor(GREEN_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(YELLOW_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(BLACK_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    actor = get_actor(RED_ACTOR);
    if (actor != NULL && actor->state.varTable[AVAR_Koopa_State] == state) {
        return actor;
    }

    return NULL;
}

API_CALLABLE((GetTowerFallPosition)) {
    Bytecode* args = script->ptrReadPos;
    Vec3f temp;
    Vec3f fallPositions[4];
    s32 height;
    s32 ownerState;
    Actor* enemy;
    Vec3f* iVec;
    Vec3f* jVec;
    s32 i, j;

    height = get_actor(BOSS_ACTOR)->state.varTable[AVAR_Boss_TowerHeight];
    switch (height) {
        case 2:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            break;
        case 3:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            break;
        case 4:
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosA));
            fallPositions[0].x = enemy->homePos.x;
            fallPositions[0].y = enemy->homePos.y;
            fallPositions[0].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosD));
            fallPositions[1].x = enemy->homePos.x;
            fallPositions[1].y = enemy->homePos.y;
            fallPositions[1].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosC));
            fallPositions[2].x = enemy->homePos.x;
            fallPositions[2].y = enemy->homePos.y;
            fallPositions[2].z = enemy->homePos.z;
            enemy = (GetKoopaBrosWithState(AVAL_Koopa_State_PosB));
            fallPositions[3].x = enemy->homePos.x;
            fallPositions[3].y = enemy->homePos.y;
            fallPositions[3].z = enemy->homePos.z;
            break;
    }

    for (i = 0; i < height - 1; i++) {
        for (j = i; j < height; j++) {
            iVec = &fallPositions[i];
            jVec = &fallPositions[j];
            if (iVec->x < jVec->x) {
                temp = *iVec;
                *iVec = *jVec;
                *jVec = temp;
            }
        }
    }

    ownerState = get_actor(script->owner1.enemyID)->state.varTable[AVAR_Koopa_State];
    switch (height) {
        case 2:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
            }
            break;
        case 3:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
            }
            break;
        case 4:
            switch (ownerState) {
                case AVAL_Koopa_State_PosA:
                    evt_set_variable(script, *args++, fallPositions[0].x);
                    evt_set_variable(script, *args++, fallPositions[0].y);
                    evt_set_variable(script, *args++, fallPositions[0].z);
                    break;
                case AVAL_Koopa_State_PosD:
                    evt_set_variable(script, *args++, fallPositions[1].x);
                    evt_set_variable(script, *args++, fallPositions[1].y);
                    evt_set_variable(script, *args++, fallPositions[1].z);
                    break;
                case AVAL_Koopa_State_PosC:
                    evt_set_variable(script, *args++, fallPositions[2].x);
                    evt_set_variable(script, *args++, fallPositions[2].y);
                    evt_set_variable(script, *args++, fallPositions[2].z);
                    break;
                case AVAL_Koopa_State_PosB:
                    evt_set_variable(script, *args++, fallPositions[3].x);
                    evt_set_variable(script, *args++, fallPositions[3].y);
                    evt_set_variable(script, *args++, fallPositions[3].z);
                    break;
            }
            break;
    }
    return ApiStatus_DONE2;
}

API_CALLABLE((GetLastActorEventType)) {
    Bytecode* args = script->ptrReadPos;
    Actor* actor = get_actor(script->owner1.actorID);

    actor->lastEventType = evt_get_variable(script, *args++);
    return ApiStatus_DONE2;
}

// respond to commands issued from BOSS_ACTOR
// (in) LVarA : event
EvtScript EVS_HandleCommand = {
    Call(SetOwnerID, THIS_ACTOR_ID)
    Call(GetStatusFlags, ACTOR_SELF, LVar0)
    IfNe(LVar0, 0)
        Return
    EndIf
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Switch(LVarA)
        CaseEq(BOSS_CMD_STABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TowerAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOWER_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_UNSTABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Wait(5)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_HURT)
                    ExecWait(EVS_Enemy_Hit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call((GetLastActorEventType), EVENT_BURN_HIT)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_BURN)
                    SetConst(LVar2, -1)
                    ExecWait(EVS_Enemy_BurnHit)
                    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                    BitwiseAndConst(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                    IfNe(LVar0, AFLAG_Boss_PlayerHitTower | AFLAG_Boss_PartnerHitTower)
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(TippingAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TIPPING_IDLE)
                    Else
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    EndIf
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_NO_DAMAGE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    SetConst(LVar0, PRT_MAIN)
                    SetConst(LVar1, THIS_ANIM_ENTER_SHELL)
                    ExecWait(EVS_Enemy_NoDamageHit)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_EXIT_SHELL)
                EndCaseGroup
            EndSwitch
            Wait(15)
        CaseEq(BOSS_CMD_TOPPLE_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TOPPLE_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Set(LFlag0, FALSE)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, TRUE)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_BURN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, FALSE, TRUE, FALSE)
                    IfEq(LFlag0, TRUE)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(GetActorSize, ACTOR_SELF, LVar3, LVar4)
                    DivF(LVar3, Float(2.0))
                    AddF(LVar1, LVar3)
                    AddF(LVar2, Float(5.0))
                    DivF(LVar3, Float(10.0))
                    PlayEffect(EFFECT_SMOKE_BURST, 0, LVar0, LVar1, LVar2, LVar3, 10, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
                    Call(JumpToGoal, ACTOR_SELF, 5, FALSE, TRUE, FALSE)
                    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
                    Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
                    Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
                EndCaseGroup
            EndSwitch
        CaseEq(BOSS_CMD_TRY_GET_UP)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Toppled)
                    Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                    Sub(LVar0, 1)
                    IfGt(LVar0, 0)
                        // still has topple turns left, just struggle a bit
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(20)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                    Else
                        // topple turns are over, koopa bros can get up
                        Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
                        Wait(12)
                        Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
                        Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP, SOUND_NONE, 0)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_JUMP)
                        Thread
                            Set(LVar0, 0)
                            Call(SetActorRotationOffset, ACTOR_SELF, 0, 18, 0)
                            Loop(4)
                                Add(LVar0, 22)
                                Call(SetActorRotation, ACTOR_SELF, 0, 0, LVar0)
                                Wait(1)
                            EndLoop
                        EndThread
                        Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(SetActorJumpGravity, ACTOR_SELF, Float(3.0))
                        Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                        Call(JumpToGoal, ACTOR_SELF, 8, FALSE, TRUE, FALSE)
                        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_STEP_A)
                        Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
                        Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -5, 36)
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_GotUp)
                        Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                        Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_IDLE)
                        Call(ResetActorSounds, ACTOR_SELF, ACTOR_SOUND_JUMP)
                    EndIf
                CaseDefault
                    Wait(20)
            EndSwitch
        CaseEq(BOSS_CMD_GET_READY)
            // if koopa just got up, change its state to ready
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_GotUp)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
            EndSwitch
        CaseEq(BOSS_CMD_SHELL_SPIN)
            // if koopa is ready, change its state to shell spin
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_Ready)
                    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_ShellSpin)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ShellAnims))
            EndSwitch
    EndSwitch
    Return
    End
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    ExecWait(EVS_TestPhase)
    Return
    End
};

EvtScript EVS_TestPhase = {
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Red Actor ID: (%d)\n", LVar9)
    // Wait(30)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

s32 FlipPosOffsets[] = { 9, 16, 22, 26, 30, 32, 33, 32, 30, 26, 22, 16, 9, 0, 4, 6, 7, 6, 4, 0, 2, 0 };

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
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN)
            SetConst(LVar2, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_BURN_STILL)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_FLIP_TRIGGER)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Toppled)
            Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 2)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ToppledAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))
            Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 18)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT)
                Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
                Thread
                    Wait(4)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -30)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -60)
                    Wait(1)
                    Call(SetActorRotation, ACTOR_SELF, 0, 0, -75)
                    Wait(1)
                EndThread
            Else
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
            EndIf
            UseBuf(Ref(FlipPosOffsets))
            Loop(ARRAY_COUNT(FlipPosOffsets))
                BufRead1(LVar0)
                Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
                Wait(1)
            EndLoop
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOP_ENTER_SHELL)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(GetStatusFlags, ACTOR_SELF, LVar0)
                IfEq(LVar0, 0)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_EXIT_SHELL)
                    Wait(10)
                EndIf
            Else
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_TOPPLE_IDLE)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, THIS_ANIM_HURT)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            IfEq(LVar0, AVAL_Koopa_State_Ready)
                SetConst(LVar0, PRT_MAIN)
                SetConst(LVar1, THIS_ANIM_IDLE)
                ExecWait(EVS_Enemy_Recover)
            EndIf
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace red_bandit_tower

namespace koopa_gang {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_TestPhase;

// extern EvtScript N(EVS_KoopaBrosEnter);
extern EvtScript EVS_TryFormingTower;
extern EvtScript EVS_BroadcastToKoopaBandits;


enum ActorPartIDs {
    PRT_TOWER           = 1,
};

#include "common/StartRumbleWithParams.inc.c"

// Actor Stats
constexpr s32 hp = 1;

s32 TowerDefense[] = {
    ELEMENT_NORMAL,   1,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,            -1,
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
        .index = PRT_TOWER,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -5, 36 },
        .opacity = 255,
        .idleAnimations = NULL,
        .defenseTable = TowerDefense,
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
    Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, 0)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_None)
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Toppled) // prevents first-turn tower attack
    // Call(SetActorVar, ACTOR_SELF, AVAR_Boss_BowserTaunts, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerHeight, 0)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_TOWER, ACTOR_PART_TARGET_NO_DAMAGE, TRUE)
    ExecWait(EVS_TestPhase)
    Return
    End
};

EvtScript EVS_TestPhase = {
    // Call(GetOwnerID, LVar9)
    // DebugPrintf("Koopa Gang Actor ID: (%d)\n", LVar9)
    // Wait(30)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, FALSE)

    // Call(SetActorFlagBits, ACTOR_ENEMY1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, FALSE)
    // Call(SetPartFlagBits, ACTOR_ENEMY1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)

    // Call(SetActorFlagBits, ACTOR_ENEMY2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
    // Call(SetPartFlagBits, ACTOR_ENEMY2, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, FALSE)
    Return
    End
};

API_CALLABLE((PlayLandOnTowerFX)) {
    Bytecode* args = script->ptrReadPos;
    s32 actorID = evt_get_variable(script, *args++);
    Actor* actor = get_actor(actorID);

    play_movement_dust_effects(2, actor->state.goalPos.x - 15.0f, actor->state.goalPos.y, actor->state.goalPos.z, actor->state.angle);
    return ApiStatus_DONE2;
}

// (in) Var0 : koopa bros actorID
// (in) Var1 : expected tower height
// (in) Var2 : tower index (height - 1, height - 2, ..., 0)
EvtScript EVS_BuildTowerWithKoopa = {
    Call(UseIdleAnimation, LVar0, FALSE)
    Switch(LVar0)
        CaseEq(RED_ACTOR)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_PointForward)
            Call(SetActorYaw, RED_ACTOR, 0)
            Call(GetActorPos, RED_ACTOR, LVar0, LVar1, LVar2)
            Sub(LVar0, 22)
            Add(LVar1, 19)
            Wait(10)
            Wait(30)
            Set(LVar0, 48)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Call(SetBattleCamTarget, LVar2, LVar3, LVar4)
            Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
            Call(SetBattleCamOffsetY, 50)
            Call(SetBattleCamDist, 400)
            Call(MoveBattleCamOver, LVar0)
            Wait(LVar0)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, RED_ACTOR, LVar2, LVar3, LVar4)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_Run)
            Call(GetGoalPos, RED_ACTOR, LVar0, LVar1, LVar2)
            Call(GetActorPos, RED_ACTOR, LVar3, LVar4, LVar5)
            IfLt(LVar0, LVar3)
                Call(SetActorYaw, RED_ACTOR, 0)
            Else
                Call(SetActorYaw, RED_ACTOR, 180)
            EndIf
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_Leap)
            Wait(5)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, RED_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar0, 54)
            Call(AddGoalPos, RED_ACTOR, 15, LVar0, -10)
            Thread
                Wait(10)
                Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_Land)
            EndThread
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_Midair)
            Call(SetActorJumpGravity, RED_ACTOR, Float(1.6))
            Call(JumpToGoal, RED_ACTOR, 20, FALSE, FALSE, FALSE)
            Call((PlayLandOnTowerFX), RED_ACTOR)
            Call(PlaySoundAtActor, RED_ACTOR, SOUND_KOOPA_BROS_LAND)
            Call(GetActorPos, RED_ACTOR, LVar3, LVar4, LVar5)
            Sub(LVar3, 15)
            Call(SetActorPos, RED_ACTOR, LVar3, LVar4, LVar5)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_Idle)
            Wait(5)
            Call(SetActorYaw, RED_ACTOR, 0)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_PointForward)
            Call(PlaySoundAtActor, RED_ACTOR, SOUND_SMALL_LENS_FLARE)
            Call(GetActorPos, RED_ACTOR, LVar0, LVar1, LVar2)
            Sub(LVar0, 22)
            Add(LVar1, 19)
            PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 30, 0)
            Wait(20)
            Wait(10)
            Call(SetAnimation, RED_ACTOR, 1, ANIM_KoopaBros_Red_Idle)
            Call(SetActorVar, RED_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosA)
            Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Stable)
        CaseEq(GREEN_ACTOR)
            Call(SetAnimation, LVarA, 1, ANIM_KoopaBros_Green_IdleCrouch)
            Wait(50)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
                Set(LVar4, 15)
                Call(SetGoalPos, GREEN_ACTOR, LVar2, LVar3, LVar4)
                Call(SetAnimation, GREEN_ACTOR, 1, ANIM_KoopaBros_Green_Run)
                Call(RunToGoal, GREEN_ACTOR, 10, FALSE)
                Call(SetAnimation, GREEN_ACTOR, 1, ANIM_KoopaBros_Green_IdleCrouch)
                Call(SetActorVar, GREEN_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosB)
        CaseEq(YELLOW_ACTOR)
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaBros_Yellow_IdleCrouch)
            Wait(60)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, YELLOW_ACTOR, LVar2, LVar3, LVar4)
            Call(AddGoalPos, YELLOW_ACTOR, 0, 18, -4)
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaBros_Yellow_Leap)
            Wait(5)
            Call(AddGoalPos, YELLOW_ACTOR, 15, 0, 0)
            Thread
                Wait(10)
                Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaBros_Yellow_Land)
            EndThread
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaBros_Yellow_Midair)
            Call(SetActorJumpGravity, YELLOW_ACTOR, Float(1.6))
            Call(JumpToGoal, YELLOW_ACTOR, 20, FALSE, FALSE, FALSE)
            Call((PlayLandOnTowerFX), YELLOW_ACTOR)
            Call(PlaySoundAtActor, YELLOW_ACTOR, SOUND_KOOPA_BROS_LAND)
            Call(GetActorPos, YELLOW_ACTOR, LVar3, LVar4, LVar5)
            Sub(LVar3, 15)
            Call(SetActorPos, YELLOW_ACTOR, LVar3, LVar4, LVar5)
            Call(SetAnimation, YELLOW_ACTOR, 1, ANIM_KoopaBros_Yellow_IdleCrouch)
            Call(SetActorVar, YELLOW_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosC)
        CaseEq(BLACK_ACTOR)
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaBros_Black_IdleCrouch)
            Wait(80)
            Call(GetHomePos, GREEN_ACTOR, LVar2, LVar3, LVar4)
            Set(LVar4, 15)
            Call(SetGoalPos, BLACK_ACTOR, LVar2, LVar3, LVar4)
            Call(AddGoalPos, BLACK_ACTOR, 0, 36, -7)
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaBros_Black_Leap)
            Wait(5)
            Call(AddGoalPos, BLACK_ACTOR, 15, 0, 0)
            Thread
                Wait(10)
                Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaBros_Black_Land)
            EndThread
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaBros_Black_Midair)
            Call(SetActorJumpGravity, BLACK_ACTOR, Float(1.6))
            Call(JumpToGoal, BLACK_ACTOR, 20, FALSE, FALSE, FALSE)
            Call((PlayLandOnTowerFX), BLACK_ACTOR)
            Call(PlaySoundAtActor, BLACK_ACTOR, SOUND_KOOPA_BROS_LAND)
            Call(GetActorPos, BLACK_ACTOR, LVar3, LVar4, LVar5)
            Sub(LVar3, 15)
            Call(SetActorPos, BLACK_ACTOR, LVar3, LVar4, LVar5)
            Call(SetAnimation, BLACK_ACTOR, 1, ANIM_KoopaBros_Black_IdleCrouch)
            Call(SetActorVar, BLACK_ACTOR, AVAR_Koopa_State, AVAL_Koopa_State_PosD)
    EndSwitch
    Return
    End
};

EvtScript EVS_TryFormingTower = {
    #define LBL_WAIT_FOR_TOWER 0

    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerHeight, 4)
    Set(LFlag0, FALSE)

    Call(GetActorVar, GREEN_ACTOR, AVAR_Koopa_State, LVar0)
    IfEq(LVar0, 0)
        Set(LVar0, GREEN_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
        Set(LVar0, YELLOW_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
        Set(LVar0, BLACK_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
        Set(LVar0, RED_ACTOR)
        Exec(EVS_BuildTowerWithKoopa)
    Else
        Return
    EndIf

    // Wait until tower state is stable
    Call(EnableBattleStatusBar, FALSE)
    Label(LBL_WAIT_FOR_TOWER)
        Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
        Wait(1)
        IfEq(LVar0, AVAL_Boss_TowerState_Toppled)
            Goto(LBL_WAIT_FOR_TOWER)
        EndIf
    Call(EnableBattleStatusBar, TRUE)

    // Finalize appearance/state of tower actor
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_POPUP, TRUE)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_TOWER, ACTOR_PART_FLAG_NO_TARGET, FALSE)

    Set(LVar0, 92)
    Call(SetTargetOffset, ACTOR_SELF, PRT_TOWER, -5, 36)
    Call(SetActorSize, ACTOR_SELF, LVar0, 45)

    Call(GetActorPos, RED_ACTOR, LVar2, LVar3, LVar4)
    Call(SetActorPos, ACTOR_SELF, LVar2, LVar3, LVar4)

    Set(LVarA, BOSS_CMD_STABLE)
    ExecWait(EVS_BroadcastToKoopaBandits)
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Stable)

    Return
    End

    #undef LBL_WAIT_FOR_TOWER
};

EvtScript EVS_Broadcast_TowerUnstable = {
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    IfEq(LVar0, AVAL_Boss_TowerState_Unstable)
        Return
    EndIf
    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
    BitwiseOrConst(LVar0, AFLAG_Boss_TowerUnstable)
    Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Unstable)
    Set(LVarA, BOSS_CMD_UNSTABLE)
    ExecWait(EVS_BroadcastToKoopaBandits)
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerHeight, LVarA)
    Set(LVar0, 92)
    Call(SetTargetOffset, ACTOR_SELF, PRT_TOWER, -5, 23)
    Call(SetActorSize, ACTOR_SELF, LVar0, 45)
    Return
    End
};

EvtScript EVS_Broadcast_ToppleHit = {
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Toppled)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    IfNe(LVar0, EVENT_BURN_HIT)
        Set(LVarA, BOSS_CMD_TOPPLE_HIT)
    Else
        Set(LVarA, BOSS_CMD_TOPPLE_BURN_HIT)
    EndIf
    ExecWait(EVS_BroadcastToKoopaBandits)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_APPLY, TRUE)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_TOWER, ACTOR_PART_FLAG_NO_TARGET, TRUE)
    Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
    BitwiseAndConst(LVar0, ~AFLAG_Boss_TowerUnstable)
    Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
    Return
    End
};

// (in) LVarA : event
EvtScript EVS_BroadcastToKoopaBandits = {
    Exec(green_bandit_tower::EVS_HandleCommand)
    Exec(yellow_bandit_tower::EVS_HandleCommand)
    Exec(black_bandit_tower::EVS_HandleCommand)
    ExecGetTID(red_bandit_tower::EVS_HandleCommand, LVar1)
    Label(0)
        IsThreadRunning(LVar1, LVar0)
        IfEq(LVar0, TRUE)
            Wait(1)
            Goto(0)
        EndIf
    Return
    End
};

// adds a random jitter to KoopaBros actors' X position while the tower is unstable
EvtScript EVS_Idle = {
    Label(0)
        Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
        IfNe(LVar0, AVAL_Boss_TowerState_Unstable)
            Wait(1)
            Goto(0)
        EndIf
        // get initial actor positions
        Call(ActorExists, GREEN_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorPos, GREEN_ACTOR, LVarA, LVarE, LVarF)
        EndIf
        Call(ActorExists, YELLOW_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorPos, YELLOW_ACTOR, LVarC, LVarE, LVarF)
        EndIf
        Call(ActorExists, BLACK_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorPos, BLACK_ACTOR, LVarD, LVarE, LVarF)
        EndIf
        Call(ActorExists, RED_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorPos, RED_ACTOR, LVarB, LVarE, LVarF)
        EndIf
        // while tower is unstable, add random X offsets to koopa bros
        Label(1)
            Call(ActorExists, GREEN_ACTOR, LVar0)
            IfNe(LVar0, FALSE)
                Call(GetActorVar, GREEN_ACTOR, AVAR_Koopa_State, LVar0)
                Switch(LVar0)
                    CaseOrEq(AVAL_Koopa_State_PosA)
                    CaseOrEq(AVAL_Koopa_State_PosD)
                    CaseOrEq(AVAL_Koopa_State_PosC)
                    CaseOrEq(AVAL_Koopa_State_PosB)
                        Call(RandInt, 2, LVar0)
                        Set(LVar1, 1)
                        Sub(LVar0, LVar1)
                        Add(LVar0, LVarA)
                        Call(GetActorPos, GREEN_ACTOR, LVar1, LVar2, LVar3)
                        Call(SetActorPos, GREEN_ACTOR, LVar0, LVar2, LVar3)
                    EndCaseGroup
                EndSwitch
            EndIf
            Call(ActorExists, YELLOW_ACTOR, LVar0)
            IfNe(LVar0, FALSE)
                Call(GetActorVar, YELLOW_ACTOR, AVAR_Koopa_State, LVar0)
                Switch(LVar0)
                    CaseOrEq(AVAL_Koopa_State_PosA)
                    CaseOrEq(AVAL_Koopa_State_PosD)
                    CaseOrEq(AVAL_Koopa_State_PosC)
                    CaseOrEq(AVAL_Koopa_State_PosB)
                        Call(RandInt, 2, LVar0)
                        Set(LVar1, 1)
                        Sub(LVar0, LVar1)
                        Add(LVar0, LVarC)
                        Call(GetActorPos, YELLOW_ACTOR, LVar1, LVar2, LVar3)
                        Call(SetActorPos, YELLOW_ACTOR, LVar0, LVar2, LVar3)
                    EndCaseGroup
                EndSwitch
            EndIf
            Call(ActorExists, BLACK_ACTOR, LVar0)
            IfNe(LVar0, FALSE)
                Call(GetActorVar, BLACK_ACTOR, AVAR_Koopa_State, LVar0)
                Switch(LVar0)
                    CaseOrEq(AVAL_Koopa_State_PosA)
                    CaseOrEq(AVAL_Koopa_State_PosD)
                    CaseOrEq(AVAL_Koopa_State_PosC)
                    CaseOrEq(AVAL_Koopa_State_PosB)
                        Call(RandInt, 2, LVar0)
                        Set(LVar1, 1)
                        Sub(LVar0, LVar1)
                        Add(LVar0, LVarD)
                        Call(GetActorPos, BLACK_ACTOR, LVar1, LVar2, LVar3)
                        Call(SetActorPos, BLACK_ACTOR, LVar0, LVar2, LVar3)
                    EndCaseGroup
                EndSwitch
            EndIf
            Call(ActorExists, RED_ACTOR, LVar0)
            IfNe(LVar0, FALSE)
                Call(GetActorVar, RED_ACTOR, AVAR_Koopa_State, LVar0)
                Switch(LVar0)
                    CaseOrEq(AVAL_Koopa_State_PosA)
                    CaseOrEq(AVAL_Koopa_State_PosD)
                    CaseOrEq(AVAL_Koopa_State_PosC)
                    CaseOrEq(AVAL_Koopa_State_PosB)
                        Call(RandInt, 2, LVar0)
                        Set(LVar1, 1)
                        Sub(LVar0, LVar1)
                        Add(LVar0, LVarB)
                        Call(GetActorPos, RED_ACTOR, LVar1, LVar2, LVar3)
                        Call(SetActorPos, RED_ACTOR, LVar0, LVar2, LVar3)
                    EndCaseGroup
                EndSwitch
            EndIf
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfEq(LVar0, AVAL_Boss_TowerState_Unstable)
                Wait(2)
                Goto(1)
            EndIf
        // once tower regains stability, reset X positions to initial values
        Call(ActorExists, GREEN_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorVar, GREEN_ACTOR, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(GetActorPos, GREEN_ACTOR, LVar1, LVar2, LVar3)
                    Call(SetActorPos, GREEN_ACTOR, LVarA, LVar2, LVar3)
                EndCaseGroup
            EndSwitch
        EndIf
        Call(ActorExists, YELLOW_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorVar, YELLOW_ACTOR, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(GetActorPos, YELLOW_ACTOR, LVar1, LVar2, LVar3)
                    Call(SetActorPos, YELLOW_ACTOR, LVarC, LVar2, LVar3)
                EndCaseGroup
            EndSwitch
        EndIf
        Call(ActorExists, BLACK_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorVar, BLACK_ACTOR, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(GetActorPos, BLACK_ACTOR, LVar1, LVar2, LVar3)
                    Call(SetActorPos, BLACK_ACTOR, LVarD, LVar2, LVar3)
                EndCaseGroup
            EndSwitch
        EndIf
        Call(ActorExists, RED_ACTOR, LVar0)
        IfNe(LVar0, FALSE)
            Call(GetActorVar, RED_ACTOR, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseOrEq(AVAL_Koopa_State_PosA)
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(GetActorPos, RED_ACTOR, LVar1, LVar2, LVar3)
                    Call(SetActorPos, RED_ACTOR, LVarB, LVar2, LVar3)
                EndCaseGroup
            EndSwitch
        EndIf
        Wait(1)
        Goto(0)
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_HIT_COMBO)
            // set flags for player or partner hitting the koopa bros tower
            Call(GetBattleFlags, LVar0)
            IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Else
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            Set(LVarA, BOSS_CMD_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Wait(30)
            // if the attack was explosive, set both flags
            Call(GetLastElement, LVar0)
            IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            ExecWait(EVS_Broadcast_TowerUnstable)
        CaseEq(EVENT_HIT)
            // set flags for player or partner hitting the koopa bros tower
            Call(GetBattleFlags, LVar0)
            IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Else
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if the attack was explosive, set both flags
            Call(GetLastElement, LVar0)
            IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if this was the second hit, topple the tower
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            IfFlag(LVar0, AFLAG_Boss_PlayerHitTower)
                IfFlag(LVar0, AFLAG_Boss_PartnerHitTower)
                    ExecWait(EVS_Broadcast_ToppleHit)
                    Wait(20)
                    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
                    Return
                EndIf
            EndIf
            // this was the first hit
            Set(LVarA, BOSS_CMD_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Wait(30)
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfNe(LVar0, AVAL_Boss_TowerState_Unstable)
                ExecWait(EVS_Broadcast_TowerUnstable)
            EndIf
            // Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_Boss_TowerUnstable)
            //     IfNotFlag(LVar0, AFLAG_Boss_Dialogue_WereGoingOver)
            //         Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TopKoopaID, LVar0)
            //         Call(ActorSpeak, MSG_CH1_0109, LVar0, 1, -1, -1)
            //         Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            //         BitwiseOrConst(LVar0, AFLAG_Boss_Dialogue_WereGoingOver)
            //         Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            //         Wait(20)
            //     EndIf
            // EndIf
        CaseEq(EVENT_BURN_HIT)
            // set flags for player or partner hitting the koopa bros tower
            Call(GetBattleFlags, LVar0)
            IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Else
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if the attack was explosive, set both flags
            Call(GetLastElement, LVar0)
            IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if this was the second hit, topple the tower
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            IfFlag(LVar0, AFLAG_Boss_PlayerHitTower)
                IfFlag(LVar0, AFLAG_Boss_PartnerHitTower)
                    ExecWait(EVS_Broadcast_ToppleHit)
                    Wait(20)
                    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
                    Return
                EndIf
            EndIf
            // this was the first hit
            Set(LVarA, BOSS_CMD_BURN_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Wait(30)
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfNe(LVar0, AVAL_Boss_TowerState_Unstable)
                ExecWait(EVS_Broadcast_TowerUnstable)
            EndIf
            // Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_Boss_TowerUnstable)
            //     IfNotFlag(LVar0, AFLAG_Boss_Dialogue_WereGoingOver)
            //         Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TopKoopaID, LVar0)
            //         Call(ActorSpeak, MSG_CH1_0109, LVar0, 1, -1, -1)
            //         Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            //         BitwiseOrConst(LVar0, AFLAG_Boss_Dialogue_WereGoingOver)
            //         Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            //         Wait(20)
            //     EndIf
            // EndIf
        CaseEq(EVENT_ZERO_DAMAGE)
            Set(LVarA, BOSS_CMD_NO_DAMAGE_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
        CaseEq(EVENT_IMMUNE)
            // set both flags if the tower is already unstable
            Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
            IfEq(LVar0, AVAL_Boss_TowerState_Unstable)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PlayerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
                BitwiseOrConst(LVar0, AFLAG_Boss_PartnerHitTower)
                Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            EndIf
            // if this was the second hit, topple the tower
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            IfFlag(LVar0, AFLAG_Boss_PlayerHitTower)
                IfFlag(LVar0, AFLAG_Boss_PartnerHitTower)
                    ExecWait(EVS_Broadcast_ToppleHit)
                    Wait(20)
                    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
                    Return
                EndIf
            EndIf
            // this was the first hit
            Set(LVarA, BOSS_CMD_NO_DAMAGE_HIT)
            ExecWait(EVS_BroadcastToKoopaBandits)
            // Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_Boss_TowerUnstable)
            //     IfNotFlag(LVar0, AFLAG_Boss_Dialogue_WereGoingOver)
            //         Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TopKoopaID, LVar0)
            //         Call(ActorSpeak, MSG_CH1_0109, LVar0, 1, -1, -1)
            //         Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            //         BitwiseOrConst(LVar0, AFLAG_Boss_Dialogue_WereGoingOver)
            //         Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            //         Wait(20)
            //     EndIf
            // EndIf
        CaseOrEq(EVENT_DEATH)
        CaseOrEq(EVENT_BURN_DEATH)
        EndCaseGroup
        CaseDefault
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(GetActorPos, BOSS_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Tower Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, GREEN_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Green Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, YELLOW_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Yellow Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, BLACK_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Black Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    Call(GetActorPos, RED_ACTOR, LVar0, LVar1, LVar2)
    DebugPrintf("Red Pos: (%d, %d, %d)\n", LVar0, LVar1, LVar2)
    // reform stable tower if tipping
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    Switch(LVar0)
        CaseEq(AVAL_Boss_TowerState_None)
        CaseEq(AVAL_Boss_TowerState_Stable)
            Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_ShellSpin)
            Set(LVarA, BOSS_CMD_SHELL_SPIN)
            ExecWait(EVS_BroadcastToKoopaBandits)
        CaseEq(AVAL_Boss_TowerState_Unstable)
            Wait(30)
            Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Stable)
            Set(LVarA, BOSS_CMD_STABLE)
            ExecWait(EVS_BroadcastToKoopaBandits)
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            BitwiseAndConst(LVar0, ~AFLAG_Boss_TowerUnstable)
            Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Set(LVar0, 92)
            Call(SetTargetOffset, ACTOR_SELF, PRT_TOWER, -5, 36)
            Call(SetActorSize, ACTOR_SELF, LVar0, 45)
        CaseEq(AVAL_Boss_TowerState_Toppled)
        CaseEq(AVAL_Boss_TowerState_ShellSpin)
    EndSwitch
    // find if any koopa bros are toppled
    Call(GetActorVar, GREEN_ACTOR, AVAR_Koopa_State, LVar0)
    IfEq(LVar0, AVAL_Koopa_State_Toppled)
        Set(LFlag0, TRUE)
    EndIf
    // zoom in to show the toppled koopa bros and have them try to get up
    IfEq(LFlag0, TRUE)
        Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
        Call(SetBattleCamTarget, 40, 0, 0)
        Call(SetBattleCamDist, 350)
        Call(SetBattleCamOffsetY, 40)
        Call(MoveBattleCamOver, 15)
        Wait(15)
    EndIf
    Set(LVarA, BOSS_CMD_TRY_GET_UP)
    ExecWait(EVS_BroadcastToKoopaBandits)
    // try tower operations
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    IfNe(LVar0, AVAL_Boss_TowerState_Stable)
        ExecWait(EVS_TryFormingTower)
    EndIf
    Set(LVarA, BOSS_CMD_GET_READY)
    ExecWait(EVS_BroadcastToKoopaBandits)
    Wait(5)
    Call(GetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, LVar0)
    IfEq(LVar0, AVAL_Boss_TowerState_Toppled)
        Return
    EndIf
    Call(SetActorVar, ACTOR_SELF, AVAR_Boss_TowerState, AVAL_Boss_TowerState_Stable)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(GetEnemyMaxHP, ACTOR_SELF, LVar0)
    Call(SetEnemyHP, ACTOR_SELF, LVar0)
    Call(GetBattlePhase, LVar0)
    Switch(LVar0)
        CaseEq(PHASE_PLAYER_BEGIN)
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            BitwiseAndConst(LVar0, ~AFLAG_Boss_PlayerHitTower)
            Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            Call(GetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
            BitwiseAndConst(LVar0, ~AFLAG_Boss_PartnerHitTower)
            Call(SetActorVar, BOSS_ACTOR, AVAR_Boss_Flags, LVar0)
        CaseEq(PHASE_ENEMY_BEGIN)
        CaseEq(PHASE_ENEMY_END)
    EndSwitch
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace koopa_gang

ActorBlueprint KoopaGang = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_SHADOW,
    .maxHP = koopa_gang::hp,
    .type = ACTOR_TYPE_KOOPA_GANG,
    .level = ACTOR_LEVEL_KOOPA_GANG,
    .partCount = ARRAY_COUNT(koopa_gang::ActorParts),
    .partsData = koopa_gang::ActorParts,
    .initScript = &koopa_gang::EVS_Init,
    .statusTable = koopa_gang::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 4,
    .powerBounceChance = 90,
    .coinReward = 0,
    .size = { 92, 45 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint GreenBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = green_bandit_tower::hp,
    .type = ACTOR_TYPE_GREEN_BANDIT,
    .level = ACTOR_LEVEL_GREEN_BANDIT,
    .partCount = ARRAY_COUNT(green_bandit_tower::ActorParts),
    .partsData = green_bandit_tower::ActorParts,
    .initScript = &green_bandit_tower::EVS_Init,
    .statusTable = green_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint YellowBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = yellow_bandit_tower::hp,
    .type = ACTOR_TYPE_YELLOW_BANDIT,
    .level = ACTOR_LEVEL_YELLOW_BANDIT,
    .partCount = ARRAY_COUNT(yellow_bandit_tower::ActorParts),
    .partsData = yellow_bandit_tower::ActorParts,
    .initScript = &yellow_bandit_tower::EVS_Init,
    .statusTable = yellow_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint BlackBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = black_bandit_tower::hp,
    .type = ACTOR_TYPE_BLACK_BANDIT,
    .level = ACTOR_LEVEL_BLACK_BANDIT,
    .partCount = ARRAY_COUNT(black_bandit_tower::ActorParts),
    .partsData = black_bandit_tower::ActorParts,
    .initScript = &black_bandit_tower::EVS_Init,
    .statusTable = black_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

ActorBlueprint RedBanditTower = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = red_bandit_tower::hp,
    .type = ACTOR_TYPE_RED_BANDIT,
    .level = ACTOR_LEVEL_RED_BANDIT,
    .partCount = ARRAY_COUNT(red_bandit_tower::ActorParts),
    .partsData = red_bandit_tower::ActorParts,
    .initScript = &red_bandit_tower::EVS_Init,
    .statusTable = red_bandit_tower::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 0,
    .size = { 38, 42 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
