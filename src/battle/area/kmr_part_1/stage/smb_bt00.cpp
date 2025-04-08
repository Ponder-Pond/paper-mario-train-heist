#include "../area.hpp"

#include "battle/battle.h"
#include "mapfs/smb_bt00_shape.h"

#define GEN_TEX_PANNER_0 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_0) \
    TEX_PAN_PARAMS_STEP(     0,   110,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     0,     1,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

#define GEN_TEX_PANNER_1 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_1) \
    TEX_PAN_PARAMS_STEP(  -160,     0,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     1,     0,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

namespace battle::area::kmr_part_1 {

namespace smb_00 {

#include "world/common/atomic/TexturePan.inc.c"

EvtScript EVS_PreBattle = {
    Call(SetSpriteShading, SHADING_NONE)
    Call(SetTexPanner, MODEL_Lava, TEX_PANNER_0)
    Thread
        GEN_TEX_PANNER_0
        Exec(N(EVS_UpdateTexturePan))
    EndThread
    Call(SetTexPanner, MODEL_LavaFlowBG, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(N(EVS_UpdateTexturePan))
    EndThread
    Call(SetTexPanner, MODEL_LavaFlowL, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(N(EVS_UpdateTexturePan))
    EndThread
    Call(SetTexPanner, MODEL_LavaFlowR, TEX_PANNER_1)
    Thread
        GEN_TEX_PANNER_1
        Exec(N(EVS_UpdateTexturePan))
    EndThread
    Return
    End
};

EvtScript EVS_PostBattle = {
    Return
    End
};

}; // namespace smb_00

Stage CastleBridge = {
    .texture = "smb_tex",
    .shape = "smb_bt00_shape",
    .hit = "smb_bt00_hit",
    .preBattle = &smb_00::EVS_PreBattle,
    .postBattle = &smb_00::EVS_PostBattle,
    // .bg = "net_bg",
};


}; // namespace battle::area::kmr_part_1
