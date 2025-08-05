#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "entity.h"
#include "sprite/npc/CalamityKammy.h"
#include "sprite/player.h"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace calamity_kammy {

extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_HandleEvent;
// extern EvtScript EVS_CommentOnHit;
extern EvtScript EVS_Death;
extern EvtScript EVS_Attack_DropBlock;

enum ActorPartIDs {
    PRT_MAIN        = 1,
    // PRT_BROOM       = 2,
};

enum N(ActorVars) {
    AVAR_PlayerTurnCount    = 0,
    AVAR_Speaking           = 1,
};

// Actor Stats
constexpr s32 hp = 10;
constexpr s32 dmgBlockDrop = 50;

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_CalamityKammy_Idle,
    STATUS_END,
};

// s32 BroomAnims[] = {
//     STATUS_KEY_NORMAL,    ANIM_BattleKammy_Anim0C,
//     STATUS_END,
// };

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
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
        .index = PRT_MAIN,
        .posOffset = { 0, 0, 0 },
        .targetOffset = { -10, 35 },
        .opacity = 255,
        .idleAnimations = DefaultAnims,
        .defenseTable = DefenseTable,
        .eventFlags = 0,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, -8 },
    },
    // {
    //     .flags = ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET | ACTOR_PART_FLAG_USE_ABSOLUTE_POSITION,
    //     .index = PRT_BROOM,
    //     .posOffset = { 0, 0, 0 },
    //     .targetOffset = { 0, 0 },
    //     .opacity = 255,
    //     .idleAnimations = BroomAnims,
    //     .defenseTable = DefenseTable,
    //     .eventFlags = 0,
    //     .elementImmunityFlags = 0,
    //     .projectileTargetOffset = { 0, 0 },
    // },
};

API_CALLABLE(MerleeAttackSpell) {
    BattleStatus* battleStatus = &gBattleStatus;
    EncounterStatus* currentEncounter = &gCurrentEncounter;
    PlayerData* playerData = &gPlayerData;

    playerData->merleeSpellType = MERLEE_SPELL_ATK_BOOST;
    playerData->merleeCastsLeft = 1;
    playerData->merleeTurnCount = 1;

    return ApiStatus_DONE2;
}

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    // Call(SetBattleFlagBits2, BS_FLAGS2_DONT_STOP_MUSIC, true)
    Call(SetActorVar, ACTOR_SELF, AVAR_PlayerTurnCount, 0)
    Call(SetActorVar, ACTOR_SELF, AVAR_Speaking, false)
    Call(MerleeAttackSpell)
    Return
    End
};

// s32 BobPhase = 0;

// API_CALLABLE(AddFlightBobbing) {
//     Actor* actor = get_actor(script->owner1.actorID);

//     BobPhase += 9;
//     BobPhase = clamp_angle(BobPhase);
//     actor->verticalRenderOffset = sin_rad(DEG_TO_RAD(BobPhase)) * 3.0f;

//     return ApiStatus_DONE2;
// }

EvtScript EVS_Idle = {
    // Loop(0)
    //     Call(AddFlightBobbing)
    //     Wait(1)
    // EndLoop
    Return
    End
};

EvtScript EVS_HandleEvent = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetLastEvent, ACTOR_SELF, LVar0)
    Switch(LVar0)
        CaseEq(EVENT_BEGIN_FIRST_STRIKE)
        CaseOrEq(EVENT_HIT_COMBO)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_CalamityKammy_Hurt) // Hurt
            ExecWait(EVS_Enemy_Hit)
            // ExecWait(EVS_CommentOnHit)
        EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_CalamityKammy_Idle) // Idle
            ExecWait(EVS_Enemy_NoDamageHit)
            // ExecWait(EVS_CommentOnHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_CalamityKammy_Hurt) // Hurt or optional Death
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            ExecWait(EVS_Death)
            Return
        CaseDefault
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

// #include "world/common/todo/SetEntityPositionF.inc.c"

API_CALLABLE(SetEntityPositionF) {
    Bytecode* args = script->ptrReadPos;
    s32 entityIndex = evt_get_variable(script, *args++);
    f32 x = evt_get_variable(script, *args++);
    f32 y = evt_get_variable(script, *args++);
    f32 z = evt_get_variable(script, *args++);
    Entity* entity = get_entity_by_index(entityIndex);

    entity->pos.x = x;
    entity->pos.y = y;
    entity->pos.z = z;
    return ApiStatus_DONE2;
}

// #include "world/common/todo/GetEntityPosition.inc.c"

