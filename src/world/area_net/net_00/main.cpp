#include "net_00.hpp"
#include "effects.h"
#include "dx/debug_menu.h"

namespace net_00 {

EntryList Entrances = { GEN_ENTRY_LIST };

#include "world/common/atomic/TexturePan.inc.c"

EvtScript EVS_TexPan = {
    SetGroup(EVT_GROUP_NEVER_PAUSE)
    Call(SetTexPanner, MODEL_bg, TEX_PANNER_C)
    Thread
        TEX_PAN_PARAMS_ID(TEX_PANNER_C)
        TEX_PAN_PARAMS_STEP(  -64,  -60,   32,   16)
        TEX_PAN_PARAMS_FREQ(    1,    1,    1,    1)
        TEX_PAN_PARAMS_INIT(    0,    0,    0,    0)
        Exec(EVS_UpdateTexturePan)
    EndThread
    Return
    End
};

#include "world/common/npc/StarSpirit.inc.c"

EvtScript EVS_Muskular_Appear = {
    Call(SetNpcPos, NPC_SELF, GEN_MUSKULAR_VEC)
    Call(PlaySoundAtNpc, NPC_SELF, SOUND_STAR_SPIRIT_APPEAR_A, SOUND_SPACE_DEFAULT)
    PlayEffect(EFFECT_SPARKLES, 0, GEN_MUSKULAR_VEC, 45, 0)
    Thread
        Call(MakeLerp, 0, 255, 80 * DT, EASING_LINEAR)
        Label(0)
            Call(UpdateLerp)
            Call(SetNpcImgFXParams, NPC_SELF, IMGFX_SET_ALPHA, LVar0, 0, 0, 0)
            Wait(1)
            IfEq(LVar1, 1)
                Goto(0)
            EndIf
    EndThread
    Thread
        Call(MakeLerp, 0, 2880, 80 * DT, EASING_QUADRATIC_OUT)
        Label(1)
            Call(UpdateLerp)
            Call(SetNpcRotation, NPC_SELF, 0, LVar0, 0)
            Wait(1)
            IfEq(LVar1, 1)
                Goto(1)
            EndIf
    EndThread
    Thread
        Call(MakeLerp, GEN_MUSKULAR_Y + 80, GEN_MUSKULAR_Y, 80 * DT, EASING_QUADRATIC_OUT)
        Label(2)
            Call(UpdateLerp)
            Call(SetNpcPos, NPC_SELF, GEN_MUSKULAR_X, LVar0, GEN_MUSKULAR_Z)
            Wait(1)
            IfEq(LVar1, 1)
                Goto(2)
            EndIf
    EndThread
    Wait(100 * DT)
    Return
    End
};

EvtScript EVS_Muskular_Depart = {
    Thread
        Loop(25)
            Call(GetNpcPos, NPC_SELF, LVar0, LVar1, LVar2)
            PlayEffect(EFFECT_SPARKLES, 4, LVar0, LVar1, LVar2, 20, 0, 0, 0, 0, 0, 0, 0, 0)
            Wait(4 * DT)
        EndLoop
    EndThread
    Thread
        Set(LVar2, 0)
        Set(LVar3, 1800)
        Call(MakeLerp, LVar2, LVar3, 100, EASING_CUBIC_IN)
        Loop(0)
            Call(UpdateLerp)
            Call(SetNpcRotation, NPC_SELF, 0, LVar0, 0)
            Wait(1)
            IfEq(LVar1, 0)
                BreakLoop
            EndIf
        EndLoop
    EndThread
    Thread
        Call(GetNpcPos, NPC_SELF, LVar2, LVar3, LVar4)
        Set(LVar5, LVar3)
        Add(LVar5, 180)
        Call(MakeLerp, LVar3, LVar5, 100 * DT, EASING_CUBIC_IN)
        Loop(0)
            Call(UpdateLerp)
            Call(SetNpcPos, NPC_SELF, LVar2, LVar0, LVar4)
            Wait(1)
            IfEq(LVar1, 0)
                BreakLoop
            EndIf
        EndLoop
        Call(SetNpcPos, NPC_SELF, NPC_DISPOSE_LOCATION)
    EndThread
    Thread
        Wait(15 * DT)
        Call(PlaySoundAtNpc, NPC_SELF, SOUND_STAR_SPIRIT_DEPART_1, SOUND_SPACE_DEFAULT)
    EndThread
    Wait(10 * DT)
    Return
    End
};

EvtScript EVS_NpcInit_Muskular = {
    Return // TEMP
    Thread
        ExecWait(EVS_Muskular_Appear)

        Call(FadeOutMusic, 0, 5000)

        WaitSecs(2)
        ExecWait(EVS_Muskular_Depart)

        WaitSecs(3)

        Call(GotoMapSpecial, Ref("mac_05"), 0, TRANSITION_SLOW_FADE_TO_WHITE)
    EndThread
    Return
    End
};

NpcData NpcData_Muskular = {
    .id = NPC_Muskular,
    .settings = &NpcSettings_StarSpirit,
    .pos = { NPC_DISPOSE_LOCATION },
    .flags = COMMON_PASSIVE_FLAGS | NPC_FLAG_IGNORE_CAMERA_FOR_YAW,
    .init = &EVS_NpcInit_Muskular,
    .yaw = 0,
    .drops = NO_DROPS,
    .animations = MUSKULAR_ANIMS,
};

NpcGroupList DefaultNPCs = {
    NPC_GROUP(N(NpcData_Muskular)),
    {},
};

EvtScript EVS_Main = {
    Call(SetSpriteShading, SHADING_NONE)
    EVT_SETUP_CAMERA_NO_LEAD(0, 0, 0)
    Call(SetMusicTrack, 0, SONG_SHOOTING_STAR_SUMMIT, 0, 8)
    Exec(EVS_TexPan)
    Call(DisablePlayerInput, TRUE)
    Call(DisablePlayerPhysics, TRUE)
    Call(MakeNpcs, TRUE, Ref(DefaultNPCs))
    Return
    End
};

}; // namespace net_00

MapSettings net_00_settings = {
    .main = &net_00::EVS_Main,
    .entryList = &net_00::Entrances,
    .entryCount = ENTRY_COUNT(net_00::Entrances),
    .background = &gBackgroundImage,
};

s32 net_00_map_init(void) {
    sprintf(wMapTexName, "hos_tex");
    return FALSE;
}
