#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "mapfs/trn_bt00_shape.h"
#include "sprite/npc/KoopaGang.h"
#include "sprite/npc/BuzzyBeetle.h"
#include "sprite/npc/BrigaderBones.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace green_bandit_koopa {

// Green Bandit Params
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
constexpr s32 hp = 1;
constexpr s32 dmgSlowImpact = 1;
constexpr s32 dmgFastImpact = 2;

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
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(HPBarToHome, ACTOR_SELF)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_State, AVAL_Koopa_State_Ready)
    // Call(SetActorVar, ACTOR_SELF, AVAR_Koopa_ToppleTurns, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_ActorsSpawned, FALSE)
    Exec(EVS_ManageFirstPhase)
    Return
    End
};

EvtScript EVS_ManageFirstPhase = {
    Call(GetActorVar, ACTOR_SELF, AVAR_Phase, LVar0)
    Set(LVar0, AVAL_GreenPhase)
    Switch(LVar0)
        CaseEq(AVAL_IntroPhase)
        CaseEq(AVAL_GreenPhase)
            //* Models
            Call(EnableModel, MODEL_BombBox, TRUE)

            // // YELLOWANCHOR  Yellow Phase Actors
            // Call(SetActorFlagBits, ACTOR_YELLOW_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_INVISIBLE,  TRUE)
            // Call(SetPartFlagBits, ACTOR_YELLOW_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_GIANT_CHOMP, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, 2, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_YELLOW_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_YELLOW_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // // BLACKANCHOR Black Phase Actors
            // Call(SetActorFlagBits, ACTOR_BLACK_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_BLACK_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_CRATE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_DYANMITE_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_DYANMITE_CRATE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // // REDANCHOR Red Phase Actors
            // Call(SetActorFlagBits, ACTOR_RED_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_RED_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // Call(SetActorFlagBits, ACTOR_PYRO_GUY, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_INVISIBLE, TRUE)
            // Call(SetPartFlagBits, ACTOR_PYRO_GUY, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // // ORANGEANCHOR Bowser Phase Actors
            // Call(SetActorPos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_BOWSER_THE_KID, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_BOWSER_THE_KID, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_BOWSER_THE_KID)
            // Call(SetActorPos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_KOOPA_GANG, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            // Call(SetPartFlagBits, ACTOR_KOOPA_GANG, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_KOOPA_GANG)
            // Call(SetActorPos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_GREEN_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_GREEN_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
        CaseEq(AVAL_YellowPhase)
            //* Models
            Call(EnableModel, MODEL_SnipingCrate, TRUE)
            Call(EnableModel, MODEL_BarrelBlack, TRUE)

            // GREENANCHOR Green Phase Actors
            Call(SetActorPos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GREEN_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GREEN_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_GREEN_BANDIT)

            Call(SetActorPos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BUZZY_BEETLE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BUZZY_BEETLE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_BUZZY_BEETLE)

            Call(SetActorPos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BRIGADER_BONES, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartPos, ACTOR_BRIGADER_BONES, 2, NPC_DISPOSE_LOCATION)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, 2, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_BRIGADER_BONES)

            // YELLOWANCHOR Yellow Phase Actors
            Call(SetActorPos, ACTOR_YELLOW_BANDIT, 105, 0, 20)
            Call(ForceHomePos, ACTOR_YELLOW_BANDIT, 105, 0, 20)
            Call(SetActorFlagBits, ACTOR_YELLOW_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_YELLOW_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_YELLOW_BANDIT)

            Call(SetActorPos, ACTOR_GIANT_CHOMP, 25, 0, 20)
            Call(ForceHomePos, ACTOR_GIANT_CHOMP, 25, 0, 20)
            Call(SetActorFlagBits, ACTOR_GIANT_CHOMP, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, FALSE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, 2, ACTOR_PART_FLAG_NO_TARGET, FALSE)
            // Call(RemoveActor, ACTOR_GIANT_CHOMP)

            Call(SetActorPos, ACTOR_YELLOW_HAMMER_BRO, 145, 0, 20)
            Call(ForceHomePos, ACTOR_YELLOW_HAMMER_BRO, 145, 0, 20)
            Call(SetActorFlagBits, ACTOR_YELLOW_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_YELLOW_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_YELLOW_HAMMER_BRO)

            // BLACKANCHOR Black Phase Actors
            Call(SetActorPos, ACTOR_BLACK_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BLACK_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BLACK_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BLACK_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_BLACK_BANDIT)

            Call(SetActorPos, ACTOR_CRATE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_CRATE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_CRATE, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_CRATE)

            Call(SetActorPos, ACTOR_DYANMITE_CRATE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_DYANMITE_CRATE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_DYANMITE_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_DYANMITE_CRATE, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_DYANMITE_CRATE)

            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_1, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_1,  NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_SHY_GUY_RIDER_1)

            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_2, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_2, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_SHY_GUY_RIDER_2)

            // REDANCHOR Red Phase Actors
            Call(SetActorPos, ACTOR_RED_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_RED_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_RED_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_RED_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_RED_BANDIT)

            Call(SetActorPos, ACTOR_PYRO_GUY, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_PYRO_GUY, NPC_DISPOSE_LOCATION)
            Call(SetActorYaw, ACTOR_PYRO_GUY, 180)
            Call(SetActorFlagBits, ACTOR_PYRO_GUY, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_PYRO_GUY, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
            // Call(RemoveActor, ACTOR_PYRO_GUY)

            // ORANGEANCHOR Bowser Phase Actors
            // Call(SetActorPos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_BOWSER_THE_KID, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_BOWSER_THE_KID, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_BOWSER_THE_KID)
            // Call(SetActorPos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_KOOPA_GANG, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            // Call(SetPartFlagBits, ACTOR_KOOPA_GANG, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_KOOPA_GANG)
            // Call(SetActorPos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_GREEN_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_GREEN_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
        CaseEq(AVAL_BlackPhase)
            // Models
            Call(EnableModel, MODEL_SnipingCrate, TRUE)
            Call(EnableModel, MODEL_BarrelBlack, TRUE)

            // GREENANCHOR Green Phase Actors
            Call(SetActorPos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GREEN_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GREEN_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_GREEN_BANDIT)

            Call(SetActorPos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BUZZY_BEETLE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BUZZY_BEETLE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_BUZZY_BEETLE)

            Call(SetActorPos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BRIGADER_BONES, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartPos, ACTOR_BRIGADER_BONES, 2, NPC_DISPOSE_LOCATION)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, 2, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_BRIGADER_BONES)

            // YELLOWANCHOR Yellow Phase Actors
            Call(SetActorPos, ACTOR_YELLOW_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_YELLOW_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_YELLOW_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_YELLOW_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_YELLOW_BANDIT)

            Call(SetActorPos, ACTOR_GIANT_CHOMP, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GIANT_CHOMP, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GIANT_CHOMP, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, 2, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_GIANT_CHOMP)

            Call(SetActorPos, ACTOR_YELLOW_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_YELLOW_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_YELLOW_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_YELLOW_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_YELLOW_HAMMER_BRO)

            // BLACKANCHOR Black Phase Actors
            Call(SetActorPos, ACTOR_BLACK_BANDIT, 115, 10, 20)
            Call(ForceHomePos, ACTOR_BLACK_BANDIT, 115, 10, 20)
            Call(SetActorFlagBits, ACTOR_BLACK_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_BLACK_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_BLACK_BANDIT)

            Call(SetActorPos, ACTOR_CRATE, 15, 0, 20)
            Call(ForceHomePos, ACTOR_CRATE, 15, 0, 20)
            Call(SetActorFlagBits, ACTOR_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, FALSE)
            Call(SetPartFlagBits, ACTOR_CRATE, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_CRATE)

            Call(SetActorPos, ACTOR_DYANMITE_CRATE, 55, 0, 20)
            Call(ForceHomePos, ACTOR_DYANMITE_CRATE, 55, 0, 20)
            Call(SetActorFlagBits, ACTOR_DYANMITE_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, FALSE)
            Call(SetPartFlagBits, ACTOR_DYANMITE_CRATE, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_DYANMITE_CRATE)

            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_1, 45, -25, -50)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_1, 45, -25, -50)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_SHY_GUY_RIDER_1)

            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_2, -25, -25, -50)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_2, -25, -25, -50)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_SHY_GUY_RIDER_2)

            // REDANCHOR Red Phase Actors
            Call(SetActorPos, ACTOR_RED_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_RED_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_RED_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_RED_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // Call(RemoveActor, ACTOR_RED_BANDIT)

            Call(SetActorPos, ACTOR_PYRO_GUY, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_PYRO_GUY, NPC_DISPOSE_LOCATION)
            Call(SetActorYaw, ACTOR_PYRO_GUY, 180)
            Call(SetActorFlagBits, ACTOR_PYRO_GUY, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_PYRO_GUY, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
            // Call(RemoveActor, ACTOR_PYRO_GUY)

            // ORANGEANCHOR Bowser Phase Actors
            // Call(SetActorPos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_BOWSER_THE_KID, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_BOWSER_THE_KID, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_BOWSER_THE_KID)
            // Call(SetActorPos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_KOOPA_GANG, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            // Call(SetPartFlagBits, ACTOR_KOOPA_GANG, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_KOOPA_GANG)
            // Call(SetActorPos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_GREEN_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_GREEN_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
        CaseEq(AVAL_RedPhase)
            //* Models
            Call(EnableModel, MODEL_BombBox, TRUE)
            Call(EnableModel, MODEL_BombPile, TRUE)
            Call(EnableModel, MODEL_BarrelRed, TRUE)

            // GREENANCHOR Green Phase Actors
            Call(SetActorPos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GREEN_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GREEN_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BUZZY_BEETLE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BUZZY_BEETLE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BRIGADER_BONES, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartPos, ACTOR_BRIGADER_BONES, 2, NPC_DISPOSE_LOCATION)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, 2, ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // YELLOWANCHOR Yellow Phase Actors
            Call(SetActorPos, ACTOR_YELLOW_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_YELLOW_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_YELLOW_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_YELLOW_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_GIANT_CHOMP, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GIANT_CHOMP, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GIANT_CHOMP, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, 2, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_YELLOW_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_YELLOW_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_YELLOW_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_YELLOW_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // BLACKANCHOR Black Phase Actors
            Call(SetActorPos, ACTOR_BLACK_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BLACK_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BLACK_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BLACK_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_CRATE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_CRATE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_CRATE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_DYANMITE_CRATE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_DYANMITE_CRATE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_DYANMITE_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_DYANMITE_CRATE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_1, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_1, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)

            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_2, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_2, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)

            // REDANCHOR Red Phase Actors
            Call(SetActorPos, ACTOR_RED_BANDIT, 115, 25, 20)
            Call(ForceHomePos, ACTOR_RED_BANDIT, 115, 25, 20)
            Call(SetActorFlagBits, ACTOR_RED_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_RED_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, FALSE)

            Call(SetActorPos, ACTOR_PYRO_GUY, 145, 55, 25)
            Call(ForceHomePos, ACTOR_PYRO_GUY, 145, 55, 25)
            Call(SetActorYaw, ACTOR_PYRO_GUY, 180)
            Call(SetActorFlagBits, ACTOR_PYRO_GUY, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_PYRO_GUY, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)

            // ORANGEANCHOR Bowser Phase Actors
            // Call(SetActorPos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_BOWSER_THE_KID, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_BOWSER_THE_KID, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_BOWSER_THE_KID, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_BOWSER_THE_KID)
            // Call(SetActorPos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_KOOPA_GANG, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_KOOPA_GANG, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            // Call(SetPartFlagBits, ACTOR_KOOPA_GANG, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            // // Call(RemoveActor, ACTOR_KOOPA_GANG)
            // Call(SetActorPos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(ForceHomePos, ACTOR_GREEN_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            // Call(SetActorFlagBits, ACTOR_GREEN_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            // Call(SetPartFlagBits, ACTOR_GREEN_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
        CaseEq(AVAL_BowserPhase)
            //* Models
            // Call(EnableModel, MODEL_BombBox, TRUE)
            // Call(EnableModel, MODEL_BombPile, TRUE)
            // Call(EnableModel, MODEL_BarrelRed, TRUE)

            // GREENANCHOR Green Phase Actors
            Call(SetActorPos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GREEN_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GREEN_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GREEN_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_GREEN_BANDIT)
            Call(SetActorPos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BUZZY_BEETLE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BUZZY_BEETLE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BUZZY_BEETLE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_BUZZY_BEETLE)
            Call(SetActorPos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BRIGADER_BONES, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BRIGADER_BONES, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartPos, ACTOR_BRIGADER_BONES, 2, NPC_DISPOSE_LOCATION)
            Call(SetPartFlagBits, ACTOR_BRIGADER_BONES, 2, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_BRIGADER_BONES)

            // YELLOWANCHOR Yellow Phase Actors
            Call(SetActorPos, ACTOR_YELLOW_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_YELLOW_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_YELLOW_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_YELLOW_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_YELLOW_BANDIT)
            Call(SetActorPos, ACTOR_GIANT_CHOMP, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_GIANT_CHOMP, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_GIANT_CHOMP, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(SetPartFlagBits, ACTOR_GIANT_CHOMP, 2, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_GIANT_CHOMP)
            Call(SetActorPos, ACTOR_YELLOW_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_YELLOW_HAMMER_BRO, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_YELLOW_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_YELLOW_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_YELLOW_HAMMER_BRO)

            // BLACKANCHOR Black Phase Actors
            Call(SetActorPos, ACTOR_BLACK_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_BLACK_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_BLACK_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_BLACK_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_BLACK_BANDIT)
            Call(SetActorPos, ACTOR_CRATE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_CRATE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_CRATE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_CRATE)
            Call(SetActorPos, ACTOR_DYANMITE_CRATE, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_DYANMITE_CRATE, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_DYANMITE_CRATE, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_DYANMITE_CRATE, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_DYANMITE_CRATE)
            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_1, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_1, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_1, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_1, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_SHY_GUY_RIDER_1)
            Call(SetActorPos, ACTOR_SHY_GUY_RIDER_2, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_SHY_GUY_RIDER_2, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_SHY_GUY_RIDER_2, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
            Call(SetPartFlagBits, ACTOR_SHY_GUY_RIDER_2, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_SHY_GUY_RIDER_2)

            // REDANCHOR Red Phase Actors
            Call(SetActorPos, ACTOR_RED_BANDIT, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_RED_BANDIT, NPC_DISPOSE_LOCATION)
            Call(SetActorFlagBits, ACTOR_RED_BANDIT, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
            Call(SetPartFlagBits, ACTOR_RED_BANDIT, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(RemoveActor, ACTOR_RED_BANDIT)
            Call(SetActorPos, ACTOR_PYRO_GUY, NPC_DISPOSE_LOCATION)
            Call(ForceHomePos, ACTOR_PYRO_GUY, NPC_DISPOSE_LOCATION)
            Call(SetActorYaw, ACTOR_PYRO_GUY, 180)
            Call(SetActorFlagBits, ACTOR_PYRO_GUY, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_PYRO_GUY, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            Call(RemoveActor, ACTOR_PYRO_GUY)

            // ORANGEANCHOR Bowser Phase Actors
            Call(SetActorPos, ACTOR_BOWSER_THE_KID, 100, 35, 20)
            Call(ForceHomePos, ACTOR_BOWSER_THE_KID, 100, 35, 20)
            Call(SetActorFlagBits, ACTOR_BOWSER_THE_KID, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_BOWSER_THE_KID, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_BOWSER_THE_KID)
            Call(SetActorPos, ACTOR_KOOPA_GANG, 60, 0, 20)
            Call(ForceHomePos, ACTOR_KOOPA_GANG, 60, 0, 20)
            Call(SetActorFlagBits, ACTOR_KOOPA_GANG, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, FALSE)
            Call(SetPartFlagBits, ACTOR_KOOPA_GANG, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(RemoveActor, ACTOR_KOOPA_GANG)
            Call(SetActorPos, ACTOR_GREEN_HAMMER_BRO, 20, 0, 20)
            Call(ForceHomePos, ACTOR_GREEN_HAMMER_BRO, 20, 0, 20)
            Call(SetActorFlagBits, ACTOR_GREEN_HAMMER_BRO, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_HEALTH_BAR, FALSE)
            Call(SetPartFlagBits, ACTOR_GREEN_HAMMER_BRO, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_INVISIBLE, FALSE)
    EndSwitch
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
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(EnableBattleStatusBar, FALSE)
    Call(ActorExists, ACTOR_BUZZY_BEETLE, LVar2)
    IfNe(LVar2, FALSE)
        Call(GetActorHP, ACTOR_BUZZY_BEETLE, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_BUZZY_BEETLE)
                Call(EnableIdleScript, ACTOR_BUZZY_BEETLE, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_BUZZY_BEETLE, FALSE)
                Call(SetAnimation, ACTOR_BUZZY_BEETLE, PRT_MAIN, ANIM_BuzzyBeetle_Anim07)
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
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
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
    IfNe(LVar2, FALSE)
        Call(GetActorHP, ACTOR_BRIGADER_BONES, LVar2)
        IfNe(LVar2, 0)
            Thread
                Call(HideHealthBar, ACTOR_BRIGADER_BONES)
                Call(EnableIdleScript, ACTOR_BRIGADER_BONES, IDLE_SCRIPT_DISABLE)
                Call(UseIdleAnimation, ACTOR_BRIGADER_BONES, FALSE)
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
                PlayEffect(EFFECT_BIG_SMOKE_PUFF, LVar0, LVar1, LVar2, 0, 0, 0, 0, 0)
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
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_HEALTH_BAR, TRUE)
    Wait(10)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 60)
    Add(LVar2, 2)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(25)
    Label(0)
        Call(ActorExists, ACTOR_BUZZY_BEETLE, LVar0)
        IfNe(LVar0, FALSE)
            Wait(1)
            Goto(0)
        EndIf
        Call(ActorExists, ACTOR_BRIGADER_BONES, LVar0)
        IfNe(LVar0, FALSE)
            Wait(1)
            Goto(0)
        EndIf
    ExecWait(EVS_SecondPhaseTransition)
    // Call(SetBattleFlagBits, BS_FLAGS1_DISABLE_CELEBRATION | BS_FLAGS1_BATTLE_FLED, TRUE)
    // Call(SetBattleFlagBits2, BS_FLAGS2_DONT_STOP_MUSIC, TRUE)
    // Call(SetEndBattleFadeOutRate, 20)
    Return
    End
};

Vec3i YellowBanditSpawnPos = { 105, 0, 10 };

Formation SpawnYellowBandit = {
    ACTOR_BY_POS(YellowBanditKoopa, YellowBanditSpawnPos, 50),
};

Vec3i GiantChainChompSpawnPos = { 25, 0, 11 };

Formation SpawnGiantChainChomp = {
    ACTOR_BY_POS(GiantChainChomp, GiantChainChompSpawnPos, 100),
};

Vec3i HammerBroAltSpawnPos = { 145, 0, 10 };

Formation SpawnYellowHammerBro = {
    ACTOR_BY_POS(YellowHammerBro, HammerBroAltSpawnPos, 75),
};

EvtScript EVS_SecondPhaseTransition = {
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
            Call(GetActorVar, ACTOR_SELF, AVAR_GreenPhase_ActorsSpawned, LVarB)
            IfEq(LVarB, FALSE)
                Call(EnableModel, MODEL_BombBox, FALSE)
                Call(SummonEnemy, Ref(SpawnYellowBandit), FALSE)
                Call(SummonEnemy, Ref(SpawnGiantChainChomp), FALSE)
                Call(SummonEnemy, Ref(SpawnYellowHammerBro), FALSE)
                Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_INVISIBLE, TRUE)
                Call(SetActorVar, ACTOR_SELF, AVAR_GreenPhase_ActorsSpawned, TRUE)
                Call(SetActorVar, ACTOR_SELF, AVAR_Phase, AVAL_YellowPhase)
            EndIf
        EndIf
        Call(TranslateModel, MODEL_Tunnel, LVarA, 0, 0)
        Wait(1)
    EndLoop
    Call(TranslateModel, MODEL_Tunnel, LVarA, 0, 0)
    DebugPrintf("Transition Done!")
    Call(EnableModel, MODEL_Tunnel, FALSE)
    Call(EnableBattleStatusBar, TRUE)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
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
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
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
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 0, 2, 0)
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 2, 2, 0)
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
            Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, 0)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(7.5))
            Call(SetGoalToTarget, ACTOR_SELF)
            // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
            Thread
                Call(MakeLerp, 0, Float(47.5), 20, EASING_QUADRATIC_OUT)
                Label(0)
                Call(UpdateLerp)
                Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, LVar0)
                Wait(1)
                IfEq(LVar1, 1)
                    Goto(0)
                EndIf
            EndThread
            Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN)
            // Wait(5)
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
    // Wait(5)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 15)
    // Add(LVar1, 48)
    // Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
    Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, 0)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Add(LVar1, 25)
    Add(LVar2, 2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(7.5))
    Call(SetGoalToTarget, ACTOR_SELF)
    // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
    Thread
        Call(MakeLerp, 0, Float(47.5), 20, EASING_QUADRATIC_OUT)
        Label(0)
        Call(UpdateLerp)
        Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, LVar0)
        Wait(1)
        IfEq(LVar1, 1)
            Goto(0)
        EndIf
    EndThread
    Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN)
    // Wait(5)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgSlowImpact, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, TRUE)
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
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 0, 2, 0)
    PlayEffect(EFFECT_00, LVar0, LVar1, LVar2, 2, 5, 2, 2, 0)
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
            Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, 0)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, FALSE)
            // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
            Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(15.0))
            Call(SetGoalToTarget, ACTOR_SELF)
            // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
            Thread
                Call(MakeLerp, 0, Float(47.5), 20, EASING_QUADRATIC_OUT)
                Label(0)
                Call(UpdateLerp)
                Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, LVar0)
                Wait(1)
                IfEq(LVar1, 1)
                    Goto(0)
                EndIf
            EndThread
            Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
            // Wait(5)
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
    // Wait(5)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 15)
    // Add(LVar1, 48)
    // Call(SetPartPos, ACTOR_SELF, PRT_ARROW, LVar0, LVar1, LVar2)
    Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, 0)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, FALSE)
    // Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
    Add(LVar1, 25)
    Add(LVar2, 2)
    Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(SetPartMoveSpeed, ACTOR_SELF, PRT_BIFF, Float(15.0))
    Call(SetGoalToTarget, ACTOR_SELF)
    // Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 10, EASING_CUBIC_IN)
    Thread
        Call(MakeLerp, 0, Float(47.5), 20, EASING_QUADRATIC_OUT)
        Label(0)
        Call(UpdateLerp)
        Call(SetPartRotation, ACTOR_SELF, PRT_BIFF, 0, 0, LVar0)
        Wait(1)
        IfEq(LVar1, 1)
            Goto(0)
        EndIf
    EndThread
    Call(FlyPartTo, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2, 0, 0, EASING_COS_IN_OUT)
    // Wait(5)
    Wait(2)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_NO_CONTACT, 0, 0, dmgFastImpact, BS_FLAGS1_TRIGGER_EVENTS)
    Switch(LVar0)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Call(SetPartFlagBits, ACTOR_SELF, PRT_BIFF, ACTOR_PART_FLAG_INVISIBLE, TRUE)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(SetPartPos, ACTOR_SELF, PRT_BIFF, LVar0, LVar1, LVar2)
            Call(YieldTurn)
        EndCaseGroup
    EndSwitch
    Return
    End
};


EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace green_bandit_koopa

namespace buzzy_beetle {

extern s32 BuzzyShellAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandleEvent;

enum ActorPartIDs {
    PRT_BUZZY            = 1,
};

enum ActorVars {
    AVAR_ToppleState        = 3,
    AVAL_State_Ground       = 1,
    AVAL_State_Toppled      = 2,
    AVAR_ToppleTurns        = 4,
    AVAL_ToppleTurnZero     = 0,
    AVAL_ToppleTurnOne      = 1,
};

// Actor Stats
constexpr s32 hp = 5;
constexpr s32 dmgShellToss = 2;

s32 UprightDefense[] = {
    ELEMENT_NORMAL,   4,
    ELEMENT_FIRE,    99,
    ELEMENT_BLAST,   99,
    ELEMENT_END,
};

s32 ToppledDefense[] = {
    ELEMENT_NORMAL,   2,
    ELEMENT_END,
};

s32 StatusTable[] = {
    STATUS_KEY_NORMAL,              0,
    STATUS_KEY_DEFAULT,             0,
    STATUS_KEY_SLEEP,              90,
    STATUS_KEY_POISON,             50,
    STATUS_KEY_FROZEN,              0,
    STATUS_KEY_DIZZY,              75,
    STATUS_KEY_FEAR,                0,
    STATUS_KEY_STATIC,             50,
    STATUS_KEY_PARALYZE,           75,
    STATUS_KEY_SHRINK,             90,
    STATUS_KEY_STOP,               90,
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
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET | ACTOR_PART_FLAG_IGNORE_BELOW_CHECK,
        .index = PRT_BUZZY,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { 0, 16 },
        .opacity = 255,
        .idleAnimations = BuzzyShellAnims,
        .defenseTable = UprightDefense,
        .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { -1, -9 },
    },
};