API_CALLABLE(GetEntityPosition) {
    Bytecode* args = script->ptrReadPos;
    Entity* entity = get_entity_by_index(evt_get_variable(script, *args++));

    evt_set_variable(script, *args++, entity->pos.x);
    evt_set_variable(script, *args++, entity->pos.y);
    evt_set_variable(script, *args++, entity->pos.z);
    return ApiStatus_DONE2;
}

API_CALLABLE(BlockAppear) {
    Entity* entity = get_entity_by_index(script->varTable[9]);

    if (isInitialCall) {
        script->functionTemp[0] = 0;
        script->functionTemp[1] = 60;
    }

    entity->scale.x = (60 - script->functionTemp[1]) / 60.0f;
    entity->scale.y = (60 - script->functionTemp[1]) / 60.0f;
    entity->scale.z = (60 - script->functionTemp[1]) / 60.0f;
    entity->rot.y = (1.0f - cos_rad(entity->scale.y * PI)) * 1080.0f * 0.5f;
    script->functionTemp[1]--;

    if (script->functionTemp[1] == -1) {
        return ApiStatus_DONE2;
    }
    return ApiStatus_BLOCK;
}

API_CALLABLE(FadeInScreenBlur) {
    if (isInitialCall) {
        script->functionTemp[0] = 20;
        set_screen_overlay_center(SCREEN_LAYER_BACK, 0, 0, 0);
        set_screen_overlay_center(SCREEN_LAYER_BACK, 1, 320, 240);
        set_screen_overlay_params_back(OVERLAY_BLUR, 150);
    }
    if (script->functionTemp[0] != 0) {
        script->functionTemp[0]--;
        return ApiStatus_BLOCK;
    }

    set_screen_overlay_center(SCREEN_LAYER_BACK, 0, 0, 0);
    set_screen_overlay_center(SCREEN_LAYER_BACK, 1, 320, 240);
    set_screen_overlay_params_back(OVERLAY_NONE, -1);
    return ApiStatus_DONE2;
}

API_CALLABLE(FadeOutScreenBlur) {
    if (isInitialCall) {
        script->functionTemp[0] = 30;
        set_screen_overlay_center(SCREEN_LAYER_BACK, 0, 0, 0);
        set_screen_overlay_center(SCREEN_LAYER_BACK, 1, 320, 240);
        set_screen_overlay_params_back(OVERLAY_BLUR, 150);
    }
    if (script->functionTemp[0] != 0) {
        script->functionTemp[0]--;
        return ApiStatus_BLOCK;
    }

    set_screen_overlay_center(SCREEN_LAYER_BACK, 0, 0, 0);
    set_screen_overlay_center(SCREEN_LAYER_BACK, 1, 320, 240);
    set_screen_overlay_params_back(OVERLAY_NONE, -1);
    return ApiStatus_DONE2;
}

API_CALLABLE(DropBlock) {
    CollisionStatus* collisionStatus = &gCollisionStatus;
    PlayerStatus* playerStatus = &gPlayerStatus;
    s32 entityIndex = script->varTable[9];
    Entity* entity = get_entity_by_index(entityIndex);

    entity->collisionTimer = 0;
    collisionStatus->lastWallHammered = entityIndex | COLLISION_WITH_ENTITY_BIT;
    playerStatus->flags |= PS_FLAG_HAMMER_CHECK;
    entity->collisionFlags = ENTITY_COLLISION_PLAYER_HAMMER;
    playerStatus->actionState = ACTION_STATE_HAMMER;
    entity->blueprint->fpHandleCollision(entity);
    entity->collisionTimer = 10;
    entity->flags |= ENTITY_FLAG_DETECTED_COLLISION;
    collisionStatus->lastWallHammered = -1;

    return ApiStatus_DONE2;
}

