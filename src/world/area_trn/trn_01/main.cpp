#include "trn_01.h"

namespace trn_01 {

EntryList Entrances = { GEN_ENTRY_LIST };


#include "world/common/atomic/TexturePan.inc.c"

EvtScript EVS_TexPan = {
    Call(SetTexPanner, MODEL_Outside, TEX_PANNER_1)
    Call(EnableTexPanning, MODEL_Outside, TRUE)
    Thread
        GEN_TEX_PANNER_1
        Exec(N(EVS_UpdateTexturePan))
    EndThread
    Return
    End
};

API_CALLABLE((MapTransition)) {
    set_map_transition_effect(TRANSITION_BEGIN_OR_END_GAME);
    return ApiStatus_DONE2;
}

EvtScript EVS_EnterMap = {
    Call(GetEntryID, LVar0)
    Switch(LVar0)
        CaseEq(0)
            Call((MapTransition))
            Exec(EVS_Scene_BeginGame)
            Wait(5)
        CaseEq(1)
    EndSwitch
    Return
    End
};

EvtScript EVS_Main = {
    Set(GB_WorldLocation, GEN_MAP_LOCATION)
    Call(SetSpriteShading, SHADING_NONE)
    EVT_SETUP_CAMERA_DEFAULT(0, 0, 0)
    Call(MakeNpcs, TRUE, Ref(DefaultNPCs))
    Exec(EVS_EnterMap)
    Exec(EVS_TexPan)
    Exec(EVS_TrainBounce)
    Return
    End
};

EvtScript EVS_TrainBounce = {
    Label(1)
        Call(RandInt, 100, LVar9)
        Switch(LVar9)
            CaseLt(40)
                Wait(10)
            CaseDefault
                Call(ShakeCam, CAM_DEFAULT, 0, 10, Float(0.1))
                Wait(12.5)
                Call(ShakeCam, CAM_DEFAULT, 0, 10, Float(0.1))
                Wait(12.5)
        EndSwitch
        Wait(1)
    Goto(1)
    Return
    End
};

}; // namespace trn_01

MapSettings trn_01_settings = {
    .main = &trn_01::EVS_Main,
    .entryList = &trn_01::Entrances,
    .entryCount = ENTRY_COUNT(trn_01::Entrances),
    .tattle = { MSG_TrainLobby_MapTattle },
};