s32 BuzzyShellAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Anim05,
    STATUS_END,
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Anim01,
    STATUS_KEY_STONE,     ANIM_BuzzyBeetle_Anim00,
    STATUS_KEY_SLEEP,     ANIM_BuzzyBeetle_Anim09,
    STATUS_KEY_POISON,    ANIM_BuzzyBeetle_Anim01,
    STATUS_KEY_STOP,      ANIM_BuzzyBeetle_Anim00,
    STATUS_KEY_STATIC,    ANIM_BuzzyBeetle_Anim01,
    STATUS_KEY_PARALYZE,  ANIM_BuzzyBeetle_Anim00,
    STATUS_KEY_DIZZY,     ANIM_BuzzyBeetle_Anim0A,
    STATUS_KEY_FEAR,      ANIM_BuzzyBeetle_Anim0A,
    STATUS_END,
};

s32 ToppledAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Anim02,
    STATUS_KEY_STONE,     ANIM_BuzzyBeetle_Anim1A,
    STATUS_KEY_SLEEP,     ANIM_BuzzyBeetle_Anim1B,
    STATUS_KEY_POISON,    ANIM_BuzzyBeetle_Anim02,
    STATUS_KEY_STOP,      ANIM_BuzzyBeetle_Anim1A,
    STATUS_KEY_STATIC,    ANIM_BuzzyBeetle_Anim02,
    STATUS_KEY_DIZZY,     ANIM_BuzzyBeetle_Anim1C,
    STATUS_KEY_FEAR,      ANIM_BuzzyBeetle_Anim1C,
    STATUS_END,
};