EvtScript EVS_TakeTurn = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    ExecWait(EVS_Attack_DropBlock)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_Attack_DropBlock = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(EnableBattleStatusBar, false)
    Call(SetTargetActor, ACTOR_SELF, ACTOR_PLAYER)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Wait(30)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 25)
    // Add(LVar1, 40)

    Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    Call(SetBattleCamTarget, 50, 10, 5)
    Call(SetBattleCamDist, 325)
    Call(MoveBattleCamOver, 60)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Still) // Stonefaced
    Wait(60)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(SetBattleCamDist, 425)
    Call(MoveBattleCamOver, 1)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KAMMY_SUMMON_BLOCK)
    Thread
        DebugPrintf("Mario Animation Thread\n")
        Wait(4)
        Call(SetGoalToTarget, ACTOR_SELF)
        Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
        Add(LVar2, 2)
        Call(UseIdleAnimation, ACTOR_PLAYER, false)
        Call(SetAnimation, ACTOR_PLAYER, 0, ANIM_Mario1_Flail)
        Wait(24)
        Call(UseIdleAnimation, ACTOR_PLAYER, true)
        Call(UseIdleAnimation, ACTOR_PLAYER, false)
        // Call(SetAnimation, ACTOR_PLAYER, 0, ANIM_Mario1_LookUp)
        Call(SetAnimation, ACTOR_PLAYER, 0, ANIM_Mario1_Question)
        Wait(20)
        Call(PlaySoundAtActor, ACTOR_PLAYER, SOUND_EMOTE_QUESTION)
        Call(GetActorPos, ACTOR_PLAYER, LVar0, LVar1, LVar2)
        Add(LVar1, 20)
        PlayEffect(EFFECT_EMOTE, EMOTE_QUESTION, 0, LVar0, LVar1, LVar2, 20, 315, 30, 0, 0)
        Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KAMMY_RAISE_OBJECT)
        Wait(20)
        DebugPrintf("Mario Animation EndThread\n")
    EndThread
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_GunFireStart)
    Wait(8)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_GunFireEnd)
    DebugPrintf("Prank Shot Here\n")
    Wait(7)
    // Sub(LVar0, 15)
    // Add(LVar1, 30)
    // Set(LVar3, LVar1)
    // Add(LVar3, 20)

    Call(GetActorPos, ACTOR_PLAYER, LVar5, LVar6, LVar7)
    Add(LVar6, 200)
    Add(LVar7, 2)
    Call(MakeEntity, Ref(Entity_Hammer3Block), LVar5, LVar6, LVar7, 0, MAKE_ENTITY_END)
    Set(LVar9, LVar0)
    Call(BlockAppear)
    Thread
        Call(FadeInScreenBlur)
    EndThread
    // Call(GetActorPos, ACTOR_PLAYER, LVar5, LVar6, LVar7)
    // Add(LVar6, 200)
    // Add(LVar7, 2)
    // Call(SetEntityPositionF, LVar9, LVar5, LVar6, LVar7)

    // Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    // Call(SetBattleCamTarget, -80, 37, 5)
    // Call(SetBattleCamDist, 256)
    // Wait(10)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(GetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    ChildThread
        Wait(10)
        Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
        Call(SetBattleCamTarget, -80, 37, 5)
        Call(SetBattleCamDist, 350)
        // Call(AddBattleCamDist, 100)
        Call(MoveBattleCamOver, 10)
        Wait(5)
        Call(FadeOutScreenBlur)
    EndChildThread
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KAMMY_LOWER_OBJECT)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Laugh)
    DebugPrintf("Laugh Anim\n")
    Wait(6)
    Set(LVar5, LVar1)
    Add(LVar5, 200)
    Call(SetEntityPositionF, LVar9, LVar0, LVar5, LVar2)
    Call(GetEntityPosition, LVar9, LVar2, LVar3, LVar4)
    Call(MakeLerp, LVar5, LVar1, 20, EASING_CUBIC_IN)
    Loop(0)
        Call(UpdateLerp)
        Call(SetEntityPositionF, LVar9, LVar2, LVar0, LVar4)
        Wait(1)
        IfNe(LVar1, 1)
            BreakLoop
        EndIf
    EndLoop
    Call(DropBlock)
    DebugPrintf("Call:DropBlock\n")
    ChildThread
        Call(StartRumble, BTL_RUMBLE_HIT_EXTREME)
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(1.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(3.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(6.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(5.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(4.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(3.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(2.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(1.0))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(0.5))
        Call(ShakeCam, CAM_BATTLE, 0, 2, Float(0.2))
    EndChildThread
    Wait(2)
    Call(UseIdleAnimation, ACTOR_PLAYER, true)
    Call(SetGoalToTarget, ACTOR_SELF)
    Call(SetDamageSource, DMG_SRC_CRUSH)
    Call(EnemyDamageTarget, ACTOR_SELF, LVar0, DAMAGE_TYPE_MAGIC | DAMAGE_TYPE_UNBLOCKABLE | DAMAGE_TYPE_NO_CONTACT, SUPPRESS_EVENT_ALL, 0, dmgBlockDrop, BS_FLAGS1_TRIGGER_EVENTS)
    ExecWaitOnActor(ACTOR_PLAYER, EVS_Player_Crushed_Impl)
    Set(LVarF, LVar0)
    Switch(LVarF)
        CaseOrEq(HIT_RESULT_HIT)
        CaseOrEq(HIT_RESULT_NO_DAMAGE)
            Wait(19)
            Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
        EndCaseGroup
    EndSwitch
    // Call(GetLastDamage, ACTOR_PARTNER, LVar0)
    // IfEq(LVar0, 0)
    //     Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
    //     Call(SetBattleCamTarget, 69, 14, -18)
    //     Call(SetBattleCamDist, 340)
    //     Call(SetBattleCamOffsetY, 62)
    //     Call(MoveBattleCamOver, 20)
        // Call(FreezeBattleCam, true)
        // Wait(20)
        // Thread
        //     Call(SetGoalToHome, ACTOR_SELF)
        //     Call(AddGoalPos, ACTOR_SELF, 10, 5, 0)
        //     Call(FlyToGoal, ACTOR_SELF, 6, 0, EASING_LINEAR)
        // EndThread
        // Call(ActorSpeak, MSG_CH8_009A, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Talk, ANIM_CalamityKammy_Idle)
        // Wait(10)
        // Call(FreezeBattleCam, false)
    //     Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    // EndIf
    Wait(10)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

EvtScript EVS_HandlePhase = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(GetBattlePhase, LVar0)
    Switch(LVar0)
        CaseEq(PHASE_PLAYER_BEGIN)
            Call(GetActorVar, ACTOR_SELF, AVAR_PlayerTurnCount, LVar0)
            Switch(LVar0)
                CaseEq(0)
                    Call(UseBattleCamPreset, BTL_CAM_ACTOR)
                    Call(BattleCamTargetActor, ACTOR_SELF)
                    // Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
                    // Call(SetBattleCamTarget, 50, 10, 5)
                    // Call(SetBattleCamDist, 340)
                    // Call(SetBattleCamOffsetY, 62)
                    Call(MoveBattleCamOver, 20)
                    Wait(20)
                    // Call(ActorSpeak, MSG_CH8_0093, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Talk, ANIM_CalamityKammy_Idle)
                    // Wait(10)
                    // Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
                    // Call(SetBattleCamTarget, -56, -9, -18)
                    // Call(SetBattleCamDist, 340)
                    // Call(SetBattleCamOffsetY, 62)
                    // Call(MoveBattleCamOver, 1)
                    // Wait(10)
                    // Call(ActorSpeak, MSG_CH8_0094, ACTOR_PARTNER, 1, ANIM_Twink_ShoutAngry, ANIM_Twink_Angry)
                    // Wait(10)
                    // Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
                    // Call(SetBattleCamTarget, -66, -9, -18)
                    // Call(SetBattleCamDist, 340)
                    // Call(SetBattleCamOffsetY, 62)
                    // Call(MoveBattleCamOver, 10)
                    // Wait(10)
                    // Call(ActorSpeak, MSG_CH8_0095, ACTOR_PLAYER, 1, ANIM_BattleParakarry_EnterShell, ANIM_BattleParakarry_ShellFly)
                    // Wait(10)
                    // Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
                    Call(AddActorVar, ACTOR_SELF, AVAR_PlayerTurnCount, 1)
            EndSwitch
        CaseEq(PHASE_ENEMY_BEGIN)
            // do nothing
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, true)
    Return
    End
};

// EvtScript EVS_CommentOnHit = {
//     Call(FreezeBattleState, true)
//     Call(UseIdleAnimation, ACTOR_SELF, false)
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
//     Call(SetActorVar, ACTOR_SELF, AVAR_Speaking, true)
//     Call(GetActorHP, ACTOR_SELF, LVar0)
//     Switch(LVar0)
//         CaseEq(10)
//             Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
//             Call(SetBattleCamTarget, 69, 14, -18)
//             Call(SetBattleCamDist, 340)
//             Call(SetBattleCamOffsetY, 62)
//             Call(MoveBattleCamOver, 20)
//             Call(FreezeBattleCam, true)
//             Wait(20)
//             Call(ActorSpeak, MSG_CH8_0097, ACTOR_SELF, PRT_MAIN, ANIM_BattleKammy_Anim09, ANIM_CalamityKammy_Idle)
//             Wait(10)
//             Call(FreezeBattleCam, false)
//             Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
//         CaseOrEq(9)
//         CaseOrEq(8)
//             Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
//             Call(SetBattleCamTarget, 69, 14, -18)
//             Call(SetBattleCamDist, 340)
//             Call(SetBattleCamOffsetY, 62)
//             Call(MoveBattleCamOver, 20)
//             Call(FreezeBattleCam, true)
//             Wait(20)
//             Call(ActorSpeak, MSG_CH8_0098, ACTOR_SELF, PRT_MAIN, ANIM_BattleKammy_Anim0A, ANIM_CalamityKammy_Idle)
//             Wait(10)
//             Call(FreezeBattleCam, false)
//             Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
//         EndCaseGroup
//         CaseOrEq(7)
//         CaseOrEq(6)
//         CaseOrEq(5)
//         CaseOrEq(4)
//         CaseOrEq(3)
//         CaseOrEq(2)
//         CaseOrEq(1)
//             Call(GetActorVar, ACTOR_SELF, AVAR_PlayerTurnCount, LVar0)
//             IfEq(LVar0, 2)
//                 BreakSwitch
//             EndIf
//             Call(UseBattleCamPreset, BTL_CAM_REPOSITION)
//             Call(SetBattleCamTarget, 69, 14, -18)
//             Call(SetBattleCamDist, 340)
//             Call(SetBattleCamOffsetY, 62)
//             Call(MoveBattleCamOver, 20)
//             Call(FreezeBattleCam, true)
//             Wait(20)
//             Call(ActorSpeak, MSG_CH8_0099, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Talk, ANIM_CalamityKammy_Idle)
//             Wait(10)
//             Call(FreezeBattleCam, false)
//             Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
//             Call(AddActorVar, ACTOR_SELF, AVAR_PlayerTurnCount, 1)
//         EndCaseGroup
//     EndSwitch
//     Call(SetActorVar, ACTOR_SELF, AVAR_Speaking, false)
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, true)
//     Call(FreezeBattleState, false)
//     Return
//     End
// };

EvtScript EVS_Death = {
    Call(UseIdleAnimation, ACTOR_SELF, false)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(HideHealthBar, ACTOR_SELF)
    Call(UseBattleCamPreset, BTL_CAM_ACTOR)
    Call(BattleCamTargetActor, ACTOR_SELF)
    Call(MoveBattleCamOver, 60)
    Wait(10)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Hurt) // Hurt?
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar2, 1)
    // Call(SetPartPos, ACTOR_SELF, PRT_BROOM, LVar0, LVar1, LVar2)
    // Call(SetPartFlagBits, ACTOR_SELF, PRT_BROOM, ACTOR_PART_FLAG_INVISIBLE, false)
    // Call(PlaySoundAtActor, ACTOR_SELF, SOUND_FALL_QUICK)
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Set(LVar1, 0)
    // Call(SetActorJumpGravity, ACTOR_SELF, Float(0.8))
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(JumpToGoal, ACTOR_SELF, 15, false, true, false)
    // Call(PlaySoundAtActor, ACTOR_SELF, SOUND_ACTOR_COLLAPSE)
    Call(UseBattleCamPreset, BTL_CAM_INTERRUPT)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(JumpToGoal, ACTOR_SELF, 10, false, true, false)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(JumpToGoal, ACTOR_SELF, 5, false, true, false)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KNOCKOUT_CHIRPING)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(ForceHomePos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SEEING_STARS)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_Defeat)
    Wait(78)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_CalamityKammy_WhiteFlag)
    Wait(11)
    Call(SetBattleFlagBits, BS_FLAGS1_DISABLE_CELEBRATION | BS_FLAGS1_BATTLE_FLED, true)
    Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_DMG_APPLY, true)
    // Call(ActorSpeak, MSG_CH8_009B, ACTOR_SELF, PRT_MAIN, ANIM_BattleKammy_Anim03, ANIM_CalamityKammy_Defeat)
    Wait(10)
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(MoveBattleCamOver, 60)
    Wait(40)
    Return
    End
};

}; // namespace calamity_kammy

ActorBlueprint CalamityKammy = {
    .flags = 0,
    .maxHP = calamity_kammy::hp,
    .type = ACTOR_TYPE_CALAMITY_KAMMY,
    .level = ACTOR_LEVEL_CALAMITY_KAMMY,
    .partCount = ARRAY_COUNT(calamity_kammy::ActorParts),
    .partsData = calamity_kammy::ActorParts,
    .initScript = &calamity_kammy::EVS_Init,
    .statusTable = calamity_kammy::StatusTable,
    .escapeChance = 0,
    .airLiftChance = 0,
    .hurricaneChance = 0,
    .spookChance = 0,
    .upAndAwayChance = 0,
    .spinSmashReq = 4,
    .powerBounceChance = 80,
    .coinReward = 0,
    .size = { 24, 32 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
