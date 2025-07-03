#include "common.h"
#include "effects.h"
#include "battle/battle.h"
#include "script_api/battle.h"
#include "mapfs/trn_bt00_shape.h"
#include "sprite/npc/KoopaGang2.h"
#include "boss.hpp"
#include "train_heist_actors.hpp"
#include "dx/debug_menu.h"

namespace battle::actor {

namespace koopa_gang {

extern s32 RedShellAnims[];
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
    PRT_MAIN               = 1, // PRT_TOWER
    PRT_GREEN              = 2,
    PRT_YELLOW             = 3,
    PRT_BLACK              = 4,
    PRT_RED                = 5,
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
        .idleAnimations = RedShellAnims,
        .defenseTable = DefaultDefense,
        .eventFlags = ACTOR_EVENT_FLAGS_NONE,
        .elementImmunityFlags = 0,
        .projectileTargetOffset = { 0, 0 },
    },
};

s32 RedAnims[] = {
    STATUS_KEY_NORMAL,     ANIM_KoopaGang2_Red_ShellSpinTower1,
    STATUS_END,
};

s32 RedTowerAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Red_Idle,
    STATUS_END,
};

s32 RedShellAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Red_ShellSpin,
    STATUS_END,
};

s32 BasicRedHurtAnims[] = {
    STATUS_KEY_NORMAL,    ANIM_KoopaGang_Red_Hurt,
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

// EvtScript EVS_UnstableGang = {
//     IfEq(LVar0, AVAL_KoopaGang_TowerState_Unstable)
//         Return
//     EndIf
//     Wait(5)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(N(RedTippingAnims)))
//     Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_IdleTipping)


//     Call(SetIdleAnimations, ACTOR_SELF, PRT_BLACK, Ref(N(BlackTippingAnims)))
//     Call(SetAnimation, ACTOR_SELF, PRT_BLACK, ANIM_KoopaGang_Black_IdleTipping)


//     Call(SetIdleAnimations, ACTOR_SELF, PRT_YELLOW, Ref(N(YellowTippingAnims)))
//     Call(SetAnimation, ACTOR_SELF, PRT_YELLOW, ANIM_KoopaGang_Yellow_IdleTipping)


//     Call(SetIdleAnimations, ACTOR_SELF, PRT_GREEN, Ref(N(GreenTippingAnims)))
//     Call(SetAnimation, ACTOR_SELF, PRT_GREEN, ANIM_KoopaGang_Green_IdleTipping)
//     Call(GetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, LVar0)

//     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_TowerState, AVAL_KoopaGang_TowerState_Unstable)
//     Return
//     End
// };

// EvtScript EVS_ToppleGang = {
//     Call(SetPartJumpGravity, ACTOR_SELF, PRT_MAIN, Float(1.6))
//     Call(GetPartPos, ACTOR_SELF, PRT_MAIN, LVar0, LVar1, LVar2)
//     Add(LVar0, 45)
//     Add(LVar1, -54)
//     Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
//     Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_KoopaGang_Red_IdleToppled)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(N(BasicRedToppledAnims)))
//     Call(JumpPartTo, ACTOR_SELF, PRT_MAIN, 10, FALSE, TRUE, FALSE)
//     Call(GetPartPos, ACTOR_SELF, PRT_MAIN, LVar0, LVar1, LVar2)
//     Call(SetPartPos, ACTOR_SELF, PRT_MAIN, LVar0, LVar1, LVar2)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(N(RedToppledAnims)))
//     Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(ToppledDefense))


//     Call(SetPartJumpGravity, ACTOR_SELF, PRT_BLACK, Float(1.6))
//     Add(LVar0, 30)
//     Add(LVar1, -36)
//     Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
//     Call(SetAnimation, ACTOR_SELF, PRT_BLACK, ANIM_KoopaGang_Black_IdleToppled)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_BLACK, Ref(N(BasicBlackToppledAnims)))
//     Call(JumpPartTo, ACTOR_SELF, PRT_BLACK, 10, FALSE, TRUE, FALSE)
//     Call(GetPartPos, ACTOR_SELF, PRT_BLACK, LVar0, LVar1, LVar2)
//     Call(SetPartPos, ACTOR_SELF, PRT_BLACK, LVar0, LVar1, LVar2)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_BLACK, Ref(N(BlackToppledAnims)))
//     Call(SetDefenseTable, ACTOR_SELF, PRT_BLACK, Ref(ToppledDefense))


//     Call(SetPartJumpGravity, ACTOR_SELF, PRT_YELLOW, Float(1.6))
//     Call(GetPartPos, ACTOR_SELF, PRT_YELLOW, LVar0, LVar1, LVar2)
//     Add(LVar0, 15)
//     Add(LVar1, -18)
//     Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
//     Call(SetAnimation, ACTOR_SELF, PRT_YELLOW, ANIM_KoopaGang_Yellow_IdleToppled)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_YELLOW, Ref(N(BasicYellowToppledAnims)))
//     Call(JumpPartTo, ACTOR_SELF, PRT_YELLOW, 10, FALSE, TRUE, FALSE)
//     // Call(JumpPartTo, ACTOR_SELF, PRT_MAIN, 5, FALSE, TRUE, FALSE)
//     Call(GetPartPos, ACTOR_SELF, PRT_YELLOW, LVar0, LVar1, LVar2)
//     Call(SetPartPos, ACTOR_SELF, PRT_YELLOW, LVar0, LVar1, LVar2)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_YELLOW, Ref(N(YellowToppledAnims)))
//     Call(SetDefenseTable, ACTOR_SELF, PRT_YELLOW, Ref(ToppledDefense))


//     Call(SetPartJumpGravity, ACTOR_SELF, PRT_GREEN, Float(1.6))
//     Call(GetPartPos, ACTOR_SELF, PRT_GREEN, LVar0, LVar1, LVar2)
//     Set(LVar0, 60)
//     Set(LVar1, 0)
//     Call(SetGoalPos, ACTOR_SELF, LVar0, LVar1, LVar2)
//     Call(SetAnimation, ACTOR_SELF, PRT_GREEN, ANIM_KoopaGang_Green_HurtStill)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_GREEN, Ref(N(BasicGreenHurtAnims)))
//     Call(JumpPartTo, ACTOR_SELF, PRT_GREEN, 20, FALSE, TRUE, FALSE)
//     Call(SetAnimation, ACTOR_SELF, PRT_GREEN, ANIM_KoopaGang_Green_IdleToppled)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_GREEN, Ref(N(BasicGreenToppledAnims)))
//     Call(JumpPartTo, ACTOR_SELF, PRT_GREEN, 10, FALSE, TRUE, FALSE)
//     // Call(JumpPartTo, ACTOR_SELF, PRT_GREEN, 5, FALSE, TRUE, FALSE)
//     Call(GetPartPos, ACTOR_SELF, PRT_GREEN, LVar0, LVar1, LVar2)
//     Call(SetPartPos, ACTOR_SELF, PRT_GREEN, LVar0, LVar1, LVar2)
//     Call(SetIdleAnimations, ACTOR_SELF, PRT_GREEN, Ref(N(GreenToppledAnims)))
//     Call(SetDefenseTable, ACTOR_SELF, PRT_GREEN, Ref(ToppledDefense))
//     IfEq(LFlag0, TRUE)
//         Call(N(StartRumbleWithParams), 256, 5)
//         Thread
//             Call(ShakeCam, CAM_BATTLE, 0, 5, Float(0.8))
//         EndThread
//     EndIf

//     Call(SetActorVar, ACTOR_BOWSER_THE_KID, AVAR_BowserPhase_KoopaGangSpitAttack, FALSE)
//     Call(SetActorVar, ACTOR_SELF, AVAR_KoopaGang_ToppleTurns, AVAL_KoopaGang_ToppleTurnOne)
//     Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_NO_TARGET, TRUE)
//     Return
//     End
// };

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
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(RedTowerAnims))
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
    Call((SpawnSpinEffect), LVar0, LVar1, LVar2, 60)
    Wait(60)
    Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(RedShellAnims))
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

}; // namespace koopa_gang

ActorBlueprint KoopaGang = {
    .flags = ACTOR_FLAG_NO_DMG_POPUP | ACTOR_FLAG_NO_HEALTH_BAR,
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
    .spinSmashReq = 0,
    .powerBounceChance = 70,
    .coinReward = 0,
    .size = { 92, 45 },
    .healthBarOffset = { 0, 0 },
    .statusIconOffset = { -10, 20 },
    .statusTextOffset = { 10, 20 },
};

}; // namespace battle::actor
