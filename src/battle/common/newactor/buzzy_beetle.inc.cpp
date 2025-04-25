// #include "../area.h"
// #include "sprite/npc/BuzzyBeetle.h"
// #include "boss.h"

// #define NAMESPACE A(buzzy_beetle)

// extern s32 N(DefaultAnims)[];
// extern EvtScript N(EVS_Init);
// extern EvtScript N(EVS_Idle);
// extern EvtScript N(EVS_TakeTurn);
// extern EvtScript N(EVS_HandleEvent);

// enum N(ActorPartIDs) {
//     PRT_MAIN            = 1,
// };

// enum N(ActorVars) {
//     AVAR_ToppleState        = 3,
//     AVAL_State_Ground       = 1,
//     AVAL_State_Toppled      = 2,
//     AVAR_ToppleTurns        = 4,
//     AVAL_ToppleTurnZero     = 0,
//     AVAL_ToppleTurnOne      = 1,
// };

// enum N(ActorParams) {
//     DMG_SHELL_TOSS      = 2,
//     // DMG_CEILING_DROP    = 3,
// };

// s32 N(UprightDefense)[] = {
//     ELEMENT_NORMAL,   4,
//     ELEMENT_FIRE,    99,
//     ELEMENT_BLAST,   99,
//     ELEMENT_END,
// };

// s32 N(ToppledDefense)[] = {
//     ELEMENT_NORMAL,   2,
//     ELEMENT_END,
// };

// s32 N(StatusTable)[] = {
//     STATUS_KEY_NORMAL,              0,
//     STATUS_KEY_DEFAULT,             0,
//     STATUS_KEY_SLEEP,              90,
//     STATUS_KEY_POISON,             50,
//     STATUS_KEY_FROZEN,              0,
//     STATUS_KEY_DIZZY,              75,
//     STATUS_KEY_FEAR,                0,
//     STATUS_KEY_STATIC,             50,
//     STATUS_KEY_PARALYZE,           75,
//     STATUS_KEY_SHRINK,             90,
//     STATUS_KEY_STOP,               90,
//     STATUS_TURN_MOD_DEFAULT,        0,
//     STATUS_TURN_MOD_SLEEP,          0,
//     STATUS_TURN_MOD_POISON,         0,
//     STATUS_TURN_MOD_FROZEN,         0,
//     STATUS_TURN_MOD_DIZZY,          0,
//     STATUS_TURN_MOD_FEAR,           0,
//     STATUS_TURN_MOD_STATIC,         0,
//     STATUS_TURN_MOD_PARALYZE,       0,
//     STATUS_TURN_MOD_SHRINK,         0,
//     STATUS_TURN_MOD_STOP,           0,
//     STATUS_END,
// };

// ActorPartBlueprint N(ActorParts)[] = {
//     {
//         .flags = ACTOR_PART_FLAG_PRIMARY_TARGET,
//         .index = PRT_MAIN,
//         .posOffset = { 0, 0, 0 },
//         .targetOffset = { 0, 16 },
//         .opacity = 255,
//         .idleAnimations = N(DefaultAnims),
//         .defenseTable = N(UprightDefense),
//         .eventFlags = ACTOR_EVENT_FLAG_FLIPABLE,
//         .elementImmunityFlags = 0,
//         .projectileTargetOffset = { -1, -9 },
//     },
// };

// ActorBlueprint NAMESPACE = {
//     .flags = ACTOR_FLAG_NO_ATTACK,
//     .type = ACTOR_TYPE_BUZZY_BEETLE,
//     .level = ACTOR_LEVEL_BUZZY_BEETLE,
//     .maxHP = 5,
//     .partCount = ARRAY_COUNT(N(ActorParts)),
//     .partsData = N(ActorParts),
//     .initScript = &N(EVS_Init),
//     .statusTable = N(StatusTable),
//     .escapeChance = 60,
//     .airLiftChance = 75,
//     .hurricaneChance = 75,
//     .spookChance = 75,
//     .upAndAwayChance = 95,
//     .spinSmashReq = 0,
//     .powerBounceChance = 90,
//     .coinReward = 1,
//     .size = { 22, 18 },
//     .healthBarOffset = { 0, 0 },
//     .statusIconOffset = { -8, 16 },
//     .statusTextOffset = { 8, 13 },
// };

// s32 N(DefaultAnims)[] = {
//     STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Anim05,
//     STATUS_END,
// };

