#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "mapfs/trn_bt00_shape.h"
#include "sprite/npc/KoopaGang.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace koopa_gang_green {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_UnstableGang;
extern EvtScript EVS_ToppleGang;
extern EvtScript EVS_Move_Spin;
extern EvtScript EVS_Move_FormTower;

enum ActorPartIDs {
    PRT_MAIN               = 1,
};

enum ActorVars {
    AVAR_KoopaGang_ToppleTurns = 2,
    AVAL_KoopaGang_ToppleTurnZero = 0,
    AVAL_KoopaGang_ToppleTurnOne = 1,
    AVAR_KoopaGang_TowerState        = 3,
    AVAL_KoopaGang_TowerState_None           = 0,
    AVAL_KoopaGang_TowerState_Stable         = 1,
    AVAL_KoopaGang_TowerState_Unstable       = 2,
    AVAL_KoopaGang_TowerState_Toppled        = 3, // also init value to prevent first-turn tower attack
    AVAR_KoopaGang_Flags                          = 4,
    AFLAG_KoopaGang_TowerUnstable            = 0x010,
    AFLAG_KoopaGang_PlayerHitTower           = 0x040,
    AFLAG_KoopaGang_PartnerHitTower          = 0x080,
};

// Actor Stats
constexpr s32 hp = 1;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   0,
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
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,     ANIM_KoopaGang_Green_ShellSpin,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_TARGET_NO_DAMAGE, TRUE)
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
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
            // Call(GetBattleFlags, LVar0)
            // IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // Else
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if the attack was explosive, set both flags
            // Call(GetLastElement, LVar0)
            // IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if this was the second hit, topple the tower
            // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //         ExecWait(EVS_ToppleGang)
            //         Wait(20)
            //         Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            //         Return
            //     EndIf
            // EndIf
            // ExecWait(EVS_UnstableGang)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
        //     Call(GetBattleFlags, LVar0)
        //     IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     Else
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if the attack was explosive, set both flags
        //     Call(GetLastElement, LVar0)
        //     IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if this was the second hit, topple the tower
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //             ExecWait(EVS_ToppleGang)
        //             Wait(20)
        //             Call(UseIdleAnimation, ACTOR_SELF, TRUE)
        //             Return
        //         EndIf
        //     EndIf
        //     Wait(30)
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
        //     IfNe(LVar0, AVAL_KoopaGang_TowerState_Unstable)
        //         ExecWait(EVS_UnstableGang)
        //     EndIf
        // EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_TopEnterShell)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfEq(LVar0, 0)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Green_TopEnterShell)
                Wait(10)
            EndIf
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_TopExitShell)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_Hurt)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_Hurt)
            ExecWait(EVS_Defeat)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Green_Idle)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    //Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KingBoo_Idle)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(10)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

#define LBL_FORMTOWER 0
#define LBL_ENDTURN 1
EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
    // Switch(LVar0)
    //     CaseEq(AVAL_KoopaGang_TowerState_None)
    //         Label(LBL_FORMTOWER)
    //             ExecWait(EVS_Move_FormTower)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Stable)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Unstable)
    //         ExecWait(EVS_Move_Spin)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Toppled)
    //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, LVar0)
    //         Switch(LVar0)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnOne)
    //                 // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, AVAL_KoopaGang_ToppleTurnZero)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnZero)
    //                 Goto(LBL_FORMTOWER)
    //         EndSwitch
    // EndSwitch
    // Label(LBL_ENDTURN)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_FormTower = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)

    // Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 45)
    // Sub(LVar1, -54)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Leap)
    // Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Midair)
    // Wait(10)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Land)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Idle)
    // Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))


    // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Unstable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_Spin = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_TOWER_SPIN_3)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Green_ShellSpin)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};


EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace koopa_gang_green