#include "common/battle/SetAbsoluteStatusOffsets.inc.c"

EvtScript EVS_Init = {
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Ground)
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
    // Call(HPBarToHome, ACTOR_SELF)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_BUZZY, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
    // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Idle = {
    Return
    End
};

// switch the anim on LVar1 if toppled
EvtScript EVS_CheckToppleAnim = {
    Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar3)
    IfEq(LVar3, AVAL_State_Toppled)
        Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
        Set(LVar1, LVar2)
    EndIf
    Return
    End
};

s32 FlipPosOffsets[] = { 7, 13, 17, 21, 23, 24, 23, 21, 17, 13, 7, 0,  4,  7,  6,  4,  0,  2,  0 };

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Hit)
        EndCaseGroup
        CaseEq(EVENT_BURN_HIT)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfNe(LVar0, AVAL_State_Toppled)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim0B)
                SetConst(LVar2, ANIM_BuzzyBeetle_Anim0B)
                ExecWait(EVS_Enemy_BurnHit)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim1D)
                SetConst(LVar2, ANIM_BuzzyBeetle_Anim1D)
                ExecWait(EVS_Enemy_BurnHit)
            EndIf
        CaseEq(EVENT_BURN_DEATH)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfNe(LVar0, AVAL_State_Toppled)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim0B)
                SetConst(LVar2, ANIM_BuzzyBeetle_Anim0B)
                ExecWait(EVS_Enemy_BurnHit)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim0B)
                ExecWait(EVS_Enemy_Death)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim1D)
                SetConst(LVar2, ANIM_BuzzyBeetle_Anim1D)
                ExecWait(EVS_Enemy_BurnHit)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim1D)
                ExecWait(EVS_Enemy_Death)
            EndIf
            Return
        CaseEq(EVENT_SPIN_SMASH_HIT)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_SpinSmashHit)
        CaseEq(EVENT_FLIP_TRIGGER)
            Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
            Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Toppled)
            Call(SetTargetOffset, ACTOR_SELF, PRT_BUZZY, 0, 16)
            Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_BUZZY, -1, -9)
            Call(SetActorVar, ACTOR_SELF, AVAR_ToppleTurns, AVAL_ToppleTurnOne)
            Call(SetDefenseTable, ACTOR_SELF, PRT_BUZZY, Ref(ToppledDefense))
            Call(SetIdleAnimations, ACTOR_SELF, PRT_BUZZY, Ref(ToppledAnims))
            Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_FLIPPED, TRUE)
            Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Anim07)
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
            Thread
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -45)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -90)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -135)
                Wait(1)
                Call(SetActorRotation, ACTOR_SELF, 0, 0, -180)
                Wait(1)
            EndThread
            UseBuf(Ref(FlipPosOffsets))
            Loop(19)
                BufRead1(LVar0)
                Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
                Wait(1)
            EndLoop
            Call(SetActorDispOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
            Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
            Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Anim19)
            Call(EnableIdleScript, ACTOR_GREEN_BANDIT, IDLE_SCRIPT_DISABLE)
            Call(UseIdleAnimation, ACTOR_GREEN_BANDIT, FALSE)
            Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, AVAL_GreenPhase_SlowCannonAttack)
            Call(SetAnimation, ACTOR_GREEN_BANDIT, green_bandit_koopa::PRT_CANNON, ANIM_KoopaGang_Green_CannonSlowDown)
            Wait(30)
            Call(SetIdleAnimations, ACTOR_GREEN_BANDIT, green_bandit_koopa::PRT_CANNON, Ref(green_bandit_koopa::CannonSlowAnims))
            Call(EnableIdleScript, ACTOR_GREEN_BANDIT, IDLE_SCRIPT_ENABLE)
            Call(UseIdleAnimation, ACTOR_GREEN_BANDIT, TRUE)
        CaseEq(EVENT_SHOCK_HIT)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            ExecWait(EVS_Enemy_Knockback)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim04)
            ExecWait(EVS_Enemy_ReturnHome)
        CaseEq(EVENT_SHOCK_DEATH)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            ExecWait(EVS_Enemy_ShockHit)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_ZERO_DAMAGE)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim0C)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Anim17)
                Wait(8)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseEq(EVENT_IMMUNE)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim0C)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Anim0D)
                Wait(8)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseEq(EVENT_SPIKE_TAUNT)
            Wait(10)
            Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
            Call(GetStatusFlags, ACTOR_SELF, LVar3)
            IfFlag(LVar3, STATUS_FLAG_SHRINK)
                Add(LVar1, 9)
            Else
                Add(LVar1, 24)
            EndIf
            PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 20, 0)
            Wait(20)
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_SPIN_SMASH_DEATH)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_SpinSmashHit)
            SetConst(LVar0, PRT_BUZZY)
            SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
            ExecWait(EVS_CheckToppleAnim)
            ExecWait(EVS_Enemy_Death)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim01)
                ExecWait(EVS_Enemy_Recover)
            EndIf
        CaseEq(EVENT_SCARE_AWAY)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim04)
                SetConst(LVar2, ANIM_BuzzyBeetle_Anim07)
                ExecWait(EVS_Enemy_ScareAway)
                Return
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
        CaseEq(EVENT_BEGIN_AIR_LIFT)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim04)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
            EndIf
            ExecWait(EVS_Enemy_AirLift)
        CaseEq(EVENT_BLOW_AWAY)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim08)
            EndIf
            ExecWait(EVS_Enemy_BlowAway)
            Return
        CaseEq(EVENT_AIR_LIFT_FAILED)
            Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
            IfEq(LVar0, AVAL_State_Ground)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim0C)
                ExecWait(EVS_Enemy_NoDamageHit)
                Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Anim0D)
                Wait(8)
            Else
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
                ExecWait(EVS_Enemy_NoDamageHit)
            EndIf
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
    Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar3)
    IfEq(LVar3, AVAL_State_Toppled)
        Call(GetActorVar, ACTOR_SELF, AVAR_ToppleTurns, LVar0)
        Switch(LVar0)
            CaseEq(AVAL_ToppleTurnOne)
                Call(SetActorVar, ACTOR_SELF, AVAR_ToppleTurns, AVAL_ToppleTurnZero)
                Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
                Call(AddActorDecoration, ACTOR_SELF, PRT_BUZZY, 0, ACTOR_DECORATION_SWEAT)
                Wait(30)
                Call(RemoveActorDecoration, ACTOR_SELF, PRT_BUZZY, 0)
                Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
                Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, AVAL_GreenPhase_SlowCannonAttack)
            CaseEq(AVAL_ToppleTurnZero)
                Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(1.0))
                Call(AddActorDecoration, ACTOR_SELF, PRT_BUZZY, 0, ACTOR_DECORATION_SWEAT)
                Wait(20)
                Call(RemoveActorDecoration, ACTOR_SELF, PRT_BUZZY, 0)
                SetConst(LVar0, PRT_BUZZY)
                SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
                SetConst(LVar2, ANIM_BuzzyBeetle_Anim01)
                ExecWait(EVS_Enemy_FlipBackUp)
                Call(SetActorYaw, ACTOR_SELF, 0)
                Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Ground)
                Call(SetDefenseTable, ACTOR_SELF, PRT_BUZZY, Ref(UprightDefense))
                Call(SetIdleAnimations, ACTOR_SELF, PRT_BUZZY, Ref(DefaultAnims))
                Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_FLIPPED, FALSE)
                Goto(0)
        EndSwitch
    Else
        Label(0)
        Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, AVAL_GreenPhase_FastCannonAttack)
        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SHELL_SPIN)
        Call(SetAnimationRate, ACTOR_SELF, PRT_BUZZY, Float(4.0))
        Call(SetAnimation, ACTOR_SELF, PRT_BUZZY, ANIM_BuzzyBeetle_Anim05)
        Call(SetIdleAnimations, ACTOR_SELF, PRT_BUZZY, Ref(BuzzyShellAnims))
        Call(SetAnimation, ACTOR_GREEN_BANDIT, green_bandit_koopa::PRT_CANNON, ANIM_KoopaGang_Green_CannonIdle)
        Wait(10)
        Call(SetIdleAnimations, ACTOR_GREEN_BANDIT, green_bandit_koopa::PRT_CANNON, Ref(green_bandit_koopa::CannonAnims))
        Goto(1)
    EndIf
    Label(1)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

}; // namespace buzzy_beetle

