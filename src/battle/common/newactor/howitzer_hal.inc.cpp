#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/HowitzerHal.h"
#include "boss.hpp"

namespace battle::actor {

namespace howitzer_hal {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_Attack_FireBullet;

enum ActorPartIDs {
    PRT_MAIN            = 1,
    PRT_BULLET          = 2,
};

// Actor Stats
constexpr s32 hp = 4;
constexpr s32 dmgImpact = 1;

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   1,
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
        .targetOffset = { -6, 29 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -9 },
    },
    {
        .flags = ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 0 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_HowitzerHal_Idle,
    STATUS_KEY_STONE,     ANIM_HowitzerHal_Still,
    STATUS_KEY_STOP,      ANIM_HowitzerHal_Still,
    STATUS_KEY_PARALYZE,  ANIM_HowitzerHal_Still,
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

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
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
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetActorVar, ACTOR_BRIGADER_BONES, AVAR_GreenPhase_BrigaderCommand, LVar0)
    IfEq(LVar0, true)
        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
        ExecWait(EVS_Attack_FireBullet)
    Else
        Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DisableAnims))
    EndIf
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Attack_FireBullet = {
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_HowitzerHal_Shot)
    Wait(13)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 10, Float(1.0))
    EndThread
    Call(StartRumble, BTL_RUMBLE_PLAYER_HEAVY)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_BULLET_BILL_FIRE)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Sub(LVar0, 33)
    Add(LVar1, 19)
    Add(LVar2, 3)
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 0, 2, 0)
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 2, 2, 0)
    Wait(2)
    Call(SetAnimation, ACTOR_SELF, PRT_BULLET, ANIM_HowitzerHal_BulletBill)
    Wait(4)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
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
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BULLET, ACTOR_PART_FLAG_INVISIBLE, false)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BULLET, Float(13.0))
            Call(SetGoalToTarget, ACTOR_SELF)
            Call(FlyPartTo, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2, 0, 0, EASING_CUBIC_IN)
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
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BULLET, ACTOR_PART_FLAG_INVISIBLE, false)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BULLET, Float(13.0))
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(FlyPartTo, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2, 0, 0, EASING_CUBIC_IN)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgImpact, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BULLET, ACTOR_PART_FLAG_INVISIBLE, true)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartPos, ACTOR_SELF, PRT_BULLET, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

}; // namespace howitzer_hal

ActorBlueprint HowitzerHal = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN,
    .maxHP = howitzer_hal::hp,
    .type = ACTOR_TYPE_BILL_BLASTER,
    .level = 0,
    .partCount = ARRAY_COUNT(howitzer_hal::ActorParts),
    .partsData = howitzer_hal::ActorParts,
    .initScript = &howitzer_hal::EVS_Init,
    .statusTable = howitzer_hal::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 20,
    .hurricaneChance = 15,
    .spookChance = 0,
    .upAndAwayChance = 95,
    .spinSmashReq = 4,
    .powerBounceChance = 100,
    .coinReward = 1,
    .size = { 55, 32 },
    .healthBarOffset = { 8, 0 },
    .statusIconOffset = { -24, 20 },
    .statusTextOffset = { 5, 25 },
};

}; // namespace battle::actor