namespace koopa_gang_yellow {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_UnstableGang;
extern EvtScript EVS_ToppleGang;
extern EvtScript EVS_Move_Spin;
extern EvtScript EVS_Move_FormTower;

enum ActorPartIDs {
    PRT_MAIN               = 1,
};

enum ActorVars {
    AVAR_KoopaGang_ToppleTurns = 2,
    AVAL_KoopaGang_ToppleTurnZero = 0,
    AVAL_KoopaGang_ToppleTurnOne = 1,
    AVAR_KoopaGang_TowerState        = 3,
    AVAL_KoopaGang_TowerState_None           = 0,
    AVAL_KoopaGang_TowerState_Stable         = 1,
    AVAL_KoopaGang_TowerState_Unstable       = 2,
    AVAL_KoopaGang_TowerState_Toppled        = 3, // also init value to prevent first-turn tower attack
    AVAR_KoopaGang_Flags                          = 4,
    AFLAG_KoopaGang_TowerUnstable            = 0x010,
    AFLAG_KoopaGang_PlayerHitTower           = 0x040,
    AFLAG_KoopaGang_PartnerHitTower          = 0x080,
};

// Actor Stats
constexpr s32 hp = 1;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   0,
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
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,     ANIM_KoopaGang_Yellow_ShellSpin,
    STATUS_END,
};

s32 BasicYellowHurtAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Yellow_Hurt,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_TARGET_NO_DAMAGE, TRUE)
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
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
            // Call(GetBattleFlags, LVar0)
            // IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // Else
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if the attack was explosive, set both flags
            // Call(GetLastElement, LVar0)
            // IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if this was the second hit, topple the tower
            // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //         ExecWait(EVS_ToppleGang)
            //         Wait(20)
            //         Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            //         Return
            //     EndIf
            // EndIf
            // ExecWait(EVS_UnstableGang)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
        //     Call(GetBattleFlags, LVar0)
        //     IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     Else
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if the attack was explosive, set both flags
        //     Call(GetLastElement, LVar0)
        //     IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if this was the second hit, topple the tower
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //             ExecWait(EVS_ToppleGang)
        //             Wait(20)
        //             Call(UseIdleAnimation, ACTOR_SELF, TRUE)
        //             Return
        //         EndIf
        //     EndIf
        //     Wait(30)
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
        //     IfNe(LVar0, AVAL_KoopaGang_TowerState_Unstable)
        //         ExecWait(EVS_UnstableGang)
        //     EndIf
        // EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_TopEnterShell)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfEq(LVar0, 0)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Yellow_TopEnterShell)
                Wait(10)
            EndIf
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_TopExitShell)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_Hurt)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_Hurt)
            ExecWait(EVS_Defeat)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Yellow_Idle)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    //Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KingBoo_Idle)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(10)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

#define LBL_FORMTOWER 0
#define LBL_ENDTURN 1
EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
    // Switch(LVar0)
    //     CaseEq(AVAL_KoopaGang_TowerState_None)
    //         Label(LBL_FORMTOWER)
    //             ExecWait(EVS_Move_FormTower)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Stable)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Unstable)
    //         ExecWait(EVS_Move_Spin)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Toppled)
    //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, LVar0)
    //         Switch(LVar0)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnOne)
    //                 // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, AVAL_KoopaGang_ToppleTurnZero)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnZero)
    //                 Goto(LBL_FORMTOWER)
    //         EndSwitch
    // EndSwitch
    // Label(LBL_ENDTURN)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_FormTower = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)

    // Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 45)
    // Sub(LVar1, -54)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Leap)
    // Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Midair)
    // Wait(10)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Land)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Idle)
    // Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))


    // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Unstable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_Spin = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_TOWER_SPIN_3)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Yellow_ShellSpin)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};


EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace koopa_gang_yellow

namespace koopa_gang_black {

extern s32 BlackAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_UnstableGang;
extern EvtScript EVS_ToppleGang;
extern EvtScript EVS_Move_Spin;
extern EvtScript EVS_Move_FormTower;

enum ActorPartIDs {
    PRT_MAIN               = 1,
};

enum ActorVars {
    AVAR_KoopaGang_ToppleTurns = 2,
    AVAL_KoopaGang_ToppleTurnZero = 0,
    AVAL_KoopaGang_ToppleTurnOne = 1,
    AVAR_KoopaGang_TowerState        = 3,
    AVAL_KoopaGang_TowerState_None           = 0,
    AVAL_KoopaGang_TowerState_Stable         = 1,
    AVAL_KoopaGang_TowerState_Unstable       = 2,
    AVAL_KoopaGang_TowerState_Toppled        = 3, // also init value to prevent first-turn tower attack
    AVAR_KoopaGang_Flags                          = 4,
    AFLAG_KoopaGang_TowerUnstable            = 0x010,
    AFLAG_KoopaGang_PlayerHitTower           = 0x040,
    AFLAG_KoopaGang_PartnerHitTower          = 0x080,
};

// Actor Stats
constexpr s32 hp = 1;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   0,
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
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,     ANIM_KoopaGang_Black_ShellSpin,
    STATUS_END,
};

