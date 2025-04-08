/* auto-generated, do not edit */
#include "star_rod_macros.h"

// type: MapProperties
#define GEN_MAP_LOCATION LOCATION_GOOMBA_ROAD

// type: EntryList
#define GEN_ENTRY_LIST \
    { -560.0,    0.0,    0.0,    0.0 },

// type: TexPanner
#define GEN_TEX_PANNER_1 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_1) \
    TEX_PAN_PARAMS_STEP(     0,   400,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     0,     1,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

// type: TexPanner
#define GEN_TEX_PANNER_2 \
    TEX_PAN_PARAMS_ID(TEX_PANNER_2) \
    TEX_PAN_PARAMS_STEP(   150,   150,     0,     0) \
    TEX_PAN_PARAMS_FREQ(     1,     1,     0,     0) \
    TEX_PAN_PARAMS_INIT(     0,     0,     0,     0)

// type: Marker:Entry
// name: Entry0
#define GEN_ENTRY0_X -560
#define GEN_ENTRY0_Y 0
#define GEN_ENTRY0_Z 0
#define GEN_ENTRY0_DIR 0
#define GEN_ENTRY0_VEC -560,0,0

// type: Marker:Entity:HiddenYellowBlock
// name: HiddenItemBlock1
#define GEN_HIDDEN_ITEM_BLOCK1_X -120
#define GEN_HIDDEN_ITEM_BLOCK1_Y 95
#define GEN_HIDDEN_ITEM_BLOCK1_Z -90
#define GEN_HIDDEN_ITEM_BLOCK1_DIR 0
#define GEN_HIDDEN_ITEM_BLOCK1_VEC -120,95,-90
#define GEN_HIDDEN_ITEM_BLOCK1_XYZA -120, 95, -90, 0
#define GEN_HIDDEN_ITEM_BLOCK1_ITEM ITEM_NONE
#define GEN_HIDDEN_ITEM_BLOCK1_PARAMS GEN_HIDDEN_ITEM_BLOCK1_XYZA, GEN_HIDDEN_ITEM_BLOCK1_ITEM

// type: Marker:Entity:Chest
// name: Chest
#define GEN_CHEST_X 124
#define GEN_CHEST_Y 0
#define GEN_CHEST_Z 55
#define GEN_CHEST_DIR -50
#define GEN_CHEST_VEC 124,0,55
#define GEN_CHEST_XYZA 124, 0, 55, -50
#define GEN_CHEST_PARAMS GEN_CHEST_XYZA

// type: Marker:NPC
// name: DockNPC
// anim: ANIM_RussT_Idle
#define GEN_DOCK_NPC_X -450
#define GEN_DOCK_NPC_Y 0
#define GEN_DOCK_NPC_Z 0
#define GEN_DOCK_NPC_DIR 0
#define GEN_DOCK_NPC_VEC -450,0,0
#define GEN_DOCK_NPC_TERRITORY \
{}

// type: Marker:NPC
// name: Enemy1
// anim: ANIM_SpearGuy_Anim03
#define GEN_ENEMY1_X 124
#define GEN_ENEMY1_Y 0
#define GEN_ENEMY1_Z 55
#define GEN_ENEMY1_DIR 0
#define GEN_ENEMY1_VEC 124,0,55
#define GEN_ENEMY1_TERRITORY \
{ \
    .wander = { \
        .centerPos   = { 124, 0, 55 }, \
        .wanderSize  = { 20 }, \
        .moveSpeedOverride = NO_OVERRIDE_MOVEMENT_SPEED, \
        .wanderShape = SHAPE_CYLINDER, \
        .detectPos   = { 124, 0, 55 }, \
        .detectSize  = { 100 }, \
        .detectShape = SHAPE_CYLINDER, \
        .isFlying = FALSE, \
    }, \
}

// type: Marker:Position
// name: Tree1
#define GEN_TREE1_X -4
#define GEN_TREE1_Y 0
#define GEN_TREE1_Z -17
#define GEN_TREE1_DIR 0
#define GEN_TREE1_VEC -4,0,-17