// s32 N(GroundAnims)[] = {
//     STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Anim01,
//     STATUS_KEY_STONE,     ANIM_BuzzyBeetle_Anim00,
//     STATUS_KEY_SLEEP,     ANIM_BuzzyBeetle_Anim09,
//     STATUS_KEY_POISON,    ANIM_BuzzyBeetle_Anim01,
//     STATUS_KEY_STOP,      ANIM_BuzzyBeetle_Anim00,
//     STATUS_KEY_STATIC,    ANIM_BuzzyBeetle_Anim01,
//     STATUS_KEY_PARALYZE,  ANIM_BuzzyBeetle_Anim00,
//     STATUS_KEY_DIZZY,     ANIM_BuzzyBeetle_Anim0A,
//     STATUS_KEY_FEAR,      ANIM_BuzzyBeetle_Anim0A,
//     STATUS_END,
// };

// s32 N(ToppledAnims)[] = {
//     STATUS_KEY_NORMAL,    ANIM_BuzzyBeetle_Anim02,
//     STATUS_KEY_STONE,     ANIM_BuzzyBeetle_Anim1A,
//     STATUS_KEY_SLEEP,     ANIM_BuzzyBeetle_Anim1B,
//     STATUS_KEY_POISON,    ANIM_BuzzyBeetle_Anim02,
//     STATUS_KEY_STOP,      ANIM_BuzzyBeetle_Anim1A,
//     STATUS_KEY_STATIC,    ANIM_BuzzyBeetle_Anim02,
//     STATUS_KEY_DIZZY,     ANIM_BuzzyBeetle_Anim1C,
//     STATUS_KEY_FEAR,      ANIM_BuzzyBeetle_Anim1C,
//     STATUS_END,
// };

// #include "common/battle/SetAbsoluteStatusOffsets.inc.c"

// EvtScript N(EVS_Init) = {
//     Call(BindIdle, ACTOR_SELF, Ref(N(EVS_Idle)))
//     Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Ground)
//     Call(BindTakeTurn, ACTOR_SELF, Ref(N(EVS_TakeTurn)))
//     Call(BindHandleEvent, ACTOR_SELF, Ref(N(EVS_HandleEvent)))
//     // Call(SetActorPos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
//     // Call(ForceHomePos, ACTOR_SELF, NPC_DISPOSE_LOCATION)
//     // Call(HPBarToHome, ACTOR_SELF)
//     // Call(SetPartFlagBits, ACTOR_SELF, PRT_MAIN, ACTOR_PART_FLAG_INVISIBLE | ACTOR_PART_FLAG_NO_TARGET, TRUE)
//     // Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_NO_ATTACK | ACTOR_FLAG_SKIP_TURN, TRUE)
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, TRUE)
//     Return
//     End
// };

// EvtScript N(EVS_Idle) = {
//     Return
//     End
// };

// // switch the anim on LVar1 if toppled
// EvtScript N(EVS_CheckToppleAnim) = {
//     Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar3)
//     IfEq(LVar3, AVAL_State_Toppled)
//         Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(1.0))
//         Set(LVar1, LVar2)
//     EndIf
//     Return
//     End
// };

// s32 N(FlipPosOffsets)[] = { 7, 13, 17, 21, 23, 24, 23, 21, 17, 13, 7, 0,  4,  7,  6,  4,  0,  2,  0 };