s32 BasicBlackHurtAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Black_Hurt,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_TARGET_NO_DAMAGE, TRUE)
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
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
            // Call(GetBattleFlags, LVar0)
            // IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // Else
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if the attack was explosive, set both flags
            // Call(GetLastElement, LVar0)
            // IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if this was the second hit, topple the tower
            // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //         ExecWait(EVS_ToppleGang)
            //         Wait(20)
            //         Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            //         Return
            //     EndIf
            // EndIf
            // ExecWait(EVS_UnstableGang)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
        //     Call(GetBattleFlags, LVar0)
        //     IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     Else
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if the attack was explosive, set both flags
        //     Call(GetLastElement, LVar0)
        //     IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if this was the second hit, topple the tower
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //             ExecWait(EVS_ToppleGang)
        //             Wait(20)
        //             Call(UseIdleAnimation, ACTOR_SELF, TRUE)
        //             Return
        //         EndIf
        //     EndIf
        //     Wait(30)
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
        //     IfNe(LVar0, AVAL_KoopaGang_TowerState_Unstable)
        //         ExecWait(EVS_UnstableGang)
        //     EndIf
        // EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_TopEnterShell)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfEq(LVar0, 0)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_TopEnterShell)
                Wait(10)
            EndIf
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_TopExitShell)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_Hurt)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_Hurt)
            ExecWait(EVS_Defeat)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Black_Idle)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    //Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KingBoo_Idle)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(10)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

#define LBL_FORMTOWER 0
#define LBL_ENDTURN 1
EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
    // Switch(LVar0)
    //     CaseEq(AVAL_KoopaGang_TowerState_None)
    //         Label(LBL_FORMTOWER)
    //             ExecWait(EVS_Move_FormTower)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Stable)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Unstable)
    //         ExecWait(EVS_Move_Spin)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Toppled)
    //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, LVar0)
    //         Switch(LVar0)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnOne)
    //                 // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, AVAL_KoopaGang_ToppleTurnZero)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnZero)
    //                 Goto(LBL_FORMTOWER)
    //         EndSwitch
    // EndSwitch
    // Label(LBL_ENDTURN)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_FormTower = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)

    // Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 45)
    // Sub(LVar1, -54)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Leap)
    // Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Midair)
    // Wait(10)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Land)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Idle)
    // Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))


    // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Unstable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_Spin = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_TOWER_SPIN_3)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_ShellSpin)
    Call(GetPartPos, ACTOR_SELF, PRT_MAIN, LVar0, LVar1, LVar2)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};


EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace koopa_gang_black

namespace koopa_gang_red {

extern s32 DefaultAnims[];
extern EvtScript EVS_Init;
extern EvtScript EVS_Idle;
extern EvtScript EVS_HandleEvent;
extern EvtScript EVS_HandlePhase;
extern EvtScript EVS_TakeTurn;
extern EvtScript EVS_Defeat;
extern EvtScript EVS_UnstableGang;
extern EvtScript EVS_ToppleGang;
extern EvtScript EVS_Move_Spin;
extern EvtScript EVS_Move_FormTower;

enum ActorPartIDs {
    PRT_MAIN               = 1,
};

enum ActorVars {
    AVAR_KoopaGang_ToppleTurns = 2,
    AVAL_KoopaGang_ToppleTurnZero = 0,
    AVAL_KoopaGang_ToppleTurnOne = 1,
    AVAR_KoopaGang_TowerState        = 3,
    AVAL_KoopaGang_TowerState_None           = 0,
    AVAL_KoopaGang_TowerState_Stable         = 1,
    AVAL_KoopaGang_TowerState_Unstable       = 2,
    AVAL_KoopaGang_TowerState_Toppled        = 3, // also init value to prevent first-turn tower attack
    AVAR_KoopaGang_Flags                          = 4,
    AFLAG_KoopaGang_TowerUnstable            = 0x010,
    AFLAG_KoopaGang_PlayerHitTower           = 0x040,
    AFLAG_KoopaGang_PartnerHitTower          = 0x080,
};

// Actor Stats
constexpr s32 hp = 1;

s32 DefaultDefense[] = {
    ELEMENT_NORMAL,   0,
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
        .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
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
};

s32 DefaultAnims[] = {
    STATUS_KEY_NORMAL,     ANIM_KoopaGang_Red_ShellSpin,
    STATUS_END,
};

s32 BasicRedHurtAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Red_Hurt,
    STATUS_END,
};