ActorBlueprint GreenBanditKoopa = {
    .flags = ACTOR_FLAG_NO_SHADOW, // ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_NO_ATTACK,
    .maxHP = green_bandit_koopa::hp,
    .type = green_bandit_koopa::THIS_ACTOR_TYPE,
    .level = green_bandit_koopa::THIS_LEVEL,
    .partCount = ARRAY_COUNT(green_bandit_koopa::ActorParts),
    .partsData = green_bandit_koopa::ActorParts,
    .initScript = &green_bandit_koopa::EVS_Init,
    .statusTable = green_bandit_koopa::StatusTable,
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

ActorBlueprint BuzzyBeetle = {
    .flags = ACTOR_FLAG_NO_SHADOW,
    .maxHP = buzzy_beetle::hp,
    .type = ACTOR_TYPE_BUZZY_BEETLE_GREEN_PHASE,
    .level = ACTOR_LEVEL_BUZZY_BEETLE_GREEN_PHASE,
    .partCount = ARRAY_COUNT(buzzy_beetle::ActorParts),
    .partsData = buzzy_beetle::ActorParts,
    .initScript = &buzzy_beetle::EVS_Init,
    .statusTable = buzzy_beetle::StatusTable,
    .escapeChance = 60,
    .airLiftChance = 75,
    .hurricaneChance = 75,
    .spookChance = 75,
    .upAndAwayChance = 95,
    .spinSmashReq = 0,
    .powerBounceChance = 90,
    .coinReward = 1,
    .size = { 22, 18 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -8, 16 },
    .statusTextOffset = { 8, 13 },
};

}; // namespace battle::actor
