#include "sbk_13.h"

extern EvtScript N(EVS_Main);
extern NpcGroupList N(DefaultNPCs);

EntryList N(Entrances) = {
    [sbk_13_ENTRY_0]    { -475.0,    0.0,    0.0,   90.0 },
    [sbk_13_ENTRY_1]    {  475.0,    0.0,    0.0,  270.0 },
    [sbk_13_ENTRY_2]    {    0.0,    0.0, -475.0,  180.0 },
    [sbk_13_ENTRY_3]    {    0.0,    0.0,  475.0,    0.0 },
};

MapSettings N(settings) = {
    .main = &N(EVS_Main),
    .entryList = &N(Entrances),
    .entryCount = ENTRY_COUNT(N(Entrances)),
    .background = &gBackgroundImage,
    .tattle = { MSG_MapTattle_sbk_13 },
};

#include "world/common/todo/SpawnSunEffect.inc.c"

EvtScript N(EVS_ExitWalk_sbk_12_1) = EVT_EXIT_WALK(60, sbk_13_ENTRY_0, "sbk_12", sbk_12_ENTRY_1);
EvtScript N(EVS_ExitWalk_sbk_14_0) = EVT_EXIT_WALK(60, sbk_13_ENTRY_1, "sbk_14", sbk_14_ENTRY_0);
EvtScript N(EVS_ExitWalk_sbk_03_3) = EVT_EXIT_WALK(60, sbk_13_ENTRY_2, "sbk_03", sbk_03_ENTRY_3);
EvtScript N(EVS_ExitWalk_sbk_23_2) = EVT_EXIT_WALK(60, sbk_13_ENTRY_3, "sbk_23", sbk_23_ENTRY_2);

EvtScript N(EVS_BindExitTriggers) = {
    BindTrigger(Ref(N(EVS_ExitWalk_sbk_12_1)), TRIGGER_FLOOR_ABOVE, COLLIDER_deiliw, 1, 0)
    BindTrigger(Ref(N(EVS_ExitWalk_sbk_14_0)), TRIGGER_FLOOR_ABOVE, COLLIDER_deilie, 1, 0)
    BindTrigger(Ref(N(EVS_ExitWalk_sbk_03_3)), TRIGGER_FLOOR_ABOVE, COLLIDER_deilin, 1, 0)
    BindTrigger(Ref(N(EVS_ExitWalk_sbk_23_2)), TRIGGER_FLOOR_ABOVE, COLLIDER_deilis, 1, 0)
    Return
    End
};

API_CALLABLE(N(SetFog)) {
    enable_world_fog();
    set_world_fog_dist(985, 990);
    set_world_fog_color(1, 50, 75, 239);
    enable_entity_fog();
    set_entity_fog_dist(985, 990);
    set_entity_fog_color(1, 50, 75, 239);
    return ApiStatus_DONE2;
}

EvtScript N(EVS_Main) = {
    Set(GB_WorldLocation, LOCATION_DRY_DRY_DESERT)
    Call(SetSpriteShading, SHADING_NONE)
    IfEq(GB_StoryProgress, STORY_CH2_GOT_PULSE_STONE)
        Call(DisablePulseStone, FALSE)
    EndIf
    EVT_SETUP_CAMERA_NO_LEAD(0, 0, 0)
    //Call(MakeNpcs, FALSE, Ref(N(DefaultNPCs)))
    Call(N(SpawnSunEffect))
    //Call(SetMusicTrack, 0, SONG_DRY_DRY_DESERT, 0, 8)
    Call(PlayAmbientSounds, AMBIENT_SEA)
    Call(MakeNpcs, FALSE, Ref(N(DefaultNPCs)))
    Call(N(SpawnSunEffect))
    End
};