#include "common/StartRumbleWithParams.inc.c"

EvtScript EVS_Init = {
    Call(BindTakeTurn, ACTOR_SELF, Ref(EVS_TakeTurn))
    Call(BindIdle, ACTOR_SELF, Ref(EVS_Idle))
    Call(BindHandleEvent, ACTOR_SELF, Ref(EVS_HandleEvent))
    Call(BindHandlePhase, ACTOR_SELF, Ref(EVS_HandlePhase))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(SetPartTargetFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_TARGET_NO_DAMAGE, TRUE)
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
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
            // Call(GetBattleFlags, LVar0)
            // IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // Else
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if the attack was explosive, set both flags
            // Call(GetLastElement, LVar0)
            // IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            //     BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // EndIf
            // // if this was the second hit, topple the tower
            // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
            // IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
            //     IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
            //         ExecWait(EVS_ToppleGang)
            //         Wait(20)
            //         Call(UseIdleAnimation, ACTOR_SELF, TRUE)
            //         Return
            //     EndIf
            // EndIf
            // ExecWait(EVS_UnstableGang)
        CaseOrEq(EVENT_HIT)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_Hurt)
            ExecWait(EVS_Enemy_Hit)
            Wait(10)
            // set flags for player or partner hitting the koopa bros tower
        //     Call(GetBattleFlags, LVar0)
        //     IfFlag(LVar0, BS_FLAGS1_PARTNER_ACTING)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     Else
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if the attack was explosive, set both flags
        //     Call(GetLastElement, LVar0)
        //     IfFlag(LVar0, DAMAGE_TYPE_SHOCK | DAMAGE_TYPE_BLAST)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //         BitwiseOrConst(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //         Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     EndIf
        //     // if this was the second hit, topple the tower
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_Flags, LVar0)
        //     IfFlag(LVar0, AFLAG_KoopaGang_PlayerHitTower)
        //         IfFlag(LVar0, AFLAG_KoopaGang_PartnerHitTower)
        //             ExecWait(EVS_ToppleGang)
        //             Wait(20)
        //             Call(UseIdleAnimation, ACTOR_SELF, TRUE)
        //             Return
        //         EndIf
        //     EndIf
        //     Wait(30)
        //     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
        //     IfNe(LVar0, AVAL_KoopaGang_TowerState_Unstable)
        //         ExecWait(EVS_UnstableGang)
        //     EndIf
        // EndCaseGroup
        CaseOrEq(EVENT_ZERO_DAMAGE)
        CaseOrEq(EVENT_IMMUNE)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_TopEnterShell)
            ExecWait(EVS_Enemy_NoDamageHit)
            Call(GetStatusFlags, ACTOR_SELF, LVar0)
            IfEq(LVar0, 0)
                Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_TopEnterShell)
                Wait(10)
            EndIf
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_TopExitShell)
            ExecWait(EVS_Enemy_NoDamageHit)
        EndCaseGroup
        CaseEq(EVENT_DEATH)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_Hurt)
            ExecWait(EVS_Enemy_Hit)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_Hurt)
            ExecWait(EVS_Defeat)
            Return
        CaseEq(EVENT_RECOVER_STATUS)
            SetConst(LVar0, PRT_MAIN)
            SetConst(LVar1, ANIM_KoopaGang_Red_Idle)
            ExecWait(EVS_Enemy_Recover)
    EndSwitch
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Defeat = {
    Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
    Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    Add(LVar1, 20)
    //Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KingBoo_Idle)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_EMOTE_IDEA)
    PlayEffect(EFFECT_EMOTE, EMOTE_EXCLAMATION, 0, LVar0, LVar1, LVar2, 24, 0, 25, 0, 0)
    Wait(10)
    Call(RemoveActor, ACTOR_SELF)
    Return
    End
};

