#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/ShyGuyRider.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"

namespace battle::actor {

namespace shy_guy_rider {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_Attack_ArrowShot;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    PRT_ARROW       = 2,
};

// Actor Stats
constexpr s32 hp = 8;
constexpr s32 dmgArrowShot = 1;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_ShyGuyRider_Idle,
    STATUS_KEY_STONE,     ANIM_ShyGuyRider_Idle,
    STATUS_KEY_SLEEP,     ANIM_ShyGuyRider_Idle,
    STATUS_KEY_POISON,    ANIM_ShyGuyRider_Idle,
    STATUS_KEY_STOP,      ANIM_ShyGuyRider_Idle,
    STATUS_KEY_STATIC,    ANIM_ShyGuyRider_Idle,
    STATUS_KEY_PARALYZE,  ANIM_ShyGuyRider_Idle,
    STATUS_KEY_DIZZY,     ANIM_ShyGuyRider_Idle,
    STATUS_KEY_FEAR,      ANIM_ShyGuyRider_Idle,
    STATUS_END,
};

s32 ArrowAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_ShyGuyRider_Arrow,
    STATUS_END,
};

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
        .flags = ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -9, 46 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -6, -17 },
    },
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_ARROW,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = ArrowAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(HPBarToHome, ACTOR_SELF)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
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
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    ExecWait(EVS_Attack_ArrowShot)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Attack_ArrowShot = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_ShyGuyRider_ShootArrow)
    Wait(17)
    Call(SetAnimation, ACTOR_SELF, PRT_ARROW, ANIM_ShyGuyRider_Arrow)
    Call(EnemyTestTarget, ACTOR_SELF, LVar0, 0, 0, 1, BS_FLAGS1_INCLUDE_POWER_UPS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_MISS)
        CaseOrEq(HIT_RESULT_LUCKY)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
            Wait(5)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Sub(LVar0, 15)
            Add(LVar1, 48)
            Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_ARROW, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_ARROW, Float(16.0))
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(FlyPartTo, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2, 0, 0, EASING_LINEAR)
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
    Wait(5)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 15)
    Add(LVar1, 48)
    Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_ARROW, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_ARROW, Float(16.0))
    Call(FlyPartTo, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2, 0, 0, EASING_LINEAR)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, DMG_STATUS_KEY(STATUS_FLAG_POISON, 2, 100), dmgArrowShot, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_ARROW, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace shy_guy_rider

ActorBlueprint ShyGuyRider = {
    .flags = ACTOR_FLAG_FLYING | ACTOR_FLAG_NO_HEALTH_BAR,
    .type = ACTOR_TYPE_SHY_GUY_RIDER,
    .level = ACTOR_LEVEL_SHY_GUY_RIDER,
    .maxHP = shy_guy_rider::hp,
    .partCount = ARRAY_COUNT(shy_guy_rider::ActorParts),
    .partsData = shy_guy_rider::ActorParts,
    .initScript = &shy_guy_rider::EVS_Init,
    .statusTable = shy_guy_rider::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 0,
    .coinReward = 1,
    .size = { 64, 56 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -22, 24 },
    .statusTextOffset = { 11, 40 },
};

}; // namespace battle::actor