// EvtScript N(EVS_HandleEvent) = {
//     Call(UseIdleAnimation, ACTOR_SELF, FALSE)
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
//     Call(GetLastEvent, ACTOR_SELF, LVar0)
//     Switch(LVar0)
//         CaseOrEq(EVENT_HIT_COMBO)
//         CaseOrEq(EVENT_HIT)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
//             ExecWait(N(EVS_CheckToppleAnim))
//             ExecWait(EVS_Enemy_Hit)
//         EndCaseGroup
//         CaseEq(EVENT_BURN_HIT)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfNe(LVar0, AVAL_State_Toppled)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim0B)
//                 SetConst(LVar2, ANIM_BuzzyBeetle_Anim0B)
//                 ExecWait(EVS_Enemy_BurnHit)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim1D)
//                 SetConst(LVar2, ANIM_BuzzyBeetle_Anim1D)
//                 ExecWait(EVS_Enemy_BurnHit)
//             EndIf
//         CaseEq(EVENT_BURN_DEATH)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfNe(LVar0, AVAL_State_Toppled)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim0B)
//                 SetConst(LVar2, ANIM_BuzzyBeetle_Anim0B)
//                 ExecWait(EVS_Enemy_BurnHit)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim0B)
//                 ExecWait(EVS_Enemy_Death)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim1D)
//                 SetConst(LVar2, ANIM_BuzzyBeetle_Anim1D)
//                 ExecWait(EVS_Enemy_BurnHit)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim1D)
//                 ExecWait(EVS_Enemy_Death)
//             EndIf
//             Return
//         CaseEq(EVENT_SPIN_SMASH_HIT)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
//             ExecWait(N(EVS_CheckToppleAnim))
//             ExecWait(EVS_Enemy_SpinSmashHit)
//         CaseEq(EVENT_FLIP_TRIGGER)
//             Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(1.0))
//             Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Toppled)
//             Call(SetTargetOffset, ACTOR_SELF, PRT_MAIN, 0, 16)
//             Call(SetProjectileTargetOffset, ACTOR_SELF, PRT_MAIN, -1, -9)
//             Call(SetActorVar, ACTOR_SELF, AVAR_ToppleTurns, AVAL_ToppleTurnOne)
//             Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(N(ToppledDefense)))
//             Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(N(ToppledAnims)))
//             Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_FLIPPED, TRUE)
//             Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BuzzyBeetle_Anim07)
//             Call(SetActorRotationOffset, ACTOR_SELF, 0, 12, 0)
//             Thread
//                 Wait(1)
//                 Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
//                 Wait(1)
//                 Call(SetActorRotation, ACTOR_SELF, 0, 0, -45)
//                 Wait(1)
//                 Call(SetActorRotation, ACTOR_SELF, 0, 0, -90)
//                 Wait(1)
//                 Call(SetActorRotation, ACTOR_SELF, 0, 0, -135)
//                 Wait(1)
//                 Call(SetActorRotation, ACTOR_SELF, 0, 0, -180)
//                 Wait(1)
//             EndThread
//             UseBuf(Ref(N(FlipPosOffsets)))
//             Loop(19)
//                 BufRead1(LVar0)
//                 Call(SetActorDispOffset, ACTOR_SELF, 0, LVar0, 0)
//                 Wait(1)
//             EndLoop
//             Call(SetActorDispOffset, ACTOR_SELF, 0, 0, 0)
//             Call(SetActorRotationOffset, ACTOR_SELF, 0, 0, 0)
//             Call(SetActorRotation, ACTOR_SELF, 0, 0, 0)
//             Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BuzzyBeetle_Anim19)
//         CaseEq(EVENT_SHOCK_HIT)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             ExecWait(EVS_Enemy_ShockHit)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             ExecWait(EVS_Enemy_Knockback)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim04)
//             ExecWait(EVS_Enemy_ReturnHome)
//         CaseEq(EVENT_SHOCK_DEATH)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             ExecWait(EVS_Enemy_ShockHit)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             ExecWait(EVS_Enemy_Death)
//             Return
//         CaseEq(EVENT_ZERO_DAMAGE)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim0C)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//                 Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BuzzyBeetle_Anim17)
//                 Wait(8)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//             EndIf
//         CaseEq(EVENT_IMMUNE)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim0C)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//                 Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BuzzyBeetle_Anim0D)
//                 Wait(8)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//             EndIf
//         CaseEq(EVENT_SPIKE_TAUNT)
//             Wait(10)
//             Call(GetActorPos, ACTOR_SELF, LVar0, LVar1, LVar2)
//             Call(GetStatusFlags, ACTOR_SELF, LVar3)
//             IfFlag(LVar3, STATUS_FLAG_SHRINK)
//                 Add(LVar1, 9)
//             Else
//                 Add(LVar1, 24)
//             EndIf
//             PlayEffect(EFFECT_LENS_FLARE, 0, LVar0, LVar1, LVar2, 20, 0)
//             Wait(20)
//         CaseEq(EVENT_DEATH)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
//             ExecWait(N(EVS_CheckToppleAnim))
//             ExecWait(EVS_Enemy_Hit)
//             Wait(10)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
//             ExecWait(N(EVS_CheckToppleAnim))
//             ExecWait(EVS_Enemy_Death)
//             Return
//         CaseEq(EVENT_SPIN_SMASH_DEATH)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
//             ExecWait(N(EVS_CheckToppleAnim))
//             ExecWait(EVS_Enemy_SpinSmashHit)
//             SetConst(LVar0, PRT_MAIN)
//             SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             SetConst(LVar2, ANIM_BuzzyBeetle_Anim19)
//             ExecWait(N(EVS_CheckToppleAnim))
//             ExecWait(EVS_Enemy_Death)
//             Return
//         CaseEq(EVENT_RECOVER_STATUS)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim01)
//                 ExecWait(EVS_Enemy_Recover)
//             EndIf
//         CaseEq(EVENT_SCARE_AWAY)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim04)
//                 SetConst(LVar2, ANIM_BuzzyBeetle_Anim07)
//                 ExecWait(EVS_Enemy_ScareAway)
//                 Return
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//             EndIf
//         CaseEq(EVENT_BEGIN_AIR_LIFT)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim04)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
//             EndIf
//             ExecWait(EVS_Enemy_AirLift)
//         CaseEq(EVENT_BLOW_AWAY)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim07)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim08)
//             EndIf
//             ExecWait(EVS_Enemy_BlowAway)
//             Return
//         CaseEq(EVENT_AIR_LIFT_FAILED)
//             Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar0)
//             IfEq(LVar0, AVAL_State_Ground)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim0C)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//                 Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BuzzyBeetle_Anim0D)
//                 Wait(8)
//             Else
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
//                 ExecWait(EVS_Enemy_NoDamageHit)
//             EndIf
//         CaseDefault
//     EndSwitch
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, TRUE)
//     Return
//     End
// };

