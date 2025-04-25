#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "sprite/npc/PyroGuy.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"

namespace battle::actor {

namespace pyro_guy {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;

enum N(ActorPartIDs) {
    PRT_MAIN        = 1,
};

// Actor Stats
constexpr s32 hp = 10;
constexpr s32 dmgTackle = 4;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_PyroGuy_Anim08,
    STATUS_KEY_STONE,     ANIM_PyroGuy_Anim00,
    STATUS_KEY_SLEEP,     ANIM_PyroGuy_Anim07,
    STATUS_KEY_POISON,    ANIM_PyroGuy_Anim01,
    STATUS_KEY_STOP,      ANIM_PyroGuy_Anim00,
    STATUS_KEY_STATIC,    ANIM_PyroGuy_Anim01,
    STATUS_KEY_PARALYZE,  ANIM_PyroGuy_Anim00,
    STATUS_KEY_DIZZY,     ANIM_PyroGuy_Anim08,
    STATUS_KEY_FEAR,      ANIM_PyroGuy_Anim08,
    STATUS_END,
};

s32 DefenseTable[] = {
    ELEMENT_NORMAL,   0,
    ELEMENT_WATER,   -2,
    ELEMENT_ICE,     -2,
    ELEMENT_FIRE,    99,
    ELEMENT_SHOCK,    0,
    ELEMENT_SMASH,    0,
    ELEMENT_BLAST,   -1,
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
        .targetOffset = { 0, 30 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAG_FIREY,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
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
    Call(SetActorYaw, ACTOR_SELF, 180)
    Return
    End
};

#include "common/battle/SetAbsoluteStatusOffsets.inc.c"

EvtScript EVS_Idle = {
    Return
    End
};

EvtScript EVS_ReturnHome = {
    Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_WALK, SOUND_ACTOR_STEP_A, SOUND_ACTOR_STEP_B)
    Call(SetActorSounds, ACTOR_SELF, ACTOR_SOUND_WALK_INCREMENT, 10, 0)
    SetConst(LVar0, PRT_MAIN)
    SetConst(LVar1, ANIM_PyroGuy_Anim04)
    SetConst(LVar2, ANIM_PyroGuy_Anim01)
    ExecWait(EVS_Enemy_ReturnHome)
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
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            SetConst(LVar2, -1)
            ExecWait(EVS_Enemy_BurnHit)
        CaseEq(EVENT_BURN_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_PyroDefeated, TRUE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            SetConst(LVar2, -1)
            ExecWait(EVS_Enemy_BurnHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_PyroDefeated, TRUE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SHOCK_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Knockback)
            Call(SetActorSpeed, ACTOR_SELF, Float(4.0))
            ExecWait(EVS_ReturnHome)
        CaseEq(EVENT_SHOCK_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_PyroDefeated, TRUE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
        CaseOrEq(EVENT_AIR_LIFT_FAILED)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim01)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            Call(SetActorVar, ACTOR_RED_BANDIT, AVAR_RedPhase_PyroDefeated, TRUE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim06)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim01)
            ExecWait(EVS_Enemy_Recover)
        CaseEq(EVENT_SCARE_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim03)
            SetConst(LVar2, ANIM_PyroGuy_Anim09)
            ExecWait(EVS_Enemy_ScareAway)
            Return
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim03)
            ExecWait(EVS_Enemy_AirLift)
        CaseEq(EVENT_BLOW_AWAY)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_PyroGuy_Anim03)
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_UP_AND_AWAY)
            Return
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 10)
    Add(LVar1, 30)
    Call(SetBattleCamTarget, LVar0, LVar1, LVar2)
    Call(SetBattleCamDist, 250)
    Call(SetBattleCamOffsetY, 0)
    Call(MoveBattleCamOver, 30)
    Wait(30)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar0, 20)
    Add(LVar1, 15)
    PlayEffect(EFFECT_RECOVER, 2, LVar0, LVar1, LVar2, 0, 0)
    Wait(30)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_PyroGuy_Anim03)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace pyro_guy

ActorBlueprint PyroGuy = {
    .flags = ACTOR_FLAG_NO_SHADOW,
    .maxHP = pyro_guy::hp,
    .type = ACTOR_TYPE_PYRO_GUY_RED_PHASE,
    .level = 0,
    .partCount = ARRAY_COUNT(pyro_guy::ActorParts),
    .partsData = pyro_guy::ActorParts,
    .initScript = &pyro_guy::EVS_Init,
    .statusTable = pyro_guy::StatusTable,
    .escapeChance = 60,
    .airLiftChance = 85,
    .hurricaneChance = 80,
    .spookChance = 60,
    .upAndAwayChance = 95,
    .spinSmashReq = 0,
    .powerBounceChance = 85,
    .coinReward = 1,
    .size = { 28, 36 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
