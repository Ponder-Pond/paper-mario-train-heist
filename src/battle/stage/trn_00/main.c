#include "battle/battle.h"
#include "script_api/battle.h"
#include "stage.xml.h"

#define GEN_TEX_PANNER_0 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_0) \
    TEX_PAN_PARAMS_STEP(   291,     0,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     1,     0,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

#define GEN_TEX_PANNER_1 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_1) \
    TEX_PAN_PARAMS_STEP( -2000,     0,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     1,     0,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

#define GEN_TEX_PANNER_2 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_2) \
    TEX_PAN_PARAMS_STEP(   592,     0,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     1,     0,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

#define GEN_TEX_PANNER_3 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_3) \
    TEX_PAN_PARAMS_STEP( -2000,     0,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     1,     0,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)


EvtScript EVS_PreBattle = {
    Call(SetSpriteShading, SHADING_NONE)
    Call(SetCamBGColor, CAM_BATTLE, 0, 0, 0)
    Call(SetTexPanner, MODEL_Cactus, TEX_PANNER_0)
    Thread
        GEN_TEX_PANNER_0
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_MountainsShade, TEX_PANNER_0)
    Thread
        GEN_TEX_PANNER_0
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_GravelTop, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_GravelMiddle, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_GravelFront, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_TrainTracksTop, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_TrainTracksFront, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_BrushBig, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_Hill, TEX_PANNER_2)
    Thread
        GEN_TEX_PANNER_2
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(SetTexPanner, MODEL_BrushSmall, TEX_PANNER_3)
    Thread
        GEN_TEX_PANNER_3
        Exec(EVS_UpdateTexturePan)
    EndThread
    Call(EnableModel, MODEL_Tunnel, false)
    Call(EnableModel, MODEL_SnipingCrate, false)
    Call(EnableModel, MODEL_BarrelBlack, false)
    Call(EnableModel, MODEL_BombBox, false)
    Call(EnableModel, MODEL_BombPile, false)
    Call(EnableModel, MODEL_BarrelRed, false)
    Label(0)
        Loop(0)
            Add(LVar0, 28)
            IfGt(LVar0, 359)
                Sub(LVar0, 360)
            EndIf
            Call(RotateModel, MODEL_Wheel1, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel2, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel3, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel4, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel5, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel6, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel7, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel8, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel9, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel10, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel11, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel12, LVar0, 0, 0, 1)
            Call(RotateModel, MODEL_Wheel13, LVar0, 0, 0, 1)
            Wait(1)
        EndLoop
    Goto(0)
    Label(1)
        Call(RandInt, 100, LVar9)
        Switch(LVar9)
            CaseLt(80)
                WaitSecs(1)
            CaseDefault
                Call(ShakeCam, CAM_BATTLE, 0, 1, Float(0.5))
                WaitSecs(2)
        EndSwitch
        Wait(1)
    Goto(1)
    Return
    End
};

EvtScript EVS_PostBattle = {
    Return
    End
};

s32 ForegroundModels[] = {
    STAGE_MODEL_LIST_END
};

OVL_DEF_STAGE() = {
    .texture = "trn_tex",
    .bg = "trn_bg",
    .preBattle = &EVS_PreBattle,
    .postBattle = &EVS_PostBattle,
    .foregroundModelList = ForegroundModels,
};