#define LBL_FORMTOWER 0
#define LBL_ENDTURN 1
EvtScript EVS_TakeTurn = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    // Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)
    // Switch(LVar0)
    //     CaseEq(AVAL_KoopaGang_TowerState_None)
    //         Label(LBL_FORMTOWER)
    //             ExecWait(EVS_Move_FormTower)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Stable)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Unstable)
    //         ExecWait(EVS_Move_Spin)
    //         // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, TRUE)
    //         Goto(LBL_ENDTURN)
    //     CaseEq(AVAL_KoopaGang_TowerState_Toppled)
    //         Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, LVar0)
    //         Switch(LVar0)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnOne)
    //                 // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    //                 Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, AVAL_KoopaGang_ToppleTurnZero)
    //             CaseEq(AVAL_KoopaGang_ToppleTurnZero)
    //                 Goto(LBL_FORMTOWER)
    //         EndSwitch
    // EndSwitch
    // Label(LBL_ENDTURN)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_FormTower = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)

    // Call(SetActorJumpGravity, ACTOR_SELF, Float(1.6))
    // Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Sub(LVar0, 45)
    // Sub(LVar1, -54)
    // Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Leap)
    // Call(JumpToGoal, ACTOR_SELF, 10, FALSE, TRUE, FALSE)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Midair)
    // Wait(10)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Land)
    // Wait(5)
    // Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_Idle)
    // Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(DefaultDefense))


    // Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetActorVar, ACTOR_ENEMY0, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
    Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET, FALSE)
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Unstable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};

EvtScript EVS_Move_Spin = {
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
    Call(UseIdleAnimation, ACTOR_SELF, FALSE)
    Call(PlaySoundAtActor, ACTOR_SELF, SOUND_KOOPA_BROS_TOWER_SPIN_3)
    Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_ShellSpin)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(DefaultAnims))
    Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Stable)
    Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
    Call(UseIdleAnimation, ACTOR_SELF, TRUE)
    Return
    End
};


EvtScript EVS_HandlePhase = {
    Return
    End
};

}; // namespace koopa_gang_red

ActorBlueprint KoopaGangGreen = {
    .flags = ACTOR_FLAG_NO_DMG_POPUP | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_ATTACK,
    .maxHP = koopa_gang_green::hp,
    .type = ACTOR_TYPE_KOOPA_GANG,
    .level = ACTOR_LEVEL_KOOPA_GANG,
    .partCount = ARRAY_COUNT(koopa_gang_green::ActorParts),
    .partsData = koopa_gang_green::ActorParts,
    .initScript = &koopa_gang_green::EVS_Init,
    .statusTable = koopa_gang_green::StatusTable,
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

ActorBlueprint KoopaGangYellow = {
    .flags = ACTOR_FLAG_NO_DMG_POPUP | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_ATTACK,
    .maxHP = koopa_gang_yellow::hp,
    .type = ACTOR_TYPE_KOOPA_GANG,
    .level = ACTOR_LEVEL_KOOPA_GANG,
    .partCount = ARRAY_COUNT(koopa_gang_yellow::ActorParts),
    .partsData = koopa_gang_yellow::ActorParts,
    .initScript = &koopa_gang_yellow::EVS_Init,
    .statusTable = koopa_gang_yellow::StatusTable,
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

ActorBlueprint KoopaGangBlack = {
    .flags = ACTOR_FLAG_NO_DMG_POPUP | ACTOR_FLAG_NO_HEALTH_BAR | ACTOR_FLAG_SKIP_TURN | ACTOR_FLAG_NO_ATTACK,
    .maxHP = koopa_gang_black::hp,
    .type = ACTOR_TYPE_KOOPA_GANG,
    .level = ACTOR_LEVEL_KOOPA_GANG,
    .partCount = ARRAY_COUNT(koopa_gang_black::ActorParts),
    .partsData = koopa_gang_black::ActorParts,
    .initScript = &koopa_gang_black::EVS_Init,
    .statusTable = koopa_gang_black::StatusTable,
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

ActorBlueprint KoopaGangRed = {
    .flags = ACTOR_FLAG_NO_DMG_POPUP | ACTOR_FLAG_NO_HEALTH_BAR,
    .maxHP = koopa_gang_red::hp,
    .type = ACTOR_TYPE_KOOPA_GANG,
    .level = ACTOR_LEVEL_KOOPA_GANG,
    .partCount = ARRAY_COUNT(koopa_gang_red::ActorParts),
    .partsData = koopa_gang_red::ActorParts,
    .initScript = &koopa_gang_red::EVS_Init,
    .statusTable = koopa_gang_red::StatusTable,
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
