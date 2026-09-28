#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "boss.hpp"
#include "sprite/npc/ShyGuy.h"
#include "dx/debug_menu.h"

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Dummy;

enum ActorPartIDs {
    PRT_MAIN        = 1,
};

// Actor Stats
constexpr s32 hp = 50;

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
    STATUS_KEY_UNUSED,              0,
    STATUS_KEY_STATIC,              0,
    STATUS_KEY_PARALYZE,            0,
    STATUS_KEY_SHRINK,              0,
    STATUS_KEY_STOP,                0,
    STATUS_TURN_MOD_DEFAULT,        0,
    STATUS_TURN_MOD_SLEEP,          0,
    STATUS_TURN_MOD_POISON,         0,
    STATUS_TURN_MOD_FROZEN,         0,
    STATUS_TURN_MOD_DIZZY,          0,
    STATUS_TURN_MOD_UNUSED,         0,
    STATUS_TURN_MOD_STATIC,         0,
    STATUS_TURN_MOD_PARALYZE,       0,
    STATUS_TURN_MOD_SHRINK,         0,
    STATUS_TURN_MOD_STOP,           0,
    STATUS_END,
};

ActorPartBlueprint ActorParts[] = {
    {
        .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 24 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -10 },
    },
};

extern "C" export ActorBlueprint blueprint = {
    .flags = ACTOR_FLAG_NO_HEALTH_BAR,
    .maxHP = hp,
    .type = ACTOR_TYPE_STAGE_GUY,
    .level = ACTOR_LEVEL_STAGE_GUY,
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
    .powerBounceChance = 75,
    .coinReward = 0,
    .size = { 30, 30 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_ShyGuy_Black_Idle,
    STATUS_END,
};

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_Dummy))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Dummy))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_Dummy))
    Return
    End
};

EvtScript EVS_Dummy = {
    Return
    End
};