// EvtScript N(EVS_TakeTurn) = {
//     Call(UseIdleAnimation, ACTOR_SELF, FALSE)
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_DISABLE)
//     Call(GetActorVar, ACTOR_SELF, AVAR_ToppleState, LVar3)
//     IfEq(LVar3, AVAL_State_Toppled)
//         Call(GetActorVar, ACTOR_SELF, AVAR_ToppleTurns, LVar0)
//         Switch(LVar0)
//             CaseEq(AVAL_ToppleTurnOne)
//                 Call(SetActorVar, ACTOR_SELF, AVAR_ToppleTurns, AVAL_ToppleTurnZero)
//                 Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(1.0))
//                 Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
//                 Wait(30)
//                 Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
//                 Call(UseBattleCamPreset, BTL_CAM_DEFAULT)
//                 Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, FALSE)
//             CaseEq(AVAL_ToppleTurnZero)
//                 Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(1.0))
//                 Call(AddActorDecoration, ACTOR_SELF, PRT_MAIN, 0, ACTOR_DECORATION_SWEAT)
//                 Wait(20)
//                 Call(RemoveActorDecoration, ACTOR_SELF, PRT_MAIN, 0)
//                 SetConst(LVar0, PRT_MAIN)
//                 SetConst(LVar1, ANIM_BuzzyBeetle_Anim02)
//                 SetConst(LVar2, ANIM_BuzzyBeetle_Anim01)
//                 ExecWait(EVS_Enemy_FlipBackUp)
//                 Call(SetActorYaw, ACTOR_SELF, 0)
//                 Call(SetActorVar, ACTOR_SELF, AVAR_ToppleState, AVAL_State_Ground)
//                 Call(SetDefenseTable, ACTOR_SELF, PRT_MAIN, Ref(N(UprightDefense)))
//                 Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(N(GroundAnims)))
//                 Call(SetActorFlagBits, ACTOR_SELF, ACTOR_FLAG_FLIPPED, FALSE)
//                 Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, FALSE)
//         EndSwitch
//     Else
//         Call(SetActorVar, ACTOR_GREEN_BANDIT, AVAR_GreenPhase_CannonAttacks, TRUE)
//         Call(PlaySoundAtActor, ACTOR_SELF, SOUND_SHELL_SPIN)
//         Call(SetAnimationRate, ACTOR_SELF, PRT_MAIN, Float(4.0))
//         Call(SetAnimation, ACTOR_SELF, PRT_MAIN, ANIM_BuzzyBeetle_Anim05)
//         Call(SetIdleAnimations, ACTOR_SELF, PRT_MAIN, Ref(N(DefaultAnims)))
//     EndIf
//     Call(EnableIdleScript, ACTOR_SELF, IDLE_SCRIPT_ENABLE)
//     Call(UseIdleAnimation, ACTOR_SELF, TRUE)
//     Return
//     End
// };
