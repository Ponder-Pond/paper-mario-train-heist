#ifndef _VARIABLES_H_
#define _VARIABLES_H_

#include "ultra64.h"
#include "common_structs.h"
#include "types.h"
#include "macros.h"
#include "enums.h"

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

extern PlayerStatus* gPlayerStatusPtr;
extern CollisionStatus gCollisionStatus;
extern GameStatus gGameStatus;
extern GameStatus* gGameStatusPtr;
extern u32 gRandSeed;
extern ItemData gItemTable[];
extern MoveData gMoveTable[];

extern s32 gBattleState;
extern BattleStatus gBattleStatus;
extern s32 gLastDrawBattleState;
extern s32 gDefeatedBattleSubstate;
extern s32 gBattleSubState;
extern s32 gDefeatedBattleState;
extern s32 gCurrentBattleID;
extern s32 gCurrentStageID;
extern struct Battle* gOverrideBattlePtr;

extern Camera gCameras[4];
extern s32 gCurrentCameraID;

extern CollisionData gCollisionData;

extern u8* gBackgroundTintModePtr;
extern s32 gEntityHideMode;

extern HiddenPanelsData gCurrentHiddenPanels;

extern BackgroundHeader gBackgroundImage;

extern CollisionData gZoneCollisionData;

// Animation related

extern PartnerAnimations gPartnerAnimations[12];

extern SpriteShadingProfile* gSpriteShadingProfile;

extern Window gWindows[64];

extern s32 gEncounterState;
extern s32 gOverrideFlags;

extern WindowStyle gWindowStyles[];

extern u16 gCurrentDoorSounds;
extern u16 gCurrentRoomDoorSounds;

extern UNK_FUN_PTR(TalkNotificationCallback);
extern UNK_FUN_PTR(InteractNotificationCallback);
extern UNK_FUN_PTR(ISpyNotificationCallback);
extern UNK_FUN_PTR(PulseStoneNotificationCallback);

extern Entity* TweesterTouchingPartner;
extern Entity* TweesterTouchingPlayer;

extern s32 PrevPlayerDirection;
extern s32 PeachDisguiseNpcIndex;
extern s32 PlayerRunStateTime;
extern s32 PrevPlayerCamRelativeYaw;
extern s32 D_800F7B44;
extern f32 PlayerNormalYaw;
extern f32 PlayerNormalPitch;
extern s32 NpcHitQueryColliderID;
extern Vec3s StandardActorHomePositions[];

extern s32 gEncounterSubState;
extern s32 gTimeFreezeMode;
extern b32 EncounterStateChanged;

extern u8 IntroMessageIdx;
extern s32 PartnerIDFromMenuIndex[12]; // partner IDs

// Scripts
extern EvtScript EVS_NpcDefeat;
extern EvtScript ShakeCam1;
extern EvtScript ShakeCamX;

extern MusicSettings gMusicSettings[2];

// gfx
extern DisplayContext* gDisplayContext;
extern Gfx* gMainGfxPos;
extern u16 gMatrixListPos;
extern s32 gCurrentDisplayContextIndex;

extern s16 gCurrentCamID;

extern s32 PartnerWishAnims[][5];

extern HeapNode heap_battleHead;

extern u32 bMarioIdleAnims[];
extern s32 bMarioDefendAnims[];
extern s32 bPeachIdleAnims[];

extern PartnerStatus gPartnerStatus;
extern StatusBar gStatusBar;
extern PlayerStatus gPlayerStatus;
extern PlayerSpinState gPlayerSpinState;
extern PlayerData gPlayerData;

#if VERSION_PAL
extern s32 gCurrentLanguage;
#else
#define gCurrentLanguage 0
#endif

#ifdef _LANGUAGE_C_PLUS_PLUS
} // extern "C"
#endif

#endif
