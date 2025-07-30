#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleCommand;
extern EvtScript EVS_BeginPhase;

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
    STATUS_KEY_POISON,    THIS_ANIM_STILL,
    STATUS_KEY_STOP,      THIS_ANIM_STILL,
    STATUS_KEY_STATIC,    THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
    STATUS_KEY_PARALYZE,  THIS_ANIM_STILL,
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

s32 ShellSpinAnims[] = {
    STATUS_KEY_NORMAL,    THIS_ANIM_SHELL_SPIN,
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
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Switch(LVarA)
        CaseEq(BOSS_CMD_STABLE)
            Call(GetActorVar, ACTOR_SELF, AVAR_Koopa_State, LVar0)
            Switch(LVar0)
                CaseEq(AVAL_Koopa_State_PosA)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOP_ENTER_SHELL)
                    Wait(10)
                    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_TOWER_SPIN_3)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ShellSpinAnims))
                CaseOrEq(AVAL_Koopa_State_PosD)
                CaseOrEq(AVAL_Koopa_State_PosC)
                CaseOrEq(AVAL_Koopa_State_PosB)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_ENTER_SHELL)
                    Wait(10)
                    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_TOWER_SPIN_3)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_SHELL_SPIN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(ShellSpinAnims))
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
                    Set(LFlag0, false)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, true)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_HURT_STILL)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, false, true, false)
                    IfEq(LFlag0, true)
                        Call(N(StartRumbleWithParams), 256, 5)
                        Thread
                            Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
                        EndThread
                    EndIf
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_TOPPLE_IDLE)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicToppledAnims))
                    Call(JumpToGoal, ACTOR_SELF, 10, false, true, false)
                    Call(JumpToGoal, ACTOR_SELF, 5, false, true, false)
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
                    Set(LFlag0, false)
                    IfEq(LVar0, AVAL_Koopa_State_PosA)
                        Set(LFlag0, true)
                    EndIf
                    Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
                    Call((GetTowerFallPosition), LVar0, LVar1, LVar2)
                    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
                    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, THIS_ANIM_BURN)
                    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(BasicHurtAnims))
                    Call(JumpToGoal, ACTOR_SELF, 20, false, true, false)
                    IfEq(LFlag0, true)
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
                    Call(JumpToGoal, ACTOR_SELF, 10, false, true, false)
                    Call(JumpToGoal, ACTOR_SELF, 5, false, true, false)
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
                        // update turn counter
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, LVar0)
                    Else
                        // topple turns are over, koopa bros can get up
                        Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, -5, 36)
                        Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
                        Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, false)
                        Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))
                        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
                    EndIf
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
    ExecWait(EVS_BeginPhase)
    Return
    End
};

EvtScript EVS_BeginPhase = {
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

EvtScript EVS_HandleEvent = {
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
