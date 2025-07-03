#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/Crate.h"
#include "sprite/npc/ShyGuyRider.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"

namespace battle::actor {

namespace dyanmite_crate {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_CrateExplode;

enum ActorPartIDs {
    PRT_MAIN        = 1,
};

// Actor Stats
constexpr s32 hp = 1;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_Crate_IdleClosed,
    STATUS_END,
};

s32 OpenAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_Crate_IdleOpenDyanmite,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   99,
    ELEMENT_JUMP,     99,
    ELEMENT_SMASH,    99,
    ELEMENT_BLAST,    99,
    ELEMENT_END,
};

s32 OpenDefense[] = {
    ELEMENT_NORMAL,   99,
    ELEMENT_JUMP,     99,
    ELEMENT_SMASH,    99,
    ELEMENT_BLAST,    -1,
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
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 36 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
    },
};

EvtScript EVS_Init = {
    Call(SetActorScale, ACTOR_SELF, Float(1.5), Float(1.5), Float(1.0))
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

#include "common/battle/SetAbsoluteStatusOffsets.inc.c"

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
            Wait(25)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Crate_IdleOpenDyanmite)
            ExecWait(EVS_Enemy_Hit)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(OpenAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(OpenDefense))
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Crate_IdleOpenDyanmite)
            SetConst(LVar2, -1)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Crate_IdleOpenDyanmite)
            SetConst(LVar2, -1)
            ExecWait(EVS_Enemy_BurnHit)
            ExecWait(EVS_CrateExplode)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
            Wait(25)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Crate_DynamiteJumpedOn)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(OpenAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(OpenDefense))
        CaseOrEq(EVENT_IMMUNE)
            Wait(25)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Crate_DynamiteJumpedOn)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(OpenAnims))
            Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(OpenDefense))
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_Crate_IdleOpenDyanmite)
            ExecWait(EVS_Enemy_Hit)
            ExecWait(EVS_CrateExplode)
            Return
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_CrateExplode = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar2, 2)
    PlayEffect(EFFECT_SMOKE_RING, 0, LVar0, LVar1, LVar2, 0)
    Add(LVar1, 16)
    Add(LVar2, 2)
    PlayEffect(EFFECT_BLAST, 0, LVar0, LVar1, LVar2, Float(3.0), 30, 0)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_BULLET_BILL_EXPLODE_A)
    Call(StartRumble, BTL_RUMBLE_PLAYER_MAX)
    Thread
        Call(ShakeCam, CAM_BATTLE, 0, 5, Float(1.0))
    EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_Crate_BreakDynamite)
    Call(ActorExists, ACTOR_SHY_GUY_RIDER_1, LVar4)
    IfEq(LVar4, TRUE)
        Call(UseIdleAnimation, ACTOR_SHY_GUY_RIDER_1, FALSE)
        Call(UseIdleAnimation, ACTOR_SHY_GUY_RIDER_2, FALSE)
        Call(SetAnimation, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ANIM_ShyGuyRider_Panic)
        Wait(15)
        Call(SetAnimation, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ANIM_ShyGuyRider_Panic)
        Wait(15)
        Call(SetAnimation, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ANIM_ShyGuyRider_RunAway)
        Call(GetActorPos, ACTOR_SHY_GUY_RIDER_1, LVar0, LVar1, LVar2)
        Add(LVar0, 400)
        Call(SetGoalPos, ACTOR_SHY_GUY_RIDER_1, LVar0, LVar1, LVar2)
        Call(SetActorYaw, ACTOR_SHY_GUY_RIDER_1, 180)
        Call(EnableActorBlur, ACTOR_SHY_GUY_RIDER_1, ACTOR_BLUR_ENABLE)
        Call(SetActorSpeed, ACTOR_SHY_GUY_RIDER_1, Float(15.0))
        Call(RunToGoal, ACTOR_SHY_GUY_RIDER_1, 0, FALSE)
        Call(SetAnimation, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ANIM_ShyGuyRider_RunAway)
        Call(GetActorPos, ACTOR_SHY_GUY_RIDER_2, LVar0, LVar1, LVar2)
        Add(LVar0, 400)
        Call(SetGoalPos, ACTOR_SHY_GUY_RIDER_2, LVar0, LVar1, LVar2)
        Call(SetActorYaw, ACTOR_SHY_GUY_RIDER_2, 180)
        Call(EnableActorBlur, ACTOR_SHY_GUY_RIDER_2, ACTOR_BLUR_ENABLE)
        Call(SetActorSpeed, ACTOR_SHY_GUY_RIDER_2, Float(15.0))
        Call(RunToGoal, ACTOR_SHY_GUY_RIDER_2, 0, FALSE)
        Wait(10)
        Call(RemoveActor, ACTOR_SHY_GUY_RIDER_1)
        Wait(5)
        Call(RemoveActor, ACTOR_SHY_GUY_RIDER_2)
        Wait(5)
        Call(SetPartFlagBits, ACTOR_BLACK_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET, FALSE)
    EndIf
    Wait(30)
    // Call(RemoveActor, ACTOR_SELF)
    SetConst(LVar0, PRT_MAIN)
    SetConst(LVar1, -1)
    ExecWait(EVS_Enemy_Death)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace dyanmite_crate

ActorBlueprint DyanmiteCrate = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR,
    .maxHP = dyanmite_crate::hp,
    .type = ACTOR_TYPE_DYANMITE_CRATE,
    .level = ACTOR_LEVEL_DYANMITE_CRATE,
    .partCount = ARRAY_COUNT(dyanmite_crate::ActorParts),
    .partsData = dyanmite_crate::ActorParts,
    .initScript = &dyanmite_crate::EVS_Init,
    .statusTable = dyanmite_crate::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 0,
    .powerBounceChance = 100,
    .coinReward = 1,
    .size = { 42, 54 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
