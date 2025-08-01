#include "common.h"
#include "ld_addrs.h"
#include "model.h"
#include "effects.h"
#include "hud_element.h"
#include "model_clear_render_tasks.h"
#include "nu/nusys.h"
#include "qsort.h"
#include <string.h>

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

// models are rendered in two stages by the RDP:
// (1) main and aux textures are combined in the color combiner
// (2) the combined texture is blended with either tint or fog by the blender

// supported values for auxCombineType
enum {
    AUX_COMBINE_0               = 0,
    AUX_COMBINE_1               = 1,
    AUX_COMBINE_2               = 2,
    AUX_COMBINE_3               = 3,
    AUX_COMBINE_4               = 4,
    AUX_COMBINE_5               = 5,
    AUX_COMBINE_6               = 6,
    AUX_COMBINE_7               = 7,
    AUX_COMBINE_8               = 8,
    AUX_COMBINE_9               = 9,
    AUX_COMBINE_A               = 10,
};

// supported values for auxCombineSubType
// different subtypes are only supported for textures with EXTRA_TILE_NONE
enum {
    AUX_COMBINE_SUB_0           = 0, // multiply TEX * SHADE for color, use TEX for alpha
    AUX_COMBINE_SUB_1           = 1, // lerp from TEX to SHADE based on TEX alpha
    AUX_COMBINE_SUB_2           = 2, // TEX only, shade is ignored
    AUX_COMBINE_SUB_COUNT       = 3,
};

// different methods for combining main/aux texture maps
// derives from auxCombineType and auxCombineSubType
enum {
    // no texture
    TEX_COMBINE_NOTEX           = 0,

    // extra tile mode = EXTRA_TILE_NONE
    TEX_COMBINE_MAIN_ONLY_0     = 1,
    TEX_COMBINE_MAIN_ONLY_1     = 2,
    TEX_COMBINE_MAIN_ONLY_2     = 3,

    // extra tile mode = EXTRA_TILE_MIPMAPS
    TEX_COMBINE_MIPMAPS_0       = 4,
    TEX_COMBINE_MIPMAPS_1       = 5,
    TEX_COMBINE_MIPMAPS_2       = 6,

    // extra tile mode = EXTRA_TILE_AUX_SAME_AS_MAIN
    TEX_COMBINE_AUX_SHARED_0    = 7,
    TEX_COMBINE_AUX_SHARED_1    = 8,
    TEX_COMBINE_AUX_SHARED_2    = 9,

    // extra tile mode = EXTRA_TILE_AUX_INDEPENDENT
    // NOTE: unused; copy of TEX_COMBINE_AUX_SHARED; may not work properly
    TEX_COMBINE_AUX_IND_0       = 10,
    TEX_COMBINE_AUX_IND_1       = 11,
    TEX_COMBINE_AUX_IND_2       = 12,

    // special types selected by auxCombineType (these ignore auxCombineSubType)
    TEX_COMBINE_3               = 13,
    TEX_COMBINE_4               = 14,
    TEX_COMBINE_5               = 15,
    TEX_COMBINE_6               = 16,
    TEX_COMBINE_7               = 17,
    TEX_COMBINE_8               = 18,
    TEX_COMBINE_9               = 19,
    TEX_COMBINE_A               = 20,
};

enum {
    TINT_COMBINE_NONE           = 0,
    TINT_COMBINE_FOG            = 1,
    TINT_COMBINE_SHROUD         = 2,
    TINT_COMBINE_DEPTH          = 3,
    TINT_COMBINE_REMAP          = 4,
};

enum {
    RENDER_CLASS_1CYC           = 1, // render modes are single-cycle
    RENDER_CLASS_2CYC           = 2, // render modes are two-cycle, starting with G_RM_PASS
    RENDER_CLASS_FOG            = 3, // render modes are two-cycle, starting with G_RM_FOG_SHADE_A
    RENDER_CLASS_1CYC_SHROUD    = 4, // render modes use Gfx_RM2_SURFACE_OPA, but overwrite
    RENDER_CLASS_2CYC_SHROUD    = 5, // render modes use Gfx_RM2_SURFACE_OPA, but overwrite
    RENDER_CLASS_FOG_SHROUD     = 6,
    RENDER_CLASS_1CYC_DEPTH     = 10,
    RENDER_CLASS_2CYC_DEPTH     = 11,
};

enum {
    RENDER_TASK_LIST_NEAR, // dist < 800K
    RENDER_TASK_LIST_MID,
    RENDER_TASK_LIST_FAR, // dist >= 3M
};

#define WORLD_TEXTURE_MEMORY_SIZE 0x20000
#define BATTLE_TEXTURE_MEMORY_SIZE 0x8000

u8* gBackgroundTintModePtr; // NOTE: the type for this u8 is TintMode, as shown in SetModelTintMode
ModelList* gCurrentModels;
ModelTreeInfoList* gCurrentModelTreeNodeInfo;

extern Addr TextureHeap;

typedef struct FogSettings {
    /* 0x00 */ s32 enabled;
    /* 0x04 */ Color4i color;
    /* 0x14 */ s32 startDistance;
    /* 0x18 */ s32 endDistance;
} FogSettings; // size = 0x1C

extern Gfx Gfx_RM1_SURFACE_OPA[];
extern Gfx Gfx_RM1_DECAL_OPA[];
extern Gfx Gfx_RM1_INTERSECTING_OPA[];
extern Gfx Gfx_RM1_ALPHATEST[];
extern Gfx Gfx_RM1_SURFACE_XLU[];
extern Gfx Gfx_RM1_DECAL_XLU[];
extern Gfx Gfx_RM1_INTERSECTING_XLU[];
extern Gfx Gfx_RM1_SURFACE_OPA_NO_AA[];
extern Gfx Gfx_RM1_DECAL_OPA_NO_AA[];
extern Gfx Gfx_RM1_INTERSECTING_OPA_NO_AA[];
extern Gfx Gfx_RM1_ALPHATEST_ONESIDED[];
extern Gfx Gfx_RM1_SURFACE_XLU_NO_AA[];
extern Gfx Gfx_RM1_DECAL_XLU_NO_AA[];
extern Gfx Gfx_RM1_PASS_THROUGH[];
extern Gfx Gfx_RM1_SURFACE_XLU_AA_ZB_ZUPD[];
extern Gfx Gfx_RM1_SURFACE_OPA_NO_ZB[];
extern Gfx Gfx_RM1_ALPHATEST_NO_ZB[];
extern Gfx Gfx_RM1_SURFACE_XLU_NO_ZB[];
extern Gfx Gfx_RM1_SURFACE_XLU_ZB_ZUPD[];
extern Gfx Gfx_RM1_CLOUD_NO_ZCMP[];
extern Gfx Gfx_RM1_CLOUD[];
extern Gfx Gfx_RM1_CLOUD_NO_ZB[];
extern Gfx Gfx_RM2_SURFACE_OPA[];
extern Gfx Gfx_RM2_DECAL_OPA[];
extern Gfx Gfx_RM2_INTERSECTING_OPA[];
extern Gfx Gfx_RM2_ALPHATEST[];
extern Gfx Gfx_RM2_SURFACE_XLU[];
extern Gfx Gfx_RM2_DECAL_XLU[];
extern Gfx Gfx_RM2_INTERSECTING_XLU[];
extern Gfx Gfx_RM2_SURFACE_OPA_NO_AA[];
extern Gfx Gfx_RM2_DECAL_OPA_NO_AA[];
extern Gfx Gfx_RM2_INTERSECTING_OPA_NO_AA[];
extern Gfx Gfx_RM2_ALPHATEST_ONESIDED[];
extern Gfx Gfx_RM2_SURFACE_XLU_NO_AA[];
extern Gfx Gfx_RM2_DECAL_XLU_NO_AA[];
extern Gfx Gfx_RM2_PASS_THROUGH[];
extern Gfx Gfx_RM2_SURFACE_XLU_AA_ZB_ZUPD[];
extern Gfx Gfx_RM2_SURFACE_OPA_NO_ZB[];
extern Gfx Gfx_RM2_ALPHATEST_NO_ZB[];
extern Gfx Gfx_RM2_SURFACE_XLU_NO_ZB[];
extern Gfx Gfx_RM2_CLOUD[];
extern Gfx Gfx_RM2_CLOUD_NO_ZB[];
extern Gfx Gfx_RM3_SURFACE_OPA[];
extern Gfx Gfx_RM3_DECAL_OPA[];
extern Gfx Gfx_RM3_INTERSECTING_OPA[];
extern Gfx Gfx_RM3_ALPHATEST[];
extern Gfx Gfx_RM3_SURFACE_XLU[];
extern Gfx Gfx_RM3_DECAL_XLU[];
extern Gfx Gfx_RM3_INTERSECTING_XLU[];
extern Gfx Gfx_RM3_SURFACE_OPA_NO_AA[];
extern Gfx Gfx_RM3_DECAL_OPA_NO_AA[];
extern Gfx Gfx_RM3_INTERSECTING_OPA_NO_AA[];
extern Gfx Gfx_RM3_ALPHATEST_ONESIDED[];
extern Gfx Gfx_RM3_SURFACE_XLU_NO_AA[];
extern Gfx Gfx_RM3_DECAL_XLU_NO_AA[];
extern Gfx Gfx_RM3_PASS_THROUGH[];
extern Gfx Gfx_RM3_SURFACE_XLU_AA_ZB_ZUPD[];
extern Gfx Gfx_RM3_SURFACE_OPA_NO_ZB[];
extern Gfx Gfx_RM3_ALPHATEST_NO_ZB[];
extern Gfx Gfx_RM3_SURFACE_XLU_NO_ZB[];
extern Gfx Gfx_RM3_CLOUD[];
extern Gfx Gfx_RM3_CLOUD_NO_ZB[];

Gfx* ModelRenderModes[] = {
	[RENDER_MODE_IDX_00] Gfx_RM1_SURFACE_OPA,
	[RENDER_MODE_IDX_01] Gfx_RM1_SURFACE_OPA_NO_AA,
	[RENDER_MODE_IDX_02] Gfx_RM1_DECAL_OPA,
	[RENDER_MODE_IDX_03] Gfx_RM1_DECAL_OPA_NO_AA,
	[RENDER_MODE_IDX_04] Gfx_RM1_INTERSECTING_OPA,
	[RENDER_MODE_IDX_05] Gfx_RM1_INTERSECTING_OPA_NO_AA,
	[RENDER_MODE_IDX_06] Gfx_RM1_ALPHATEST,
	[RENDER_MODE_IDX_07] Gfx_RM1_ALPHATEST_ONESIDED,
	[RENDER_MODE_IDX_08] Gfx_RM1_SURFACE_XLU,
	[RENDER_MODE_IDX_09] Gfx_RM1_SURFACE_XLU_AA_ZB_ZUPD,
	[RENDER_MODE_IDX_0A] Gfx_RM1_SURFACE_XLU_NO_AA,
	[RENDER_MODE_IDX_0B] Gfx_RM1_SURFACE_XLU_ZB_ZUPD,
	[RENDER_MODE_IDX_0C] Gfx_RM1_DECAL_XLU,
	[RENDER_MODE_IDX_0D] Gfx_RM1_DECAL_XLU_NO_AA,
	[RENDER_MODE_IDX_0E] Gfx_RM1_INTERSECTING_XLU,
	[RENDER_MODE_IDX_0F] Gfx_RM1_PASS_THROUGH,
	[RENDER_MODE_IDX_10] Gfx_RM2_SURFACE_OPA,
	[RENDER_MODE_IDX_11] Gfx_RM2_SURFACE_OPA_NO_AA,
	[RENDER_MODE_IDX_12] Gfx_RM2_DECAL_OPA,
	[RENDER_MODE_IDX_13] Gfx_RM2_DECAL_OPA_NO_AA,
	[RENDER_MODE_IDX_14] Gfx_RM2_INTERSECTING_OPA,
	[RENDER_MODE_IDX_15] Gfx_RM2_INTERSECTING_OPA_NO_AA,
	[RENDER_MODE_IDX_16] Gfx_RM2_ALPHATEST,
	[RENDER_MODE_IDX_17] Gfx_RM2_ALPHATEST_ONESIDED,
	[RENDER_MODE_IDX_18] Gfx_RM2_SURFACE_XLU,
	[RENDER_MODE_IDX_19] Gfx_RM2_SURFACE_XLU_AA_ZB_ZUPD,
	[RENDER_MODE_IDX_1A] Gfx_RM2_SURFACE_XLU_NO_AA,
	[RENDER_MODE_IDX_1B] Gfx_RM2_DECAL_XLU,
	[RENDER_MODE_IDX_1C] Gfx_RM2_DECAL_XLU_NO_AA,
	[RENDER_MODE_IDX_1D] Gfx_RM2_INTERSECTING_XLU,
	[RENDER_MODE_IDX_1E] Gfx_RM2_PASS_THROUGH,
	[RENDER_MODE_IDX_1F] Gfx_RM3_SURFACE_OPA,
	[RENDER_MODE_IDX_20] Gfx_RM3_SURFACE_OPA_NO_AA,
	[RENDER_MODE_IDX_21] Gfx_RM3_DECAL_OPA,
	[RENDER_MODE_IDX_22] Gfx_RM3_DECAL_OPA_NO_AA,
	[RENDER_MODE_IDX_23] Gfx_RM3_INTERSECTING_OPA,
	[RENDER_MODE_IDX_24] Gfx_RM3_INTERSECTING_OPA_NO_AA,
	[RENDER_MODE_IDX_25] Gfx_RM3_ALPHATEST,
	[RENDER_MODE_IDX_26] Gfx_RM3_ALPHATEST_ONESIDED,
	[RENDER_MODE_IDX_27] Gfx_RM3_SURFACE_XLU,
	[RENDER_MODE_IDX_28] Gfx_RM3_SURFACE_XLU_AA_ZB_ZUPD,
	[RENDER_MODE_IDX_29] Gfx_RM3_SURFACE_XLU_NO_AA,
	[RENDER_MODE_IDX_2A] Gfx_RM3_DECAL_XLU,
	[RENDER_MODE_IDX_2B] Gfx_RM3_DECAL_XLU_NO_AA,
	[RENDER_MODE_IDX_2C] Gfx_RM3_INTERSECTING_XLU,
	[RENDER_MODE_IDX_2D] Gfx_RM3_PASS_THROUGH,
	[RENDER_MODE_IDX_2E] Gfx_RM1_SURFACE_OPA_NO_ZB,
	[RENDER_MODE_IDX_2F] Gfx_RM1_ALPHATEST_NO_ZB,
	[RENDER_MODE_IDX_30] Gfx_RM1_SURFACE_XLU_NO_ZB,
	[RENDER_MODE_IDX_31] Gfx_RM2_SURFACE_OPA_NO_ZB,
	[RENDER_MODE_IDX_32] Gfx_RM2_ALPHATEST_NO_ZB,
	[RENDER_MODE_IDX_33] Gfx_RM2_SURFACE_XLU_NO_ZB,
	[RENDER_MODE_IDX_34] Gfx_RM3_SURFACE_OPA_NO_ZB,
	[RENDER_MODE_IDX_35] Gfx_RM3_ALPHATEST_NO_ZB,
	[RENDER_MODE_IDX_36] Gfx_RM3_SURFACE_XLU_NO_ZB,
	[RENDER_MODE_IDX_37] Gfx_RM1_CLOUD,
	[RENDER_MODE_IDX_38] Gfx_RM1_CLOUD_NO_ZB,
	[RENDER_MODE_IDX_39] Gfx_RM2_CLOUD,
	[RENDER_MODE_IDX_3A] Gfx_RM2_CLOUD_NO_ZB,
	[RENDER_MODE_IDX_3B] Gfx_RM3_CLOUD,
	[RENDER_MODE_IDX_3C] Gfx_RM3_CLOUD_NO_ZB,
};

Gfx SolidCombineModes[][5] = {
    [TEX_COMBINE_NOTEX] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_SHADE, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_SHADE, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_TINT_DEPTH_NOTEX, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_TINT_REMAP_NOTEX, G_CC_PASS2),
    },

    [TEX_COMBINE_MAIN_ONLY_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_MODULATEIA),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_MODULATEIA, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_MODULATEIDECALA, PM_CC_20),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_MODULATEIA, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MAIN_ONLY_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_BLENDRGBA),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_BLENDRGBDECALA, PM_CC_TINT_DEPTH_NO_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_BLENDRGBA, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MAIN_ONLY_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_1A, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_DECALRGBA, PM_CC_TINT_REMAP_NO_SHADE),
    },

    // blend LODs in first cycle, tint in second cycle
    // all three sub-types are identical
    [TEX_COMBINE_MIPMAPS_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_TRILERP, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_DEPTH_MIPMAPS),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MIPMAPS_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_TRILERP, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_DEPTH_MIPMAPS),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MIPMAPS_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_TRILERP, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_DEPTH_MIPMAPS),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_REMAP_NO_SHADE),
    },

    // blend main/aux textures in first cycle, tint in second cycle
    // all three sub-types are identical
    [TEX_COMBINE_AUX_SHARED_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },
    [TEX_COMBINE_AUX_SHARED_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },
    [TEX_COMBINE_AUX_SHARED_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },

    // blend main/aux textures in first cycle, tint in second cycle
    // all three sub-types are identical
    [TEX_COMBINE_AUX_IND_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },
    [TEX_COMBINE_AUX_IND_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },
    [TEX_COMBINE_AUX_IND_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, G_CC_MODULATEIA2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },

    // shaded color multiplied main/aux textures for alpha
    [TEX_COMBINE_3] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3C),
    },
    // lerp between main/aux textures with shade alpha
    [TEX_COMBINE_4] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_22, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_5] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
    },
    [TEX_COMBINE_6] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_23, PM_CC_23),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_23, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_23, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_23, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_23, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_7] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
    },
    [TEX_COMBINE_8] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
    },
    [TEX_COMBINE_9] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
    },
    [TEX_COMBINE_A] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
    },
};

Gfx AlphaTestCombineModes[][5] = {
    [TEX_COMBINE_NOTEX] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_SHADE, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_SHADE, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_TINT_DEPTH_NOTEX, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_TINT_REMAP_NOTEX, G_CC_PASS2),
    },

    [TEX_COMBINE_MAIN_ONLY_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_MODULATEIDECALA, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_MODULATEIDECALA, PM_CC_20),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_MODULATEIDECALA, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MAIN_ONLY_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_BLENDRGBA),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_BLENDRGBA, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_BLENDRGBDECALA, PM_CC_TINT_DEPTH_NO_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_BLENDRGBA, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MAIN_ONLY_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_1A, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_DECALRGBA, PM_CC_TINT_REMAP_NO_SHADE),
    },

   // blend LODs in first cycle, tint in second cycle
    [TEX_COMBINE_MIPMAPS_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEI2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_TRILERP, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEI2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_DEPTH_MIPMAPS),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MIPMAPS_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_TRILERP, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_DEPTH_MIPMAPS),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_MIPMAPS_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_TRILERP, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(G_CC_TRILERP, G_CC_MODULATEIA2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_DEPTH_MIPMAPS),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(G_CC_TRILERP, PM_CC_TINT_REMAP_NO_SHADE),
    },

    // blend main/aux textures in first cycle, tint in second cycle
    [TEX_COMBINE_AUX_SHARED_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_SHADE_ALPHA),
    },
    [TEX_COMBINE_AUX_SHARED_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_AUX_SHARED_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_NO_SHADE),
    },

    // blend main/aux textures in first cycle, tint in second cycle
    [TEX_COMBINE_AUX_IND_0] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_AUX_IND_1] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_AUX_IND_2] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(G_CC_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC2_MULTIPLY_SHADE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_ALT_INTERFERENCE, PM_CC_TINT_REMAP_NO_SHADE),
    },

    [TEX_COMBINE_3] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3B),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_TEX_COMBINE_3A, PM_CC_TEX_COMBINE_3C),
    },
    [TEX_COMBINE_4] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_22, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_22, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_5] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC1_24, PM_CC2_24),
    },
    [TEX_COMBINE_6] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_23, PM_CC_23),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_23, G_CC_PASS2),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_23, G_CC_PASS2),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_23, G_CC_PASS2),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_23, PM_CC_TINT_REMAP_NO_SHADE),
    },
    [TEX_COMBINE_7] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC1_29, PM_CC2_29),
    },
    [TEX_COMBINE_8] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
    },
    [TEX_COMBINE_9] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
    },
    [TEX_COMBINE_A] {
        [TINT_COMBINE_NONE]     gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_FOG]      gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_SHROUD]   gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_DEPTH]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
        [TINT_COMBINE_REMAP]    gsDPSetCombineMode(PM_CC_NOISE, PM_CC_NOISE),
    },
};

void* TextureHeapBase = (void*) &TextureHeap;

u8 ShroudTintAmt = 0;
u8 ShroudTintR = 0;
u8 ShroudTintG = 0;
u8 ShroudTintB = 0;

u8 DepthTintBaseR = 0;
u8 DepthTintBaseG = 0;
u8 DepthTintBaseB = 0;
u8 DepthTintBaseA = 0;
u8 DepthTintColR = 0;
u8 DepthTintColG = 0;
u8 DepthTintColB = 0;
u8 DepthTintColA = 0; // unused?
s32 DepthTintStart = 950;
s32 DepthTintEnd = 1000;

u8 RemapTintMaxR = 255;
u8 RemapTintMaxG = 255;
u8 RemapTintMaxB = 255;
u8 RemapTintMinR = 0;
u8 RemapTintMinG = 0;
u8 RemapTintMinB = 0;

Mtx ReferenceIdentityMtx = RDP_MATRIX(
    1.000000, 0.000000, 0.000000, 0.000000,
    0.000000, 1.000000, 0.000000, 0.000000,
    0.000000, 0.000000, 1.000000, 0.000000,
    0.000000, 0.000000, 0.000000, 1.000000
);

// The depth buffer contains values encoded in a custom 18-bit floating-point format.
// There are 3 bits of exponent, 11 bits of mantissa, and 4 bits of "dz".
// However, two of the "dz" bits are inaccessible to the CPU because it can only access 8 of the 9
//   bits of each RDRAM byte (the N64 has 9-bit RAM).
// Therefore, the CPU sees it as a 16-bit value.

// Fields in floating point depth buffer format
#define DEPTH_EXPONENT_MASK 0xE000
#define DEPTH_MANTISSA_MASK 0x1FFC
#define DEPTH_DZ_MASK       0x0003

#define DEPTH_EXPONENT_SHIFT 13
#define DEPTH_MANTISSA_SHIFT 2
#define DEPTH_DZ_SHIFT       0

// Lookup table for converting depth buffer values to a 15.3 fixed-point format.
typedef struct DepthFloatFactors {
    /* 0x00 */ s32 shift;
    /* 0x04 */ s32 bias;
} DepthFloatFactors;

DepthFloatFactors DepthFloatLookupTable[] = {
    { 6, 0x00000 },
    { 5, 0x20000 },
    { 4, 0x30000 },
    { 3, 0x38000 },
    { 2, 0x3C000 },
    { 1, 0x3E000 },
    { 0, 0x3F000 },
    { 0, 0x3F800 },
    { 0, 0x00000 },
};

// Maximum depth value after the viewport transform.
// The multiplication by 2 comes from transforming depth from (-0.5, 0.5) to (0.0, 1.0).
// The multiplication by 32 comes from scaling the RSP does to increase depth precision.
#define MAX_VIEWPORT_DEPTH (2 * 32 * ((G_MAXZ / 2)))

s32 gLastRenderTaskCount = 0;

// ----------------------------------------------------------------------------
// RENDER_CLASS_1CYC, basic AA variants

// RENDER_MODE_IDX_00: RENDER_MODE_SURFACE_OPA, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_SURFACE_OPA[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD |
                          G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_02: RENDER_MODE_DECAL_OPA, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_DECAL_OPA[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_DECAL, G_RM_AA_ZB_OPA_DECAL2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD |
                          G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_04: RENDER_MODE_INTERSECTING_OPA, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_INTERSECTING_OPA[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_INTER, G_RM_AA_ZB_OPA_INTER2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD |
                          G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_06: RENDER_MODE_ALPHATEST, RENDER_CLASS_1CYC
// used by entity models and item entities
Gfx Gfx_RM1_ALPHATEST[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_08: RENDER_MODE_SURFACE_XLU, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_SURFACE_XLU[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_0C: RENDER_MODE_DECAL_XLU, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_DECAL_XLU[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_DECAL, G_RM_AA_ZB_XLU_DECAL2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_0E: RENDER_MODE_INTERSECTING_XLU, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_INTERSECTING_XLU[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_XLU_INTER, G_RM_AA_ZB_XLU_INTER2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_1CYC, basic NO_AA variants

// RENDER_MODE_IDX_01: RENDER_MODE_SURFACE_OPA_NO_AA, RENDER_CLASS_1CYC
Gfx Gfx_RM1_SURFACE_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_03: RENDER_MODE_DECAL_OPA_NO_AA, RENDER_CLASS_1CYC
Gfx Gfx_RM1_DECAL_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_ZB_OPA_DECAL, G_RM_ZB_OPA_DECAL2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_05: unused
Gfx Gfx_RM1_INTERSECTING_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_OPA_INTER, G_RM_AA_ZB_OPA_INTER2),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_07: RENDER_MODE_ALPHATEST_ONESIDED, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_ALPHATEST_ONESIDED[] = {
    gsDPSetRenderMode(G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_0A: RENDER_MODE_SURFACE_XLU_NO_AA, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_SURFACE_XLU_NO_AA[] = {
    gsDPSetRenderMode(G_RM_ZB_XLU_SURF, G_RM_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_0D: RENDER_MODE_DECAL_XLU_NO_AA, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_DECAL_XLU_NO_AA[] = {
    gsDPSetRenderMode(G_RM_ZB_OVL_SURF, G_RM_ZB_OVL_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_1CYC, special modes

// RENDER_MODE_IDX_0F: unused
// used by entity models for RENDER_MODE_PASS_THROUGH
Gfx Gfx_RM1_PASS_THROUGH[] = {
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_09: RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD, RENDER_CLASS_1CYC
Gfx Gfx_RM1_SURFACE_XLU_AA_ZB_ZUPD[] = {
    gsDPSetRenderMode(AA_EN | Z_CMP | Z_UPD | IM_RD | CLR_ON_CVG | CVG_DST_WRAP | ZMODE_XLU | FORCE_BL |
                      GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA), AA_EN | Z_CMP | Z_UPD | IM_RD |
                      CLR_ON_CVG | CVG_DST_WRAP | ZMODE_XLU | FORCE_BL |
                      GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_2E: RENDER_MODE_SURFACE_OPA_NO_ZB, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_SURFACE_OPA_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_2F: RENDER_MODE_ALPHATEST_NO_ZB, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_ALPHATEST_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_30: RENDER_MODE_SURFACE_XLU_NO_ZB, RENDER_CLASS_1CYC
// used by entity models
Gfx Gfx_RM1_SURFACE_XLU_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_0B: RENDER_MODE_SURFACE_XLU_ZB_ZUPD, RENDER_CLASS_1CYC
// custom render mode similar to RM_AA_XLU_SURF, using ZMODE_XLU instead of ZMODE_OPA and setting Z_CMP | Z_UPD
Gfx Gfx_RM1_SURFACE_XLU_ZB_ZUPD[] = {
    gsDPSetRenderMode(AA_EN | IM_RD | CVG_DST_WRAP | CLR_ON_CVG | FORCE_BL | Z_CMP | Z_UPD |
                      GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA),
                      AA_EN | IM_RD | CVG_DST_WRAP | CLR_ON_CVG | FORCE_BL | Z_CMP | Z_UPD  | ZMODE_XLU |
                      GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// no corresponding RENDER_MODE_IDX
// used by entity models for RENDER_MODE_CLOUD_NO_ZCMP
// custom render mode is identical to RM_ZB_CLD_SURF, except leaving out Z_CMP
Gfx Gfx_RM1_CLOUD_NO_ZCMP[] = {
    gsDPSetRenderMode(IM_RD | CVG_DST_SAVE | ZMODE_XLU | FORCE_BL |
                      GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA),
                      IM_RD | CVG_DST_SAVE | ZMODE_XLU | FORCE_BL |
                      GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_37: RENDER_MODE_CLOUD, RENDER_CLASS_1CYC
Gfx Gfx_RM1_CLOUD[] = {
    gsDPSetRenderMode(G_RM_ZB_CLD_SURF, G_RM_ZB_CLD_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_38: RENDER_MODE_CLOUD_NO_ZB, RENDER_CLASS_1CYC
Gfx Gfx_RM1_CLOUD_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_CLD_SURF, G_RM_CLD_SURF2),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_2CYC, basic AA variants

// RENDER_MODE_IDX_10: RENDER_MODE_SURFACE_OPA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_OPA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_12: RENDER_MODE_DECAL_OPA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_DECAL_OPA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_14: RENDER_MODE_INTERSECTING_OPA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_INTERSECTING_OPA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_INTER2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_16: RENDER_MODE_ALPHATEST, RENDER_CLASS_2CYC
// used by entity models, item entities with shading
Gfx Gfx_RM2_ALPHATEST[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_18: RENDER_MODE_SURFACE_XLU, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_XLU[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_1B: RENDER_MODE_DECAL_XLU, RENDER_CLASS_2CYC
Gfx Gfx_RM2_DECAL_XLU[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_1D: RENDER_MODE_INTERSECTING_XLU, RENDER_CLASS_2CYC
Gfx Gfx_RM2_INTERSECTING_XLU[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_INTER2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_2CYC, basic NO_AA variants

// RENDER_MODE_IDX_11: RENDER_MODE_SURFACE_OPA_NO_AA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_OPA_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_13: RENDER_MODE_DECAL_OPA_NO_AA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_DECAL_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_OPA_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_15: unused
Gfx Gfx_RM2_INTERSECTING_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_OPA_INTER2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_17: RENDER_MODE_ALPHATEST_ONESIDED, RENDER_CLASS_2CYC
Gfx Gfx_RM2_ALPHATEST_ONESIDED[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_1A: RENDER_MODE_SURFACE_XLU_NO_AA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_XLU_NO_AA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_1C: RENDER_MODE_DECAL_XLU_NO_AA, RENDER_CLASS_2CYC
Gfx Gfx_RM2_DECAL_XLU_NO_AA[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_XLU_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_2CYC, special modes

// RENDER_MODE_IDX_1E: unused
Gfx Gfx_RM2_PASS_THROUGH[] = {
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_19: RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_XLU_AA_ZB_ZUPD[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_31: RENDER_MODE_SURFACE_OPA_NO_ZB, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_OPA_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_OPA_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_32: RENDER_MODE_ALPHATEST_NO_ZB, RENDER_CLASS_2CYC
Gfx Gfx_RM2_ALPHATEST_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_33: RENDER_MODE_SURFACE_XLU_NO_ZB, RENDER_CLASS_2CYC
Gfx Gfx_RM2_SURFACE_XLU_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_39: RENDER_MODE_CLOUD, RENDER_CLASS_2CYC
Gfx Gfx_RM2_CLOUD[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_ZB_CLD_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_3A: RENDER_MODE_CLOUD_NO_ZB, RENDER_CLASS_2CYC
Gfx Gfx_RM2_CLOUD_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_PASS, G_RM_CLD_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_FOG+, basic AA variants

// RENDER_MODE_IDX_1F: RENDER_MODE_SURFACE_OPA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD / RENDER_CLASS_1CYC_SHROUD / RENDER_CLASS_1CYC_DEPTH
// used by entity models
Gfx Gfx_RM3_SURFACE_OPA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_21: RENDER_MODE_DECAL_OPA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD / RENDER_CLASS_1CYC_SHROUD / RENDER_CLASS_1CYC_DEPTH
// used by entity models
Gfx Gfx_RM3_DECAL_OPA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_23: RENDER_MODE_INTERSECTING_OPA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD / RENDER_CLASS_1CYC_SHROUD / RENDER_CLASS_1CYC_DEPTH
// used by entity models
Gfx Gfx_RM3_INTERSECTING_OPA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_INTER2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_25: RENDER_MODE_ALPHATEST, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD / RENDER_CLASS_1CYC_SHROUD / RENDER_CLASS_1CYC_DEPTH
// used by entity models
Gfx Gfx_RM3_ALPHATEST[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_27: RENDER_MODE_SURFACE_XLU, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
// used by entity models
Gfx Gfx_RM3_SURFACE_XLU[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_2A: RENDER_MODE_DECAL_XLU, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
// used by entity models
Gfx Gfx_RM3_DECAL_XLU[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_2C: RENDER_MODE_INTERSECTING_XLU, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
// used by entity models
Gfx Gfx_RM3_INTERSECTING_XLU[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_INTER2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_FOG+, basic NO_AA variants

//RENDER_MODE_IDX_20: RENDER_MODE_SURFACE_OPA_NO_AA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_SURFACE_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_OPA_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_22: RENDER_MODE_DECAL_OPA_NO_AA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_DECAL_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_OPA_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_24: unused
Gfx Gfx_RM3_INTERSECTING_OPA_NO_AA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_INTER2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_26: RENDER_MODE_ALPHATEST_ONESIDED, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_ALPHATEST_ONESIDED[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_29: RENDER_MODE_SURFACE_XLU_NO_AA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_SURFACE_XLU_NO_AA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_2B: RENDER_MODE_DECAL_XLU_NO_AA, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_DECAL_XLU_NO_AA[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_XLU_DECAL2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// ----------------------------------------------------------------------------
// RENDER_CLASS_FOG+, special modes

// RENDER_MODE_IDX_2D: unused
Gfx Gfx_RM3_PASS_THROUGH[] = {
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_28: RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_SURFACE_XLU_AA_ZB_ZUPD[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_34: RENDER_MODE_SURFACE_OPA_NO_ZB, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_SURFACE_OPA_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_OPA_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_CULL_BACK | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_35: RENDER_MODE_ALPHATEST_NO_ZB, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_ALPHATEST_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_TEX_EDGE2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_36: RENDER_MODE_SURFACE_XLU_NO_ZB, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD
Gfx Gfx_RM3_SURFACE_XLU_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_AA_XLU_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_FOG | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_3B: RENDER_MODE_CLOUD, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD / RENDER_CLASS_1CYC_SHROUD / RENDER_CLASS_1CYC_DEPTH
Gfx Gfx_RM3_CLOUD[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_ZB_CLD_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

// RENDER_MODE_IDX_3C: RENDER_MODE_CLOUD_NO_ZB, RENDER_CLASS_FOG / RENDER_CLASS_FOG_SHROUD / RENDER_CLASS_1CYC_SHROUD / RENDER_CLASS_1CYC_DEPTH
Gfx Gfx_RM3_CLOUD_NO_ZB[] = {
    gsDPSetRenderMode(G_RM_FOG_SHADE_A, G_RM_CLD_SURF2),
    gsDPSetCycleType(G_CYC_2CYCLE),
    gsSPClearGeometryMode(G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_LIGHTING | G_SHADING_SMOOTH),
    gsSPEndDisplayList(),
};

s32 RenderTaskBasePriorities[] = {
    [RENDER_MODE_SURF_SOLID_AA_ZB_LAYER0]   = -100000,
    [RENDER_MODE_SURFACE_OPA]               = 1000000,
    [RENDER_MODE_02_UNUSED]                 = 1000000,
    [RENDER_MODE_SURFACE_OPA_NO_AA]         = 1000000,
    [RENDER_MODE_SURFACE_OPA_NO_ZB]         =       0,
    [RENDER_MODE_DECAL_OPA]                 = 1000000,
    [RENDER_MODE_06_UNUSED]                 = 1000000,
    [RENDER_MODE_DECAL_OPA_NO_AA]           = 1000000,
    [RENDER_MODE_08_UNUSED]                 =       0,
    [RENDER_MODE_INTERSECTING_OPA]          = 1000000,
    [RENDER_MODE_0A_UNUSED]                 = 1000000,
    [RENDER_MODE_0B_UNUSED]                 = 1000000,
    [RENDER_MODE_0C_UNUSED]                 =       0,
    [RENDER_MODE_ALPHATEST]                 = 1000000,
    [RENDER_MODE_0E_UNUSED]                 = 1000000,
    [RENDER_MODE_ALPHATEST_ONESIDED]        = 1000000,
    [RENDER_MODE_ALPHATEST_NO_ZB]           =       0,
    [RENDER_MODE_SURFACE_XLU_LAYER1]        = 8000000,
    [RENDER_MODE_12_UNUSED]                 = 8000000,
    [RENDER_MODE_SURFACE_XLU_NO_AA]         = 8000000,
    [RENDER_MODE_SURFACE_XLU_NO_ZB]         =       0,
    [RENDER_MODE_SURFACE_XLU_ZB_ZUPD]       = 8000000,
    [RENDER_MODE_SURFACE_XLU_LAYER2]        = 7500000,
    [RENDER_MODE_17_UNUSED]                 = 7500000,
    [RENDER_MODE_18_UNUSED]                 = 7500000,
    [RENDER_MODE_19_UNUSED]                 =       0,
    [RENDER_MODE_DECAL_XLU]                 = 7000000,
    [RENDER_MODE_1B_UNUSED]                 = 7000000,
    [RENDER_MODE_DECAL_XLU_NO_AA]           = 7000000,
    [RENDER_MODE_1D_UNUSED]                 = 7000000,
    [RENDER_MODE_DECAL_XLU_AHEAD]           = 6500000,
    [RENDER_MODE_1F_UNUSED]                 = 6500000,
    [RENDER_MODE_SHADOW]                    = 6500000,
    [RENDER_MODE_21_UNUSED]                 =       0,
    [RENDER_MODE_SURFACE_XLU_LAYER3]        = 6000000,
    [RENDER_MODE_23_UNUSED]                 = 6000000,
    [RENDER_MODE_24_UNUSED]                 = 6000000,
    [RENDER_MODE_25_UNUSED]                 =       0,
    [RENDER_MODE_INTERSECTING_XLU]          = 5500000,
    [RENDER_MODE_27_UNUSED]                 = 5500000,
    [RENDER_MODE_PASS_THROUGH]              = 5500000,
    [RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD]    = 8000000,
    [RENDER_MODE_SURFACE_OPA_NO_ZB_BEHIND]  = 4000000,
    [RENDER_MODE_ALPHATEST_NO_ZB_BEHIND]    = 4250000,
    [RENDER_MODE_SURFACE_XLU_NO_ZB_BEHIND]  = 4500000,
    [RENDER_MODE_CLOUD_NO_ZCMP]             = 4500000,
    [RENDER_MODE_CLOUD]                     = 8000000,
    [RENDER_MODE_CLOUD_NO_ZB]               =  700000,
};

ModelCustomGfxBuilderList* gCurrentCustomModelGfxBuildersPtr;
ModelNode** gCurrentModelTreeRoot;
ModelTransformGroupList* gCurrentTransformGroups;
ModelCustomGfxList* gCurrentCustomModelGfxPtr;

BSS TextureHeader gCurrentTextureHeader ALIGNED(16);

BSS ModelList wModelList;
BSS ModelList bModelList;

BSS ModelTransformGroupList wTransformGroups;
BSS ModelTransformGroupList bTransformGroups;

BSS ModelCustomGfxList wCustomModelGfx;
BSS ModelCustomGfxList bCustomModelGfx;

BSS ModelCustomGfxBuilderList wCustomModelGfxBuilders;
BSS ModelCustomGfxBuilderList bCustomModelGfxBuilders;
BSS ModelLocalVertexCopyList wModelLocalVtxBuffers;
BSS ModelLocalVertexCopyList bModelLocalVtxBuffers;
BSS ModelLocalVertexCopyList* gCurrentModelLocalVtxBuffers;

BSS ModelNode* wModelTreeRoot;
BSS ModelNode* bModelTreeRoot;
BSS ModelTreeInfoList wModelTreeNodeInfo;
BSS ModelTreeInfoList bModelTreeNodeInfo;

BSS s8 wBackgroundTintMode;
BSS s8 bBackgroundTintMode;
BSS s32 TreeIterPos;
BSS FogSettings wFogSettings;
BSS FogSettings bFogSettings;
BSS FogSettings* gFogSettings;
BSS s32 texPannerMainU[MAX_TEX_PANNERS];
BSS s32 texPannerMainV[MAX_TEX_PANNERS];
BSS s32 texPannerAuxU[MAX_TEX_PANNERS];
BSS s32 texPannerAuxV[MAX_TEX_PANNERS];
BSS void* TextureHeapPos;
BSS u16 mtg_IterIdx;
BSS u16 mtg_SearchModelID;
BSS ModelNode* mtg_FoundModelNode;
BSS u16 mtg_MinChild;
BSS u16 mtg_MaxChild;
BSS u16 DepthCopyBuffer[16];
BSS RenderTask* RenderTaskLists[3];
BSS s32 RenderTaskListIdx;
BSS s32 RenderTaskCount[NUM_RENDER_TASK_LISTS];

TextureHandle TextureHandles[128];

extern Addr BattleEntityHeapBottom; // todo ???

void func_80117D00(Model* model);
void appendGfx_model_group(void* model);
void render_transform_group_node(ModelNode* node);
void render_transform_group(void* group);
void make_texture_gfx(TextureHeader*, Gfx**, IMG_PTR raster, PAL_PTR palette, IMG_PTR auxRaster, PAL_PTR auxPalette, u8, u8, u16, u16);
void load_model_transforms(ModelNode* model, ModelNode* parent, Matrix4f mdlTxMtx, s32 treeDepth);
s32 is_identity_fixed_mtx(Mtx* mtx);
void build_custom_gfx(void);

void appendGfx_model(void* data) {
    Model* model = data;
    s32 mtxPushMode;
    TextureHandle* textureHandle;
    TextureHeader* textureHeader;
    u32 extraTileType;
    s8 renderMode;
    s32 renderClass;
    s32 renderModeIdx;
    s32 flags = model->flags;

    ModelNode* modelNode;
    u16 customGfxIndex;
    s32 mtxLoadMode;
    s32 tintCombineType;
    ModelNodeProperty* prop;
    s32 temp;

    s32 fogMin, fogMax;
    s32 fogR, fogG, fogB, fogA;
    Gfx** gfxPos = &gMainGfxPos;

    mtxPushMode = G_MTX_PUSH;
    mtxLoadMode = G_MTX_LOAD;
    modelNode = model->modelNode;

    if (model->textureID != 0) {
        textureHandle = &TextureHandles[model->textureID + model->textureVariation];
        textureHeader = &textureHandle->header;

        if (textureHandle->gfx != NULL) {
            extraTileType = textureHandle->header.extraTiles;
        } else {
            textureHeader = NULL;
        }
    } else {
        textureHandle = NULL;
        textureHeader = NULL;
    }

    renderMode = model->renderMode;
    tintCombineType = 0;

    if (textureHeader != NULL) {
        switch (extraTileType) {
            case EXTRA_TILE_NONE:
                renderClass = RENDER_CLASS_1CYC;
                break;
            case EXTRA_TILE_MIPMAPS:
            case EXTRA_TILE_AUX_SAME_AS_MAIN:
            case EXTRA_TILE_AUX_INDEPENDENT:
                renderClass = RENDER_CLASS_2CYC;
                break;
            default:
                renderClass = RENDER_CLASS_1CYC;
                break;
        }
    } else {
        renderClass = RENDER_CLASS_1CYC;
    }

    if (textureHeader != NULL || renderMode <= RENDER_MODES_LAST_OPAQUE) {
        if (gFogSettings->enabled && !(flags & MODEL_FLAG_IGNORE_FOG)) {
            renderClass = RENDER_CLASS_FOG;
            tintCombineType = TINT_COMBINE_FOG;
        }
    }

    // if a model has a tint applied, set it up now
    switch ((u32)(model->customGfxIndex >> 4)) {
        case ENV_TINT_SHROUD:
            renderClass += (RENDER_CLASS_1CYC_SHROUD - RENDER_CLASS_1CYC);
            tintCombineType = TINT_COMBINE_SHROUD;
            break;
        case ENV_TINT_DEPTH:
            if (renderMode <= RENDER_MODES_LAST_OPAQUE) {
                gDPSetPrimColor((*gfxPos)++, 0, 0, DepthTintBaseR, DepthTintBaseG, DepthTintBaseB, DepthTintBaseA);
                gDPSetFogColor((*gfxPos)++, DepthTintColR, DepthTintColG, DepthTintColB, 0);
                gSPFogPosition((*gfxPos)++, DepthTintStart, DepthTintEnd);
                renderClass += (RENDER_CLASS_1CYC_DEPTH - RENDER_CLASS_1CYC);
                tintCombineType = TINT_COMBINE_DEPTH;
            }
            break;
        case ENV_TINT_REMAP:
            renderClass = RENDER_CLASS_2CYC;
            tintCombineType = TINT_COMBINE_REMAP;
            gDPSetPrimColor((*gfxPos)++, 0, 0, RemapTintMaxR, RemapTintMaxG, RemapTintMaxB, 255);
            gDPSetEnvColor((*gfxPos)++, RemapTintMinR, RemapTintMinG, RemapTintMinB, 255);
            break;
    }

    gDPPipeSync((*gfxPos)++);

    if (model->groupData != NULL) {
        Lightsn* lights = model->groupData->lightingGroup;
        if (model->groupData->lightingGroup != NULL) {
            switch (model->groupData->numLights) {
                case 0:
                    gSPSetLights0((*gfxPos)++, lights[0]);
                    break;
                case 1:
                    gSPSetLights1((*gfxPos)++, lights[0]);
                    break;
                case 2:
                    gSPSetLights2((*gfxPos)++, lights[0]);
                    break;
                case 3:
                    gSPSetLights3((*gfxPos)++, lights[0]);
                    break;
                case 4:
                    gSPSetLights4((*gfxPos)++, lights[0]);
                    break;
                case 5:
                    gSPSetLights5((*gfxPos)++, lights[0]);
                    break;
                case 6:
                    gSPSetLights6((*gfxPos)++, lights[0]);
                    break;
                case 7:
                    gSPSetLights7((*gfxPos)++, lights[0]);
                    break;
            }
        }
    }

    if (textureHeader != NULL) {
        switch (extraTileType) {
            case EXTRA_TILE_AUX_INDEPENDENT:
            case EXTRA_TILE_4:
                prop = get_model_property(modelNode, MODEL_PROP_KEY_SPECIAL);
                if (prop != NULL) {
                    s32 shift = prop->data.s;
                    u16 offsetS = prop->dataType;
                    s32 offsetT = prop->dataType;
                    make_texture_gfx(textureHeader, gfxPos,
                        textureHandle->raster, textureHandle->palette,
                        textureHandle->auxRaster, textureHandle->auxPalette,
                        (shift >> 12) & 0xF, (shift >> 16) & 0xF,
                        offsetS & 0xFFF, (offsetT >> 12) & 0xFFF);

                } else {
                    gSPDisplayList((*gfxPos)++, textureHandle->gfx);
                }
                break;
            default:
                gSPDisplayList((*gfxPos)++, textureHandle->gfx);
                break;
        }
    } else {
        gSPTexture((*gfxPos)++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
        gDPSetCombineMode((*gfxPos)++, G_CC_SHADE, G_CC_SHADE);
        gDPSetColorDither((*gfxPos)++, G_CD_MAGICSQ);
        gDPSetAlphaDither((*gfxPos)++, G_AD_PATTERN);
    }

    // setup combine modes for main/aux texture blending when fog or tint is enabled
    if (tintCombineType != TINT_COMBINE_NONE
        || renderMode == RENDER_MODE_ALPHATEST
        || renderMode == RENDER_MODE_ALPHATEST_ONESIDED
    ) {
        u32 texCombineType = TEX_COMBINE_NOTEX;

        // only the following aux combine modes are ever used:
        // (A) 0x00 -> 0, 0
        // (B) 0x08 -> 2, 0
        // (C) 0x0D -> 3, 1
        // (D) 0x10 -> 4, 0
        if (textureHeader != NULL) {
            u32 auxCombineType = textureHeader->auxCombineType;
            if (auxCombineType >= 3) {
                // combine modes 3, 4, ... are directly appended to the end of the table and subtype is ignored
                texCombineType = TEX_COMBINE_3 + (auxCombineType - 3);
            } else {
                // select based on aux combine subtypes
                // in practice, auxCombineSubType is ALWAYS zero here since the only (A) and (B) may reach this block
                texCombineType = 1 + extraTileType * AUX_COMBINE_SUB_COUNT + textureHeader->auxCombineSubType;
            }
        }

        if (!(renderMode == RENDER_MODE_ALPHATEST || renderMode == RENDER_MODE_ALPHATEST_ONESIDED)) {
            *(*gfxPos) = SolidCombineModes[texCombineType][tintCombineType];
        } else {
            *(*gfxPos) = AlphaTestCombineModes[texCombineType][tintCombineType];
        }
        (*gfxPos)++;
    }

    // setup geometry modes and render modes
    switch (renderClass) {
        case RENDER_CLASS_1CYC:
            switch (renderMode) {
                case RENDER_MODE_SURFACE_OPA:
                    renderModeIdx = RENDER_MODE_IDX_00;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_01;
                    break;
                case RENDER_MODE_DECAL_OPA:
                    renderModeIdx = RENDER_MODE_IDX_02;
                    break;
                case RENDER_MODE_DECAL_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_03;
                    break;
                case RENDER_MODE_INTERSECTING_OPA:
                    renderModeIdx = RENDER_MODE_IDX_04;
                    break;
                case RENDER_MODE_ALPHATEST:
                    renderModeIdx = RENDER_MODE_IDX_06;
                    break;
                case RENDER_MODE_ALPHATEST_ONESIDED:
                    renderModeIdx = RENDER_MODE_IDX_07;
                    break;
                case RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD:
                    renderModeIdx = RENDER_MODE_IDX_09;
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER1:
                case RENDER_MODE_SURFACE_XLU_LAYER2:
                case RENDER_MODE_SURFACE_XLU_LAYER3:
                    renderModeIdx = RENDER_MODE_IDX_08;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_0A;
                    break;
                case RENDER_MODE_SURFACE_XLU_ZB_ZUPD:
                    renderModeIdx = RENDER_MODE_IDX_0B;
                    break;
                case RENDER_MODE_DECAL_XLU:
                    renderModeIdx = RENDER_MODE_IDX_0C;
                    break;
                case RENDER_MODE_DECAL_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_0D;
                    break;
                case RENDER_MODE_INTERSECTING_XLU:
                    renderModeIdx = RENDER_MODE_IDX_0E;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_2E;
                    break;
                case RENDER_MODE_ALPHATEST_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_2F;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_30;
                    break;
                case RENDER_MODE_CLOUD:
                    renderModeIdx = RENDER_MODE_IDX_37;
                    break;
                case RENDER_MODE_CLOUD_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_38;
                    break;
                default:
                    renderModeIdx = RENDER_MODE_IDX_00;
                    break;
            }
            gSPDisplayList((*gfxPos)++, ModelRenderModes[renderModeIdx]);
            break;
        case RENDER_CLASS_2CYC:
            switch (renderMode) {
                case RENDER_MODE_SURFACE_OPA:
                    renderModeIdx = RENDER_MODE_IDX_10;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_11;
                    break;
                case RENDER_MODE_DECAL_OPA:
                    renderModeIdx = RENDER_MODE_IDX_12;
                    break;
                case RENDER_MODE_DECAL_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_13;
                    break;
                case RENDER_MODE_INTERSECTING_OPA:
                    renderModeIdx = RENDER_MODE_IDX_14;
                    break;
                case RENDER_MODE_ALPHATEST:
                    renderModeIdx = RENDER_MODE_IDX_16;
                    break;
                case RENDER_MODE_ALPHATEST_ONESIDED:
                    renderModeIdx = RENDER_MODE_IDX_17;
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER1:
                case RENDER_MODE_SURFACE_XLU_LAYER2:
                case RENDER_MODE_SURFACE_XLU_LAYER3:
                    renderModeIdx = RENDER_MODE_IDX_18;
                    break;
                case RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD:
                    renderModeIdx = RENDER_MODE_IDX_19;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_1A;
                    break;
                case RENDER_MODE_DECAL_XLU:
                    renderModeIdx = RENDER_MODE_IDX_1B;
                    break;
                case RENDER_MODE_DECAL_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_1C;
                    break;
                case RENDER_MODE_INTERSECTING_XLU:
                    renderModeIdx = RENDER_MODE_IDX_1D;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_31;
                    break;
                case RENDER_MODE_ALPHATEST_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_32;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_33;
                    break;
                case RENDER_MODE_CLOUD:
                    renderModeIdx = RENDER_MODE_IDX_39;
                    break;
                case RENDER_MODE_CLOUD_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_3A;
                    break;
                default:
                    renderModeIdx = RENDER_MODE_IDX_10;
                    break;
            }
            gSPDisplayList((*gfxPos)++, ModelRenderModes[renderModeIdx]);
            break;
        case RENDER_CLASS_FOG:
            switch (renderMode) {
                case RENDER_MODE_SURFACE_OPA:
                    renderModeIdx = RENDER_MODE_IDX_1F;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_20;
                    break;
                case RENDER_MODE_DECAL_OPA:
                    renderModeIdx = RENDER_MODE_IDX_21;
                    break;
                case RENDER_MODE_DECAL_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_22;
                    break;
                case RENDER_MODE_INTERSECTING_OPA:
                    renderModeIdx = RENDER_MODE_IDX_23;
                    break;
                case RENDER_MODE_ALPHATEST:
                    renderModeIdx = RENDER_MODE_IDX_25;
                    break;
                case RENDER_MODE_ALPHATEST_ONESIDED:
                    renderModeIdx = RENDER_MODE_IDX_26;
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER1:
                case RENDER_MODE_SURFACE_XLU_LAYER2:
                case RENDER_MODE_SURFACE_XLU_LAYER3:
                    renderModeIdx = RENDER_MODE_IDX_27;
                    break;
                case RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD:
                    renderModeIdx = RENDER_MODE_IDX_28;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_29;
                    break;
                case RENDER_MODE_DECAL_XLU:
                    renderModeIdx = RENDER_MODE_IDX_2A;
                    break;
                case RENDER_MODE_DECAL_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_2B;
                    break;
                case RENDER_MODE_INTERSECTING_XLU:
                    renderModeIdx = RENDER_MODE_IDX_2C;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_34;
                    break;
                case RENDER_MODE_ALPHATEST_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_35;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_36;
                    break;
                case RENDER_MODE_CLOUD:
                    renderModeIdx = RENDER_MODE_IDX_3B;
                    break;
                case RENDER_MODE_CLOUD_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_3C;
                    break;
                default:
                    renderModeIdx = RENDER_MODE_IDX_1F;
                    break;
            }
            gSPDisplayList((*gfxPos)++, ModelRenderModes[renderModeIdx]);
            gDPSetFogColor((*gfxPos)++, gFogSettings->color.r, gFogSettings->color.g, gFogSettings->color.b, gFogSettings->color.a);
            gSPFogPosition((*gfxPos)++, gFogSettings->startDistance, gFogSettings->endDistance);
            break;
        case RENDER_CLASS_1CYC_SHROUD:
        case RENDER_CLASS_2CYC_SHROUD:
            if (ShroudTintAmt == 255) {
                return;
            }
            gSPDisplayList((*gfxPos)++, ModelRenderModes[RENDER_MODE_IDX_10]);
            switch (renderMode) {
                case RENDER_MODE_SURFACE_OPA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_OPA_SURF2);
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_AA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_ZB_OPA_SURF2);
                    break;
                case RENDER_MODE_DECAL_OPA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_OPA_DECAL2);
                    break;
                case RENDER_MODE_DECAL_OPA_NO_AA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_ZB_OPA_DECAL2);
                    break;
                case RENDER_MODE_INTERSECTING_OPA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_OPA_INTER2);
                    break;
                case RENDER_MODE_ALPHATEST:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_TEX_EDGE2);
                    break;
                case RENDER_MODE_ALPHATEST_ONESIDED:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_TEX_EDGE2);
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER1:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_XLU_SURF2);
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER2:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_XLU_SURF2);
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER3:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_XLU_SURF2);
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_AA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_ZB_XLU_SURF2);
                    break;
                case RENDER_MODE_DECAL_XLU:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_XLU_DECAL2);
                    break;
                case RENDER_MODE_DECAL_XLU_NO_AA:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_XLU_DECAL2);
                    break;
                case RENDER_MODE_INTERSECTING_XLU:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_ZB_XLU_INTER2);
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_ZB:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_OPA_SURF2);
                    break;
                case RENDER_MODE_ALPHATEST_NO_ZB:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_TEX_EDGE2);
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_ZB:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_AA_XLU_SURF2);
                    break;
                case RENDER_MODE_CLOUD:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_ZB_CLD_SURF2);
                    break;
                case RENDER_MODE_CLOUD_NO_ZB:
                    gDPSetRenderMode(gMainGfxPos++, PM_RM_SHROUD, G_RM_CLD_SURF2);
                    break;
            }
            gDPSetFogColor((*gfxPos)++, gFogSettings->color.r, gFogSettings->color.g, gFogSettings->color.b, ShroudTintAmt);
            gDPSetBlendColor((*gfxPos)++, ShroudTintR, ShroudTintG, ShroudTintB, 255);
            gSPFogPosition((*gfxPos)++, 970, 1000);
            break;
        case RENDER_CLASS_FOG_SHROUD:
            switch (renderMode) {
                case RENDER_MODE_SURFACE_OPA:
                    renderModeIdx = RENDER_MODE_IDX_1F;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_20;
                    break;
                case RENDER_MODE_DECAL_OPA:
                    renderModeIdx = RENDER_MODE_IDX_21;
                    break;
                case RENDER_MODE_DECAL_OPA_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_22;
                    break;
                case RENDER_MODE_INTERSECTING_OPA:
                    renderModeIdx = RENDER_MODE_IDX_23;
                    break;
                case RENDER_MODE_ALPHATEST:
                    renderModeIdx = RENDER_MODE_IDX_25;
                    break;
                case RENDER_MODE_ALPHATEST_ONESIDED:
                    renderModeIdx = RENDER_MODE_IDX_26;
                    break;
                case RENDER_MODE_SURFACE_XLU_LAYER1:
                case RENDER_MODE_SURFACE_XLU_LAYER2:
                case RENDER_MODE_SURFACE_XLU_LAYER3:
                    renderModeIdx = RENDER_MODE_IDX_27;
                    break;
                case RENDER_MODE_SURFACE_XLU_AA_ZB_ZUPD:
                    renderModeIdx = RENDER_MODE_IDX_28;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_29;
                    break;
                case RENDER_MODE_DECAL_XLU:
                    renderModeIdx = RENDER_MODE_IDX_2A;
                    break;
                case RENDER_MODE_DECAL_XLU_NO_AA:
                    renderModeIdx = RENDER_MODE_IDX_2B;
                    break;
                case RENDER_MODE_INTERSECTING_XLU:
                    renderModeIdx = RENDER_MODE_IDX_2C;
                    break;
                case RENDER_MODE_SURFACE_OPA_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_34;
                    break;
                case RENDER_MODE_ALPHATEST_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_35;
                    break;
                case RENDER_MODE_SURFACE_XLU_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_36;
                    break;
                case RENDER_MODE_CLOUD:
                    renderModeIdx = RENDER_MODE_IDX_3B;
                    break;
                case RENDER_MODE_CLOUD_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_3C;
                    break;
                default:
                    renderModeIdx = RENDER_MODE_IDX_1F;
                    break;
            }
            gSPDisplayList((*gfxPos)++, ModelRenderModes[renderModeIdx]);

            // lerp between scene fog and shroud fog based on ShroudTintAmt
            fogR = (gFogSettings->color.r * (255 - ShroudTintAmt) + ShroudTintR * ShroudTintAmt) / 255;
            fogG = (gFogSettings->color.g * (255 - ShroudTintAmt) + ShroudTintG * ShroudTintAmt) / 255;
            fogB = (gFogSettings->color.b * (255 - ShroudTintAmt) + ShroudTintB * ShroudTintAmt) / 255;

            fogMin = (gFogSettings->startDistance * (255 - ShroudTintAmt) + 900 * ShroudTintAmt) / 255;
            fogMax = (gFogSettings->endDistance * (255 - ShroudTintAmt) + 1000 * ShroudTintAmt) / 255;

            gDPSetFogColor(gMainGfxPos++, fogR, fogG, fogB, gFogSettings->color.a);
            gSPFogPosition((*gfxPos)++, fogMin, fogMax);
            break;
        case RENDER_CLASS_1CYC_DEPTH:
        case RENDER_CLASS_2CYC_DEPTH:
            switch (renderMode) {
                case RENDER_MODE_SURFACE_OPA:
                    renderModeIdx = RENDER_MODE_IDX_1F;
                    break;
                case RENDER_MODE_DECAL_OPA:
                    renderModeIdx = RENDER_MODE_IDX_21;
                    break;
                case RENDER_MODE_INTERSECTING_OPA:
                    renderModeIdx = RENDER_MODE_IDX_23;
                    break;
                case RENDER_MODE_ALPHATEST:
                    renderModeIdx = RENDER_MODE_IDX_25;
                    break;
                case RENDER_MODE_CLOUD:
                    renderModeIdx = RENDER_MODE_IDX_3B;
                    break;
                case RENDER_MODE_CLOUD_NO_ZB:
                    renderModeIdx = RENDER_MODE_IDX_3C;
                    break;
                default:
                    renderModeIdx = RENDER_MODE_IDX_1F;
                    break;
            }
            gSPDisplayList((*gfxPos)++, ModelRenderModes[renderModeIdx]);
            break;
    }

    if (!(flags & MODEL_FLAG_TRANSFORM_GROUP_MEMBER)) {
        if (!(flags & MODEL_FLAG_IGNORE_MATRIX)) {
            gSPMatrix((*gfxPos)++, model->finalMtx, mtxLoadMode | mtxPushMode | G_MTX_MODELVIEW);
            if (mtxPushMode != G_MTX_NOPUSH) {
                mtxPushMode = G_MTX_NOPUSH;
            }
            if (mtxLoadMode != G_MTX_MUL) {
                mtxLoadMode = G_MTX_MUL;
            }
        }
    } else {
        mtxLoadMode = G_MTX_MUL;
        if (!(flags & MODEL_FLAG_IGNORE_MATRIX)) {
            gSPMatrix((*gfxPos)++, model->finalMtx, mtxLoadMode | mtxPushMode | G_MTX_MODELVIEW);
            if (mtxPushMode != G_MTX_NOPUSH) {
                mtxPushMode = G_MTX_NOPUSH;
            }
        }
    }

    // custom gfx 'pre'
    if (flags & MODEL_FLAG_USES_CUSTOM_GFX) {
        customGfxIndex = (model->customGfxIndex & 0xF) * 2;
        if ((*gCurrentCustomModelGfxPtr)[customGfxIndex] != NULL) {
            gSPDisplayList((*gfxPos)++, (*gCurrentCustomModelGfxPtr)[customGfxIndex]);
        }
    }

    // add tex panner gfx
    if (textureHeader != NULL) {
        if (flags & MODEL_FLAG_HAS_TEX_PANNER) {
            s32 panMainU = texPannerMainU[model->texPannerID] >> 8;
            s32 panMainV = texPannerMainV[model->texPannerID] >> 8;
            s32 panAuxU = texPannerAuxU[model->texPannerID] >> 8;
            s32 panAuxV = texPannerAuxV[model->texPannerID] >> 8;

            switch (extraTileType) {
                case EXTRA_TILE_AUX_SAME_AS_MAIN:
                    gDPSetTileSize((*gfxPos)++, G_TX_RENDERTILE, panMainU, panMainV, (textureHeader->mainW - 1) * 4 + panMainU, (textureHeader->mainH / 2 - 1) * 4 + panMainV);
                    gDPSetTileSize((*gfxPos)++, G_TX_RENDERTILE + 1, panAuxU, panAuxV, (textureHeader->mainW - 1) * 4 + panAuxU, (textureHeader->mainH / 2 - 1) * 4 + panAuxV);
                    break;
                case EXTRA_TILE_AUX_INDEPENDENT:
                    gDPSetTileSize((*gfxPos)++, G_TX_RENDERTILE, panMainU, panMainV, (textureHeader->mainW - 1) * 4 + panMainU, (textureHeader->mainH - 1) * 4 + panMainV);
                    gDPSetTileSize((*gfxPos)++, G_TX_RENDERTILE + 1, panAuxU, panAuxV, (textureHeader->auxW - 1) * 4 + panAuxU, (textureHeader->auxH - 1) * 4 + panAuxV);
                    break;
                default:
                    gDPSetTileSize((*gfxPos)++, G_TX_RENDERTILE, panMainU, panMainV, (textureHeader->mainW - 1) * 4 + panMainU, (textureHeader->mainH - 1) * 4 + panMainV);
                    break;
            }
        }
    }

    if (flags & MODEL_FLAG_BILLBOARD) {
        gSPMatrix((*gfxPos)++, gCameras[gCurrentCamID].mtxBillboard, mtxLoadMode | mtxPushMode | G_MTX_MODELVIEW);
        if (mtxPushMode != G_MTX_NOPUSH) {
            mtxPushMode = G_MTX_NOPUSH;
        }
        if (mtxLoadMode != G_MTX_MUL) {
            mtxLoadMode = G_MTX_MUL;
        }
    }

    // render the model
    if (!(flags & MODEL_FLAG_HAS_LOCAL_VERTEX_COPY)) {
        gSPDisplayList((*gfxPos)++, modelNode->displayData->displayList);
    }

    // custom gfx 'post'
    if (flags & MODEL_FLAG_USES_CUSTOM_GFX) {
        customGfxIndex++;
        if ((*gCurrentCustomModelGfxPtr)[customGfxIndex] != NULL) {
            gSPDisplayList((*gfxPos)++, (*gCurrentCustomModelGfxPtr)[customGfxIndex]);
        }
    }

    if (mtxPushMode == G_MTX_NOPUSH) {
        gSPPopMatrix((*gfxPos)++, G_MTX_MODELVIEW);
    }

    gDPPipeSync((*gfxPos)++);
}

void load_texture_impl(u32 romOffset, TextureHandle* handle, TextureHeader* header, s32 mainSize, s32 mainPalSize, s32 auxSize, s32 auxPalSize) {
    Gfx** temp;

    // load main img + palette to texture heap
    handle->raster = (IMG_PTR) TextureHeapPos;
    if (mainPalSize != 0) {
        handle->palette = (PAL_PTR) (TextureHeapPos + mainSize);
    } else {
        handle->palette = NULL;
    }
    dma_copy((u8*) romOffset, (u8*) (romOffset + mainSize + mainPalSize), TextureHeapPos);
    romOffset += mainSize + mainPalSize;
    TextureHeapPos += mainSize + mainPalSize;

    // load aux img + palette to texture heap
    if (auxSize != 0) {
        handle->auxRaster = (IMG_PTR) TextureHeapPos;
        if (auxPalSize != 0) {
            handle->auxPalette = (PAL_PTR) (TextureHeapPos + auxSize);
        } else {
            handle->auxPalette = NULL;
        }
        dma_copy((u8*) romOffset, (u8*) (romOffset + auxSize + auxPalSize), TextureHeapPos);
        TextureHeapPos += auxSize + auxPalSize;
    } else {
        handle->auxPalette = NULL;
        handle->auxRaster = NULL;
    }

    // copy header data and create a display list for the texture
    handle->gfx = (Gfx*) TextureHeapPos;
    memcpy(&handle->header, header, sizeof(*header));
    make_texture_gfx(header, (Gfx**) &TextureHeapPos, handle->raster, handle->palette, handle->auxRaster, handle->auxPalette, 0, 0, 0, 0);

    temp = (Gfx**) &TextureHeapPos;
    gSPEndDisplayList((*temp)++);
}

void load_texture_by_name(ModelNodeProperty* propertyName, s32 romOffset, s32 size) {
    char* textureName = (char*)propertyName->data.p;
    u32 startOffset = romOffset;
    s32 textureIdx = 0;
    u32 paletteSize;
    u32 rasterSize;
    u32 auxPaletteSize;
    u32 auxRasterSize;
    TextureHeader* header;
    TextureHandle* textureHandle;
    s32 mainSize;

    if (textureName == NULL) {
        (*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID = 0;
        return;
    }

    while (romOffset < startOffset + size) {
        dma_copy((u8*)romOffset, (u8*)romOffset + sizeof(gCurrentTextureHeader), &gCurrentTextureHeader);
        header = &gCurrentTextureHeader;

        rasterSize = header->mainW * header->mainH;

        // compute mipmaps size
        if (header->mainBitDepth == G_IM_SIZ_4b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 16 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
            rasterSize /= 2;
        } else if (header->mainBitDepth == G_IM_SIZ_8b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 8 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
        } else if (header->mainBitDepth == G_IM_SIZ_16b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 4 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
            rasterSize *= 2;
        } else if (header->mainBitDepth == G_IM_SIZ_32b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 2 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
            rasterSize *= 4;
        }

        // compute palette size
        if (header->mainFmt == G_IM_FMT_CI) {
            paletteSize = 0x20;
            if (header->mainBitDepth == G_IM_SIZ_8b) {
                paletteSize = 0x200;
            }
        } else {
            paletteSize = 0;
        }

        // compute aux tile size
        if (header->extraTiles == EXTRA_TILE_AUX_INDEPENDENT) {
            auxRasterSize = header->auxW * header->auxH;
            if (header->auxBitDepth == G_IM_SIZ_4b) {
                auxRasterSize /= 2;
            } else if (header->auxBitDepth == G_IM_SIZ_8b) {
            } else if (header->auxBitDepth == G_IM_SIZ_16b) {
                auxRasterSize *= 2;
            } else {
                if (header->auxBitDepth == G_IM_SIZ_32b) {
                    auxRasterSize *= 4;
                }
            }
            if (header->auxFmt == G_IM_FMT_CI) {
                auxPaletteSize = 0x20;
                if (header->auxBitDepth == G_IM_SIZ_8b) {
                    auxPaletteSize = 0x200;
                }
            } else {
                auxPaletteSize = 0;
            }
        } else {
            auxPaletteSize = 0;
            auxRasterSize = 0;
        }

        if (strcmp(textureName, header->name) == 0) {
            // found the texture with `textureName`
            break;
        }

        textureIdx++;
        mainSize = rasterSize + paletteSize + sizeof(*header);
        romOffset += mainSize;
        romOffset += auxRasterSize + auxPaletteSize;
    }

    if (romOffset >= startOffset + 0x40000) {
        // did not find the texture with `textureName`
        printf("could not find texture '%s'\n", textureName);
        (*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID = 0;
        return;
    }

    (*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID = textureIdx + 1;
    textureHandle = &TextureHandles[(*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID];
    romOffset += sizeof(*header);

    if (textureHandle->gfx == NULL) {
        load_texture_impl(romOffset, textureHandle, header, rasterSize, paletteSize, auxRasterSize, auxPaletteSize);
        load_texture_variants(romOffset + rasterSize + paletteSize + auxRasterSize + auxPaletteSize, (*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID, startOffset, size);
    }
}

// loads variations for current texture by looping through the following textures until a non-variant is found
void load_texture_variants(u32 romOffset, s32 textureID, s32 baseOffset, s32 size) {
    u32 offset;
    TextureHeader iterTextureHeader;
    TextureHeader* header;
    TextureHandle* textureHandle;
    u32 rasterSize;
    s32 paletteSize;
    u32 auxRasterSize;
    u32 auxPaletteSize;
    s32 mainSize;
    s32 currentTextureID = textureID;

    for (offset = romOffset; offset < baseOffset + size;) {
        dma_copy((u8*)offset, (u8*)offset + sizeof(iterTextureHeader), &iterTextureHeader);
        header = &iterTextureHeader;

        if (strcmp(header->name, "end_of_textures") == 0) {
            return;
        }

        if (!header->isVariant) {
            // done reading variants
            break;
        }

        rasterSize = header->mainW * header->mainH;

        // compute mipmaps size
        if (header->mainBitDepth == G_IM_SIZ_4b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 16 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
            rasterSize /= 2;
        } else if (header->mainBitDepth == G_IM_SIZ_8b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 8 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
        } else if (header->mainBitDepth == G_IM_SIZ_16b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 4 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
            rasterSize *= 2;
        } else if (header->mainBitDepth == G_IM_SIZ_32b) {
            if (header->extraTiles == EXTRA_TILE_MIPMAPS) {
                s32 d = 2;
                while (header->mainW / d >= 2 && header->mainH / d > 0) {
                    rasterSize += header->mainW / d * header->mainH / d;
                    d *= 2;
                }
            }
            rasterSize *= 4;
        }

        // compute palette size
        if (header->mainFmt == G_IM_FMT_CI) {
            paletteSize = 0x20;
            if (header->mainBitDepth == G_IM_SIZ_8b) {
                paletteSize = 0x200;
            }
        } else {
            paletteSize = 0;
        }

        // compute aux tile size
        if (header->extraTiles == EXTRA_TILE_AUX_INDEPENDENT) {
            auxRasterSize = header->auxW * header->auxH;
            if (header->auxBitDepth == G_IM_SIZ_4b) {
                auxRasterSize /= 2;
            } else if (header->auxBitDepth == G_IM_SIZ_8b) {
            } else if (header->auxBitDepth == G_IM_SIZ_16b) {
                auxRasterSize *= 2;
            } else {
                if (header->auxBitDepth == G_IM_SIZ_32b) {
                    auxRasterSize *= 4;
                }
            }
            if (header->auxFmt == G_IM_FMT_CI) {
                auxPaletteSize = 0x20;
                if (header->auxBitDepth == G_IM_SIZ_8b) {
                    auxPaletteSize = 0x200;
                }
            } else {
                auxPaletteSize = 0;
            }
        } else {
            auxPaletteSize = 0;
            auxRasterSize = 0;
        }

        textureID++;
        currentTextureID = textureID;
        textureHandle = &TextureHandles[currentTextureID];
        load_texture_impl(offset + sizeof(*header), textureHandle, header, rasterSize, paletteSize, auxRasterSize, auxPaletteSize);

        mainSize = rasterSize + paletteSize + sizeof(*header);
        offset += mainSize;
        offset += auxRasterSize + auxPaletteSize;
    }
}

ModelNodeProperty* get_model_property(ModelNode* node, ModelPropertyKeys key) {
    s32 numProperties = node->numProperties;
    ModelNodeProperty* propertyList = node->propertyList;
    s32 i;

    for (i = 0; i < numProperties; i++, propertyList++) {
        if (propertyList->key == key) {
            return propertyList;
        }
    }
    return NULL;
}

// load textures used by models, starting from current model
void load_next_model_textures(ModelNode* model, s32 romOffset, s32 texSize) {
    if (model->type != SHAPE_TYPE_MODEL) {
        if (model->groupData != NULL) {
            s32 numChildren = model->groupData->numChildren;

            if (numChildren != 0) {
                s32 i;

                for (i = 0; i < numChildren; i++) {
                    load_next_model_textures(model->groupData->childList[i], romOffset, texSize);
                }
            }
        }
    } else {
        ModelNodeProperty* propTextureName = get_model_property(model, MODEL_PROP_KEY_TEXTURE_NAME);
        if (propTextureName != NULL) {
            load_texture_by_name(propTextureName, romOffset, texSize);
        }
    }
    TreeIterPos++;
}

// load all textures used by models, starting from the root
void mdl_load_all_textures(ModelNode* rootModel, s32 romOffset, s32 size) {
    s32 baseOffset = 0;

    // textures are loaded to the upper half of the texture heap when not in the world
    if (gGameStatusPtr->context != CONTEXT_WORLD) {
        baseOffset = WORLD_TEXTURE_MEMORY_SIZE;
    }

    TextureHeapPos = TextureHeapBase + baseOffset;

    if (rootModel != NULL && romOffset != 0 && size != 0) {
        s32 i;

        for (i = 0; i < ARRAY_COUNT(TextureHandles); i++) {
            TextureHandles[i].gfx = NULL;
        }

        TreeIterPos = 0;
        if (rootModel != NULL) {
            load_next_model_textures(rootModel, romOffset, size);
        }
    }
}

s32 mdl_get_child_count(ModelNode* model) {
    s32 ret = 0;

    if (model->type != SHAPE_TYPE_MODEL && model->groupData != NULL) {
        s32 numChildren = model->groupData->numChildren;

        if (numChildren != 0) {
            s32 i;

            ret += numChildren;
            for (i = 0; i < numChildren; i++) {
                ret += mdl_get_child_count(model->groupData->childList[i]);
            }
        }
    }
    return ret;
}

void clear_model_data(void) {
    s32 i;

    if (gGameStatusPtr->context == CONTEXT_WORLD) {
        gCurrentModels = &wModelList;
        gCurrentTransformGroups = &wTransformGroups;
        gCurrentCustomModelGfxPtr = &wCustomModelGfx;
        gCurrentCustomModelGfxBuildersPtr = &wCustomModelGfxBuilders;
        gCurrentModelTreeRoot = &wModelTreeRoot;
        gCurrentModelLocalVtxBuffers = &wModelLocalVtxBuffers;
        gCurrentModelTreeNodeInfo = &wModelTreeNodeInfo;
        gBackgroundTintModePtr = &wBackgroundTintMode;
        ShroudTintAmt = 0;
        ShroudTintR = 0;
        ShroudTintG = 0;
        ShroudTintB = 0;
        gFogSettings = &wFogSettings;
    } else {
        gCurrentModels = &bModelList;
        gCurrentTransformGroups = &bTransformGroups;
        gCurrentCustomModelGfxPtr = &bCustomModelGfx;
        gCurrentCustomModelGfxBuildersPtr = &bCustomModelGfxBuilders;
        gCurrentModelTreeRoot = &bModelTreeRoot;
        gCurrentModelLocalVtxBuffers = &bModelLocalVtxBuffers;
        gCurrentModelTreeNodeInfo = &bModelTreeNodeInfo;
        gBackgroundTintModePtr = &bBackgroundTintMode;
        gFogSettings = &bFogSettings;
    }

    for (i = 0; i < ARRAY_COUNT(*gCurrentModels); i++) {
        (*gCurrentModels)[i] = 0;
    }

    for (i = 0; i < ARRAY_COUNT(*gCurrentTransformGroups); i++) {
        (*gCurrentTransformGroups)[i] = 0;
    }

    for (i = 0; i < ARRAY_COUNT(*gCurrentCustomModelGfxPtr); i++) {
        (*gCurrentCustomModelGfxPtr)[i] = 0;
        (*gCurrentCustomModelGfxBuildersPtr)[i] = 0;
    }

    *gCurrentModelTreeRoot = NULL;

    for (i = 0; i < ARRAY_COUNT(*gCurrentModelTreeNodeInfo); i++) {
        (*gCurrentModelTreeNodeInfo)[i].modelIndex = -1;
        (*gCurrentModelTreeNodeInfo)[i].treeDepth = 0;
        (*gCurrentModelTreeNodeInfo)[i].textureID = 0;
    }

    *gBackgroundTintModePtr = ENV_TINT_NONE;
    gFogSettings->enabled = FALSE;
    gFogSettings->color.r = 10;
    gFogSettings->color.g = 10;
    gFogSettings->color.b = 10;
    gFogSettings->color.a = 0;
    gFogSettings->startDistance = 950;
    gFogSettings->endDistance = 1000;

    for (i = 0; i < ARRAY_COUNT(texPannerAuxV); i++) {
        texPannerAuxV[i] = 0;
        texPannerAuxU[i] = 0;
        texPannerMainV[i] = 0;
        texPannerMainU[i] = 0;
    }
}

void init_model_data(void) {
    if (gGameStatusPtr->context == CONTEXT_WORLD) {
        gCurrentModels = &wModelList;
        gCurrentTransformGroups = &wTransformGroups;
        gCurrentCustomModelGfxPtr = &wCustomModelGfx;
        gCurrentCustomModelGfxBuildersPtr = &wCustomModelGfxBuilders;
        gCurrentModelTreeRoot = &wModelTreeRoot;
        gCurrentModelLocalVtxBuffers = &wModelLocalVtxBuffers;
        gCurrentModelTreeNodeInfo = &wModelTreeNodeInfo;
        gBackgroundTintModePtr = &wBackgroundTintMode;
        gFogSettings = &wFogSettings;
    } else {
        gCurrentModels = &bModelList;
        gCurrentTransformGroups = &bTransformGroups;
        gCurrentCustomModelGfxPtr = &bCustomModelGfx;
        gCurrentCustomModelGfxBuildersPtr = &bCustomModelGfxBuilders;
        gCurrentModelTreeRoot = &bModelTreeRoot;
        gCurrentModelLocalVtxBuffers = &bModelLocalVtxBuffers;
        gCurrentModelTreeNodeInfo = &bModelTreeNodeInfo;
        gBackgroundTintModePtr = &bBackgroundTintMode;
        gFogSettings = &bFogSettings;
    }
}

void mdl_calculate_model_sizes(void) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(*gCurrentModels); i++) {
        Model* model = (*gCurrentModels)[i];

        if (model != NULL) {
            ModelBoundingBox* bb = (ModelBoundingBox*)get_model_property(model->modelNode, MODEL_PROP_KEY_BOUNDING_BOX);

            bb->halfSizeX = (bb->maxX - bb->minX) * 0.5;
            bb->halfSizeY = (bb->maxY - bb->minY) * 0.5;
            bb->halfSizeZ = (bb->maxZ - bb->minZ) * 0.5;
            model->flags |= MODEL_FLAG_MATRIX_DIRTY;
        }
    }
}

void mdl_create_model(ModelBlueprint* bp, s32 unused) {
    ModelNode* node = bp->mdlNode;
    ModelNodeProperty* prop;
    ModelBoundingBox* bb;
    s32 modelIdx;
    Model* model;
    f32 x, y, z;

    prop = get_model_property(node, MODEL_PROP_KEY_SPECIAL);
    modelIdx = 0;
    if (prop != NULL) {
        s32 replaceWithFlame = (prop->data.s >> 4) & 0xF;

        if (replaceWithFlame != 0) {
            prop = get_model_property(node, MODEL_PROP_KEY_BOUNDING_BOX);
            if (prop != NULL) {
                ModelBoundingBox* bb = (ModelBoundingBox*) prop;
                EffectInstance* effect;

                fx_flame(replaceWithFlame - 1,
                    (bb->minX + bb->maxX) * 0.5f,
                    bb->minY,
                    (bb->minZ + bb->maxZ) * 0.5f,
                    1.0f,
                    &effect);
                return;
            }
        }
    }

    for (modelIdx = 0; modelIdx < ARRAY_COUNT(*gCurrentModels); modelIdx++) {
        if ((*gCurrentModels)[modelIdx] == NULL) {
            break;
        }
    }

    (*gCurrentModels)[modelIdx] = model = heap_malloc(sizeof(*model));
    model->flags = bp->flags | MODEL_FLAG_VALID;
    model->modelID = TreeIterPos;
    model->modelNode = bp->mdlNode;
    model->groupData = bp->groupData;
    model->matrixFreshness = 0;
    node = model->modelNode;

    prop = get_model_property(node, MODEL_PROP_KEY_SPECIAL);
    if (prop != NULL) {
        model->texPannerID = prop->data.s & 0xF;
    } else {
        model->texPannerID = TEX_PANNER_0;
    }
    model->customGfxIndex = CUSTOM_GFX_0;

    if (node->type != SHAPE_TYPE_GROUP) {
        prop = get_model_property(node, MODEL_PROP_KEY_RENDER_MODE);
    } else {
        prop = get_model_property(node, MODEL_PROP_KEY_GROUP_INFO);

        if (prop != NULL) {
            // GROUP_INFO properties always come in pairs, with the second giving the render mode
            prop++;
        }
    }

    if (prop != NULL) {
        model->renderMode = prop->data.s;
    } else {
        model->renderMode = RENDER_MODE_SURFACE_OPA;
    }

    model->textureID = (*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID;
    model->textureVariation = 0;

    if (!is_identity_fixed_mtx(bp->mtx)) {
        model->bakedMtx = heap_malloc(sizeof(*model->bakedMtx));
        *model->bakedMtx = *bp->mtx;
        model->savedMtx = *model->bakedMtx;
    } else {
        model->bakedMtx = NULL;
        guMtxIdent(&model->savedMtx);
        model->flags |= MODEL_FLAG_IGNORE_MATRIX;
    }

    guMtxIdentF(model->userTransformMtx);
    model->finalMtx = NULL;
    prop = get_model_property(node, MODEL_PROP_KEY_BOUNDING_BOX);
    if (prop != NULL) {
        ModelBoundingBox* bb = (ModelBoundingBox*) prop;

        x = (bb->minX + bb->maxX) * 0.5f;
        y = (bb->minY + bb->maxY) * 0.5f;
        z = (bb->minZ + bb->maxZ) * 0.5f;
    } else {
        x = y = z = 0.0f;
    }

    if (model->bakedMtx != NULL) {
        guMtxXFML(model->bakedMtx, x, y, z, &x, &y, &z);
    }

    model->center.x = x;
    model->center.y = y;
    model->center.z = z;

    bb = (ModelBoundingBox*) prop;
    x = bb->maxX - bb->minX;
    y = bb->maxY - bb->minY;
    z = bb->maxZ - bb->minZ;
    bb->halfSizeX = x * 0.5;
    bb->halfSizeY = y * 0.5;
    bb->halfSizeZ = z * 0.5;

    if (model->bakedMtx == NULL && x < 100.0f && y < 100.0f && z < 100.0f) {
        model->flags |= MODEL_FLAG_DO_BOUNDS_CULLING;
    }
    (*gCurrentModelTreeNodeInfo)[TreeIterPos].modelIndex = modelIdx;
}

void mdl_update_transform_matrices(void) {
    Matrix4f tempModelMtx;
    Matrix4f tempGroupMtx;
    f32 mX, mY, mZ;
    f32 mtgX, mtgY, mtgZ;
    Model* model;
    Mtx* curMtx;
    ModelBoundingBox* bb;
    ModelTransformGroup* mtg;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(*gCurrentModels); i++) {
        model = (*gCurrentModels)[i];
        if (model != NULL && (model->flags != 0) && !(model->flags & MODEL_FLAG_INACTIVE)) {
            if (!(model->flags & MODEL_FLAG_MATRIX_DIRTY)) {
                if (model->matrixFreshness != 0) {
                    // matrix was recalculated recently and stored on the matrix stack
                    // since DisplayContexts alternate, we can fetch the previous matrix from the other context
                    model->matrixFreshness--;
                    if (model->matrixFreshness == 0) {
                        // since it hasn't changed in a few frames, cache the matrix
                        model->savedMtx = *model->finalMtx;
                    }
                    // copy matrix from previous DisplayContext stack to current one
                    curMtx = model->finalMtx;
                    model->finalMtx = &gDisplayContext->matrixStack[gMatrixListPos++];
                    *model->finalMtx = *curMtx;
                } else {
                    // transform matrix is not changed, have gfx build with saved matrix
                    model->finalMtx = &model->savedMtx;
                }
            } else {
                // first frame with dirty matrix, need to recalculate it
                model->flags &= ~MODEL_FLAG_MATRIX_DIRTY;
                model->matrixFreshness = 2;

                // write matrix to the matrix stack
                curMtx = &gDisplayContext->matrixStack[gMatrixListPos++];
                if (model->bakedMtx == NULL || (model->flags & MODEL_FLAG_TRANSFORM_GROUP_MEMBER)) {
                    guMtxF2L(model->userTransformMtx, curMtx);
                } else {
                    guMtxL2F(tempModelMtx, model->bakedMtx);
                    guMtxCatF(model->userTransformMtx, tempModelMtx, tempModelMtx);
                    guMtxF2L(tempModelMtx, curMtx);
                }
                model->flags &= ~MODEL_FLAG_IGNORE_MATRIX;

                // recalculate the center of the model with transformation applied
                bb = (ModelBoundingBox*) get_model_property(model->modelNode, MODEL_PROP_KEY_BOUNDING_BOX);
                mX = (bb->minX + bb->maxX) * 0.5f;
                mY = (bb->minY + bb->maxY) * 0.5f;
                mZ = (bb->minZ + bb->maxZ) * 0.5f;
                guMtxXFML(curMtx, mX, mY, mZ, &mX, &mY, &mZ);
                model->center.x = mX;
                model->center.y = mY;
                model->center.z = mZ;

                // point matrix for gfx building to our matrix on the stack
                model->finalMtx = curMtx;

                // disable bounds culling for models with dynamic transformations
                model->flags &= ~MODEL_FLAG_DO_BOUNDS_CULLING;
            }
        }
    }

    for (i = 0; i < ARRAY_COUNT((*gCurrentTransformGroups)); i++) {
        mtg = (*gCurrentTransformGroups)[i];
        if (mtg != NULL && mtg->flags != 0 && !(mtg->flags & TRANSFORM_GROUP_FLAG_INACTIVE)) {
            if (!(mtg->flags & TRANSFORM_GROUP_FLAG_MATRIX_DIRTY)) {
                if (mtg->matrixFreshness != 0) {
                    // matrix was recalculated recently and stored on the matrix stack
                    // since DisplayContexts alternate, we can fetch the previous matrix from the other context
                    mtg->matrixFreshness--;
                    if (mtg->matrixFreshness == 0) {
                        // since it hasn't changed in a few frames, cache the matrix
                        mtg->savedMtx = *mtg->finalMtx;
                    }
                    // copy matrix from previous DisplayContext stack to current one
                    curMtx = mtg->finalMtx;
                    mtg->finalMtx = &gDisplayContext->matrixStack[gMatrixListPos++];
                    *mtg->finalMtx = *curMtx;
                } else {
                    // transform matrix is not changed, have gfx build with saved matrix
                    mtg->finalMtx = &mtg->savedMtx;
                }
            } else {
                // first frame with dirty matrix, need to recalculate it
                mtg->flags &= ~TRANSFORM_GROUP_FLAG_MATRIX_DIRTY;
                mtg->matrixFreshness = 2;

                // write matrix to the matrix stack
                curMtx = &gDisplayContext->matrixStack[gMatrixListPos++];
                if (mtg->bakedMtx == NULL) {
                    guMtxF2L(mtg->userTransformMtx, curMtx);
                } else {
                    guMtxL2F(tempGroupMtx, mtg->bakedMtx);
                    guMtxCatF(mtg->userTransformMtx, tempGroupMtx, tempGroupMtx);
                    guMtxF2L(tempGroupMtx, curMtx);
                }
                mtg->flags &= ~TRANSFORM_GROUP_FLAG_IGNORE_MATRIX;

                // recalculate the center of the transform group with transformation applied
                bb = (ModelBoundingBox*) get_model_property(mtg->baseModelNode, MODEL_PROP_KEY_BOUNDING_BOX);
                mtgX = (bb->minX + bb->maxX) * 0.5f;
                mtgY = (bb->minY + bb->maxY) * 0.5f;
                mtgZ = (bb->minZ + bb->maxZ) * 0.5f;
                guMtxXFML(curMtx, mtgX, mtgY, mtgZ, &mtgX, &mtgY, &mtgZ);
                mtg->center.x = mtgX;
                mtg->center.y = mtgY;
                mtg->center.z = mtgZ;

                // point matrix for gfx building to our matrix on the stack
                mtg->finalMtx = curMtx;
            }
        }
    }

    build_custom_gfx();
}

void render_models(void) {
    RenderTask rt;
    RenderTask* rtPtr = &rt;
    f32 outX, outY, outZ, outW;
    f32 m00, m01, m02, m03;
    f32 m10, m11, m12, m13;
    f32 m20, m21, m22, m23;
    f32 m30, m31, m32, m33;
    f32 centerX, centerY, centerZ;
    f32 bbx, bby, bbz;

    Camera* camera = &gCameras[gCurrentCameraID];
    Model* model;
    ModelBoundingBox* boundingBox;
    ModelTransformGroup* transformGroup;
    f32 xComp, yComp, zComp;

    s32 distance;
    s32 notVisible;
    s32 i;

#define TEST_POINT_VISIBILITY \
    outX = (m00 * xComp) + (m10 * yComp) + (m20 * zComp) + m30; \
    outY = (m01 * xComp) + (m11 * yComp) + (m21 * zComp) + m31; \
    outZ = (m02 * xComp) + (m12 * yComp) + (m22 * zComp) + m32; \
    outW = (m03 * xComp) + (m13 * yComp) + (m23 * zComp) + m33; \
    if (outW == 0.0f) { \
        break; \
    } \
    /* Perspective divide */ \
    outW = 1.0f / outW; \
    xComp = outX * outW; \
    yComp = outY * outW; \
    zComp = outZ * outW; \
    if (zComp > -1.0f && xComp >= -1.0f && xComp <= 1.0f && yComp >= -1.0f && yComp <= 1.0f) { \
        break; \
    }

    m00 = camera->mtxPerspective[0][0];
    m01 = camera->mtxPerspective[0][1];
    m02 = camera->mtxPerspective[0][2];
    m03 = camera->mtxPerspective[0][3];
    m10 = camera->mtxPerspective[1][0];
    m11 = camera->mtxPerspective[1][1];
    m12 = camera->mtxPerspective[1][2];
    m13 = camera->mtxPerspective[1][3];
    m20 = camera->mtxPerspective[2][0];
    m21 = camera->mtxPerspective[2][1];
    m22 = camera->mtxPerspective[2][2];
    m23 = camera->mtxPerspective[2][3];
    m30 = camera->mtxPerspective[3][0];
    m31 = camera->mtxPerspective[3][1];
    m32 = camera->mtxPerspective[3][2];
    m33 = camera->mtxPerspective[3][3];

    // enqueue all visible models not in transform groups
    for (i = 0; i < ARRAY_COUNT(*gCurrentModels); i++) {
        model = (*gCurrentModels)[i];
        if (model == NULL) {
            continue;
        }
        if (model->flags == 0) {
            continue;
        }
        if (model->flags & MODEL_FLAG_INACTIVE) {
            continue;
        }
        if (model->flags & MODEL_FLAG_HIDDEN) {
            continue;
        }
        if (model->flags & MODEL_FLAG_20) {
            continue;
        }
        if (model->flags & MODEL_FLAG_TRANSFORM_GROUP_MEMBER) {
            continue;
        }

        centerX = model->center.x;
        centerY = model->center.y;
        centerZ = model->center.z;

        // for models that are small enough to do bounds culling, only render if at least one
        // corner of its boundary box is visible
        if (model->flags & MODEL_FLAG_DO_BOUNDS_CULLING) {
            notVisible = FALSE;
            boundingBox = (ModelBoundingBox*) model->modelNode->propertyList;
            bbx = boundingBox->halfSizeX;
            bby = boundingBox->halfSizeY;
            bbz = boundingBox->halfSizeZ;

            while (TRUE) {
                if (TRUE) {
                    xComp = centerX - bbx;
                    yComp = centerY - bby;
                    zComp = centerZ - bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bbx != 0.0f) {
                    xComp = centerX + bbx;
                    yComp = centerY - bby;
                    zComp = centerZ - bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bby != 0.0f) {
                    xComp = centerX - bbx;
                    yComp = centerY + bby;
                    zComp = centerZ - bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bbx != 0.0f && bby != 0.0f) {
                    xComp = centerX + bbx;
                    yComp = centerY + bby;
                    zComp = centerZ - bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bbz != 0.0f) {
                    xComp = centerX - bbx;
                    yComp = centerY - bby;
                    zComp = centerZ + bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bbx != 0.0f && bbz != 0.0f) {
                    xComp = centerX + bbx;
                    yComp = centerY - bby;
                    zComp = centerZ + bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bby != 0.0f && bbz != 0.0f) {
                    xComp = centerX - bbx;
                    yComp = centerY + bby;
                    zComp = centerZ + bbz;
                    TEST_POINT_VISIBILITY;
                }

                if (bbx != 0.0f && bby != 0.0f && bbz != 0.0f) {
                    xComp = centerX + bbx;
                    yComp = centerY + bby;
                    zComp = centerZ + bbz;
                    TEST_POINT_VISIBILITY;
                }
                notVisible = TRUE;
                break;
            }
            // no points of the models bounding box were visible
            if (notVisible) {
                continue;
            }
        }

        // map all model depths to the interval [0, 10k] and submit render task
        transform_point(camera->mtxPerspective, centerX, centerY, centerZ, 1.0f, &outX, &outY, &outZ, &outW);
        distance = outZ + 5000.0f;
        if (distance < 0) {
            distance = 0;
        } else if (distance > 10000) {
            distance = 10000;
        }
        rtPtr->appendGfxArg = model;
        if (model->modelNode->type == SHAPE_TYPE_GROUP) {
            rtPtr->appendGfx = appendGfx_model_group;
        } else {
            rtPtr->appendGfx = appendGfx_model;
        }
        rtPtr->dist = -distance;
        rtPtr->renderMode = model->renderMode;
        queue_render_task(rtPtr);
    }

    // enqueue models in transform groups
    // only the center of the group is used for depth sorting
    for (i = 0; i < ARRAY_COUNT(*gCurrentTransformGroups); i++) {
        transformGroup = (*gCurrentTransformGroups)[i];
        if (transformGroup == NULL) {
            continue;
        }

        if (transformGroup->flags == 0) {
            continue;
        }

        if (transformGroup->flags & TRANSFORM_GROUP_FLAG_INACTIVE) {
            continue;
        }

        xComp = transformGroup->center.x;
        yComp = transformGroup->center.y;
        zComp = transformGroup->center.z;

        transform_point(
            camera->mtxPerspective,
            xComp, yComp, zComp, 1.0f,
            &outX, &outY, &outZ, &outW
        );
        if (outW == 0.0f) {
            outW = 1.0f;
        }

        distance = ((outZ / outW) * 10000.0f);

        if (!(transformGroup->flags & TRANSFORM_GROUP_FLAG_HIDDEN)) {
            rtPtr->appendGfx = render_transform_group;
            rtPtr->appendGfxArg = transformGroup;
            rtPtr->dist = -distance;
            rtPtr->renderMode = transformGroup->renderMode;
            queue_render_task(rtPtr);
        }
    }
}

void appendGfx_model_group(void* data) {
    Model* model = data;
    s32 modelTreeDepth = (*gCurrentModelTreeNodeInfo)[model->modelID].treeDepth;
    s32 i;

    for (i = model->modelID - 1; i >= 0; i--) {
        if (modelTreeDepth >= (*gCurrentModelTreeNodeInfo)[i].treeDepth) {
            break;
        }
    }

    TreeIterPos = i + 1;
    func_80117D00(model);
}

void func_80117D00(Model* model) {
    Model* mdl = model; // temps needed to match
    ModelNode* modelNode = mdl->modelNode;

    if (model->modelNode->type != SHAPE_TYPE_MODEL) {
        if (modelNode->groupData != NULL) {
            s32 numChildren = modelNode->groupData->numChildren;

            if (numChildren != 0) {
                s32 i;

                for (i = 0; i < numChildren; i++, TreeIterPos++) {
                    Model newModel = *mdl;
                    ModelNodeProperty* prop;

                    newModel.flags = mdl->flags;
                    newModel.finalMtx = mdl->finalMtx;
                    newModel.modelNode = modelNode->groupData->childList[i];
                    newModel.texPannerID = mdl->texPannerID;
                    newModel.customGfxIndex = mdl->customGfxIndex;

                    if (newModel.modelNode->type == SHAPE_TYPE_MODEL) {
                        prop = get_model_property(newModel.modelNode, MODEL_PROP_KEY_RENDER_MODE);
                    } else {
                        prop = NULL;
                    }

                    if (prop != NULL) {
                        newModel.renderMode = prop->data.s;
                    } else {
                        newModel.renderMode = 0;
                    }

                    newModel.textureID = (*gCurrentModelTreeNodeInfo)[TreeIterPos].textureID;
                    newModel.textureVariation = 0;
                    func_80117D00(&newModel);
                }
            }
        }
    } else {
        appendGfx_model(mdl);
    }
}

// this looks like a switch, but I can't figure it out
void render_transform_group_node(ModelNode* node) {
    Gfx** gfx = &gMainGfxPos;
    Model* model;

    if (node != NULL) {
        if (node->type == SHAPE_TYPE_GROUP) {
            ModelNodeProperty* groupInfoProp = get_model_property(node, MODEL_PROP_KEY_GROUP_INFO);

            if (groupInfoProp != NULL && groupInfoProp->data.s != 0) {
                model = get_model_from_list_index(mtg_IterIdx);
                if (!(model->flags & MODEL_FLAG_HIDDEN)) {
                    appendGfx_model_group(model);
                }
                mtg_IterIdx++;
                return;
            }
        }
        if (node->type != SHAPE_TYPE_MODEL) {
            if (node->groupData != NULL) {
                s32 numChildren;
                s32 i;

                if (node->groupData->transformMatrix != NULL) {
                    gSPMatrix((*gfx)++, node->groupData->transformMatrix, G_MTX_PUSH | G_MTX_MUL | G_MTX_MODELVIEW);
                }

                numChildren = node->groupData->numChildren;
                if (numChildren != 0) {
                    for (i = 0; i < numChildren; i++) {
                        render_transform_group_node(node->groupData->childList[i]);
                    }
                }

                if (node->groupData->transformMatrix != NULL) {
                    gSPPopMatrix((*gfx)++, G_MTX_MODELVIEW);
                }
            }
            return;
        }

        model = get_model_from_list_index(mtg_IterIdx);
        if (!(model->flags & MODEL_FLAG_HIDDEN)) {
            appendGfx_model(model);
        }
        mtg_IterIdx++;
    }
}

// gfx temps needed
void render_transform_group(void* data) {
    ModelTransformGroup* group = data;
    Gfx** gfx = &gMainGfxPos;

    if (!(group->flags & TRANSFORM_GROUP_FLAG_INACTIVE)) {
        mtg_IterIdx = group->minChildModelIndex;
        if (!(group->flags & TRANSFORM_GROUP_FLAG_IGNORE_MATRIX)) {
            gSPMatrix((*gfx)++, group->finalMtx, (G_MTX_PUSH | G_MTX_LOAD) | G_MTX_MODELVIEW);
        }

        render_transform_group_node(group->baseModelNode);

        if (!(group->flags & TRANSFORM_GROUP_FLAG_IGNORE_MATRIX)) {
            gSPPopMatrix((*gfx)++, G_MTX_MODELVIEW);
        }
        gDPPipeSync((*gfx)++);
    }
}

void make_texture_gfx(TextureHeader* header, Gfx** gfxPos, IMG_PTR raster, PAL_PTR palette, IMG_PTR auxRaster, PAL_PTR auxPalette, u8 auxShiftS, u8 auxShiftT, u16 auxOffsetS, u16 auxOffsetT) {
    s32 mainWidth, mainHeight;
    s32 auxWidth, auxHeight;
    s32 mainFmt;
    s32 auxFmt;
    s32 mainWrapW, mainWrapH;
    s32 auxWrapW, auxWrapH;
    s32 extraTileType;
    u32 texCombineType;
    s32 lod;
    s32 lodDivisor;
    IMG_PTR rasterPtr;
    s32 filteringMode;
    s32 auxPaletteIndex;
    s32 lutMode;
    s32 lodMode;
    s32 mainMasks, mainMaskt;
    s32 auxMasks, auxMaskt;
    s32 mainBitDepth;
    s32 auxBitDepth;
    s32 temp;

    mainWidth = header->mainW;
    mainHeight = header->mainH;

    lod = 0;
    auxPaletteIndex = 0;

    mainMasks = INTEGER_LOG2(mainWidth);
    mainMaskt = INTEGER_LOG2(mainHeight);

    mainWrapW = header->mainWrapW;
    mainWrapH = header->mainWrapH;

    mainFmt = header->mainFmt;
    mainBitDepth = header->mainBitDepth;

    extraTileType = header->extraTiles;
    filteringMode = header->filtering << G_MDSFT_TEXTFILT;

    auxWidth = header->auxW;
    auxHeight = header->auxH;

    auxMasks = INTEGER_LOG2(auxWidth);
    auxMaskt = INTEGER_LOG2(auxHeight);

    auxWrapW = header->auxWrapW;
    auxWrapH = header->auxWrapH;
    auxFmt = header->auxFmt;
    auxBitDepth = header->auxBitDepth;

    if (extraTileType == EXTRA_TILE_AUX_INDEPENDENT) {
        if (palette != NULL) {
            auxPaletteIndex = 1;
        } else {
            auxPaletteIndex = 0;
        }
    }

    if (palette != NULL || auxPalette != NULL) {
        lutMode = G_TT_RGBA16;
        if (palette != NULL) {
            if (mainBitDepth == G_IM_SIZ_4b) {
                gDPLoadTLUT_pal16((*gfxPos)++, 0, palette);
            } else if (mainBitDepth == G_IM_SIZ_8b) {
                gDPLoadTLUT_pal256((*gfxPos)++, palette);
            }
        }
        if (auxPalette != NULL) {
            if (auxBitDepth == G_IM_SIZ_4b) {
                gDPLoadTLUT_pal16((*gfxPos)++, auxPaletteIndex, auxPalette);
            } else if (auxBitDepth == G_IM_SIZ_8b) {
                gDPLoadTLUT_pal256((*gfxPos)++, auxPalette);
            }
        }
    } else {
        lutMode = G_TT_NONE;
    }

    // only the following aux combine modes are ever used:
    // (A) 0x00 -> 0, 0
    // (B) 0x08 -> 2, 0
    // (C) 0x0D -> 3, 1
    // (D) 0x10 -> 4, 0
    texCombineType = header->auxCombineType;
    if (texCombineType >= 3) {
        // combine modes 3, 4, ... are directly appended to the end of the table and subtype is ignored
        texCombineType = TEX_COMBINE_3 + (texCombineType - 3);
    } else {
        // select based on aux combine subtypes
        // in practice, auxCombineSubType is ALWAYS zero here since the only (A) and (B) may reach this block
        texCombineType = 1 + header->extraTiles * AUX_COMBINE_SUB_COUNT + header->auxCombineSubType;
    }

    *(*gfxPos) = SolidCombineModes[texCombineType][TINT_COMBINE_NONE];
    (*gfxPos)++;

    switch (extraTileType) {
        case EXTRA_TILE_NONE:
            lodMode = G_TL_TILE;
            gSPTexture((*gfxPos)++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            switch (mainBitDepth) {
                case G_IM_SIZ_4b:
                    gDPLoadTextureBlock_4b((*gfxPos)++, raster, mainFmt,
                                           mainWidth, mainHeight, 0,
                                           mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case G_IM_SIZ_8b:
                    gDPLoadTextureBlock((*gfxPos)++, raster, mainFmt, G_IM_SIZ_8b,
                                        mainWidth, mainHeight, 0,
                                        mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case G_IM_SIZ_16b:
                    gDPLoadTextureBlock((*gfxPos)++, raster, mainFmt, G_IM_SIZ_16b,
                                        mainWidth, mainHeight, 0,
                                        mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    break;
                case 3:
                    gDPLoadTextureBlock((*gfxPos)++, raster, mainFmt, G_IM_SIZ_32b,
                                        mainWidth, mainHeight, 0,
                                        mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    break;
            }
            break;
        case EXTRA_TILE_MIPMAPS:
            lodMode = G_TL_LOD;
            switch (mainBitDepth) {
                case G_IM_SIZ_4b:
                    for (rasterPtr = raster, lod = 0, lodDivisor = 1;
                         mainWidth / lodDivisor * 4 >= 64 && mainHeight / lodDivisor != 0;
                         rasterPtr += mainWidth / lodDivisor * mainHeight / lodDivisor / 2, lodDivisor *= 2, lod++)
                    {
                        gDPLoadMultiTile_4b((*gfxPos)++, rasterPtr, (u32)(rasterPtr - raster) >> 3, lod, mainFmt,
                                            mainWidth / lodDivisor, mainHeight / lodDivisor,
                                            0, 0, mainWidth / lodDivisor - 1, mainHeight / lodDivisor - 1, 0,
                                            mainWrapW, mainWrapH, mainMasks - lod, mainMaskt - lod, lod, lod);
                    }
                    break;
                case G_IM_SIZ_8b:
                    for (rasterPtr = raster, lod = 0, lodDivisor = 1;
                         mainWidth / lodDivisor * 8 >= 64 && mainHeight / lodDivisor != 0;
                         rasterPtr += mainWidth / lodDivisor * mainHeight / lodDivisor, lodDivisor *= 2, lod++)
                    {
                        gDPLoadMultiTile((*gfxPos)++, rasterPtr, ((u32)(rasterPtr - raster)) >> 3, lod, mainFmt, G_IM_SIZ_8b,
                                         mainWidth / lodDivisor, mainHeight / lodDivisor,
                                         0, 0, mainWidth / lodDivisor - 1, mainHeight / lodDivisor - 1, 0,
                                         mainWrapW, mainWrapH, mainMasks - lod, mainMaskt - lod, lod, lod);
                    }
                    break;
                case G_IM_SIZ_16b:
                    for (rasterPtr = raster, lod = 0, lodDivisor = 1;
                         mainWidth / lodDivisor * 16 >= 64 && mainHeight / lodDivisor != 0;
                         rasterPtr += mainWidth / lodDivisor * mainHeight / lodDivisor * 2, lodDivisor *= 2, lod++)
                    {
                        gDPLoadMultiTile((*gfxPos)++, rasterPtr, ((u32)(rasterPtr - raster)) >> 3, lod, mainFmt, G_IM_SIZ_16b,
                                         mainWidth / lodDivisor, mainHeight / lodDivisor,
                                         0, 0, mainWidth / lodDivisor - 1, mainHeight / lodDivisor - 1, 0,
                                         mainWrapW, mainWrapH, mainMasks - lod, mainMaskt - lod, lod, lod);
                    }
                    break;
                case G_IM_SIZ_32b:
                    for (rasterPtr = raster, lod = 0, lodDivisor = 1;
                         mainWidth / lodDivisor * 32 >= 64 && mainHeight / lodDivisor != 0;
                         rasterPtr += mainWidth / lodDivisor * mainHeight / lodDivisor * 4, lodDivisor *= 2, lod++)
                    {
                        gDPLoadMultiTile((*gfxPos)++, rasterPtr, ((u32)(rasterPtr - raster)) >> 4, lod, mainFmt, G_IM_SIZ_32b,
                                         mainWidth / lodDivisor, mainHeight / lodDivisor,
                                         0, 0, mainWidth / lodDivisor - 1, mainHeight / lodDivisor - 1, 0,
                                         mainWrapW, mainWrapH, mainMasks - lod, mainMaskt - lod, lod, lod);
                    }
                    break;
            }
            gSPTexture((*gfxPos)++, 0xFFFF, 0xFFFF, lod - 1, G_TX_RENDERTILE, G_ON);
            break;
        case EXTRA_TILE_AUX_SAME_AS_MAIN:
            gSPTexture((*gfxPos)++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            gDPPipeSync((*gfxPos)++);
            lodMode = G_TL_TILE;
            switch (mainBitDepth) {
                case G_IM_SIZ_4b:
                    gDPScrollTextureBlockHalfHeight_4b((*gfxPos)++, raster, mainFmt, mainWidth, mainHeight, 0,
                                                       mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD,
                                                       auxOffsetS, auxOffsetT, auxShiftS, auxShiftT);
                    break;
                case G_IM_SIZ_8b:
                    gDPScrollTextureBlockHalfHeight((*gfxPos)++, raster, mainFmt, G_IM_SIZ_8b, mainWidth, mainHeight, 0,
                                                    mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD,
                                                    auxOffsetS, auxOffsetT, auxShiftS, auxShiftT);
                    break;
                case G_IM_SIZ_16b:
                    gDPScrollTextureBlockHalfHeight((*gfxPos)++, raster, mainFmt, G_IM_SIZ_16b, mainWidth, mainHeight, 0,
                                                    mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD,
                                                    auxOffsetS, auxOffsetT, auxShiftS, auxShiftT);
                    break;
                case G_IM_SIZ_32b:
                    gDPScrollTextureBlockHalfHeight((*gfxPos)++, raster, mainFmt, G_IM_SIZ_32b, mainWidth, mainHeight, 0,
                                                    mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD,
                                                    auxOffsetS, auxOffsetT, auxShiftS, auxShiftT);
                    break;
            }
            break;
        case EXTRA_TILE_AUX_INDEPENDENT:
            gSPTexture((*gfxPos)++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            lodMode = G_TL_TILE;
            switch (mainBitDepth) {
                case G_IM_SIZ_4b:
                    gDPLoadTextureTile_4b((*gfxPos)++, raster, mainFmt, mainWidth, mainHeight,
                                          0, 0, mainWidth - 1, mainHeight - 1, 0,
                                          mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    lodDivisor = (((mainWidth * mainHeight) >> 1) + 7)>>3; // required to use lodDivisor here
                    break;
                case G_IM_SIZ_8b:
                    gDPLoadTextureTile((*gfxPos)++, raster, mainFmt, G_IM_SIZ_8b, mainWidth, mainHeight,
                                       0, 0, mainWidth - 1, mainHeight - 1, 0,
                                       mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    lodDivisor = ((mainWidth * mainHeight) + 7)>>3;
                    break;
                case G_IM_SIZ_16b:
                    gDPLoadTextureTile((*gfxPos)++, raster, mainFmt, G_IM_SIZ_16b, mainWidth, mainHeight,
                                       0, 0, mainWidth - 1, mainHeight - 1, 0,
                                       mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    lodDivisor = ((mainWidth * mainHeight) * 2 + 7)>>3;
                    break;
                case G_IM_SIZ_32b:
                    gDPLoadTextureTile((*gfxPos)++, raster, mainFmt, G_IM_SIZ_32b, mainWidth, mainHeight,
                                       0, 0, mainWidth - 1, mainHeight - 1, 0,
                                       mainWrapW, mainWrapH, mainMasks, mainMaskt, G_TX_NOLOD, G_TX_NOLOD);
                    lodDivisor = ((mainWidth * mainHeight / 2) * 2 + 7)>>3;
                    break;
            }

            switch (auxBitDepth) {
                case G_IM_SIZ_4b:
                    gDPScrollMultiTile_4b((*gfxPos)++, auxRaster, lodDivisor, 1, auxFmt, auxWidth, auxHeight,
                                          0, 0, auxWidth - 1, auxHeight - 1, auxPaletteIndex,
                                          auxWrapW, auxWrapH, auxMasks, auxMaskt,
                                          auxShiftS, auxShiftT, auxOffsetS, auxOffsetT);
                    break;
                case G_IM_SIZ_8b:
                    gDPScrollMultiTile((*gfxPos)++, auxRaster, lodDivisor, 1, auxFmt, G_IM_SIZ_8b, auxWidth, auxHeight,
                                       0, 0, auxWidth - 1, auxHeight - 1, auxPaletteIndex,
                                       auxWrapW, auxWrapH, auxMasks, auxMaskt,
                                       auxShiftS, auxShiftT, auxOffsetS, auxOffsetT);
                    break;
                case G_IM_SIZ_16b:
                    gDPScrollMultiTile((*gfxPos)++, auxRaster, lodDivisor, 1, auxFmt, G_IM_SIZ_16b, auxWidth, auxHeight,
                                       0, 0, auxWidth - 1, auxHeight - 1, auxPaletteIndex,
                                       auxWrapW, auxWrapH, auxMasks, auxMaskt,
                                       auxShiftS, auxShiftT, auxOffsetS, auxOffsetT);
                    break;
                case G_IM_SIZ_32b:
                    gDPScrollMultiTile((*gfxPos)++, auxRaster, lodDivisor, 1, auxFmt, G_IM_SIZ_32b, auxWidth, auxHeight,
                                       0, 0, auxWidth - 1, auxHeight - 1, auxPaletteIndex,
                                       auxWrapW, auxWrapH, auxMasks, auxMaskt,
                                       auxShiftS, auxShiftT, auxOffsetS, auxOffsetT);
                    break;
            }
    }
    gSPSetOtherMode((*gfxPos)++, G_SETOTHERMODE_H, 4, 16, filteringMode | G_TC_FILT | lutMode | lodMode | G_TP_PERSP );
}

Model* get_model_from_list_index(s32 listIndex) {
    return (*gCurrentModels)[listIndex];
}

void load_data_for_models(ModelNode* rootModel, s32 texturesOffset, s32 size) {
    Matrix4f mtx;

    guMtxIdentF(mtx);

    if (texturesOffset != 0) {
        mdl_load_all_textures(rootModel, texturesOffset, size);
    }

    *gCurrentModelTreeRoot = rootModel;
    TreeIterPos = 0;

    if (rootModel != NULL) {
        load_model_transforms(rootModel, NULL, mtx, 0);
    }
}

void load_model_transforms(ModelNode* model, ModelNode* parent, Matrix4f mdlTransformMtx, s32 treeDepth) {
    Matrix4f combinedMtx;
    Mtx sp50;
    ModelBlueprint modelBP;
    ModelBlueprint* modelBPptr = &modelBP;
    ModelNodeProperty* prop;
    ModelNode* modelTemp;
    s32 i;

    if (model->groupData != NULL && model->groupData->numChildren != 0) {
        s32 groupType;

        if (model->groupData->transformMatrix != NULL) {
            Matrix4f tempMtx;

            guMtxL2F(tempMtx, model->groupData->transformMatrix);
            guMtxCatF(tempMtx, mdlTransformMtx, combinedMtx);
        }

        prop = get_model_property(model, MODEL_PROP_KEY_GROUP_INFO);
        if (prop == NULL) {
            groupType = GROUP_TYPE_0;
        } else {
            groupType = prop->data.s;
        }

        if (model->type != SHAPE_TYPE_GROUP || groupType == GROUP_TYPE_0) {
            for (i = 0; i < model->groupData->numChildren; i++) {
                load_model_transforms(model->groupData->childList[i], model,
                                      model->groupData->transformMatrix != NULL ? combinedMtx : mdlTransformMtx,
                                      treeDepth + 1);
            }

            (*gCurrentModelTreeNodeInfo)[TreeIterPos].modelIndex = -1;
            (*gCurrentModelTreeNodeInfo)[TreeIterPos].treeDepth = treeDepth;
            TreeIterPos++;
            return;
        }
    }

    guMtxF2L(mdlTransformMtx, &sp50);
    modelBPptr->flags = 0;
    modelBPptr->mdlNode = model;
    modelBPptr->groupData = parent->groupData;
    modelBPptr->mtx = &sp50;

    if (model->type == SHAPE_TYPE_GROUP) {
        s32 childCount = mdl_get_child_count(model);

        for (i = TreeIterPos; i < TreeIterPos + childCount; i++) {
            (*gCurrentModelTreeNodeInfo)[i].modelIndex = -1;
            (*gCurrentModelTreeNodeInfo)[i].treeDepth = treeDepth + 1;
        }
        TreeIterPos += childCount;
    }

    mdl_create_model(modelBPptr, 4);
    (*gCurrentModelTreeNodeInfo)[TreeIterPos].treeDepth = treeDepth;
    TreeIterPos++;
}

s32 get_model_list_index_from_tree_index(s32 treeIndex) {
    s32 i;

    if (treeIndex < MAX_MODELS) {
        u8 modelIndex = (*gCurrentModelTreeNodeInfo)[treeIndex].modelIndex;

        if (modelIndex != (u8)-1) {
            return modelIndex;
        }
    }

    for (i = 0; i < MAX_MODELS; i++) {
        Model* model = get_model_from_list_index(i);

        if (model != NULL && model->modelID == treeIndex) {
            return i;
        }
    }
    return 0;
}

s32 get_transform_group_index(s32 modelID) {
    ModelTransformGroup* group;
    s32 i;

    for (i = 0; i < MAX_MODEL_TRANSFORM_GROUPS; i++) {
        group = get_transform_group(i);

        if (group != NULL && group->groupModelID == modelID) {
            return i;
        }
    }

    return -1;
}

void get_model_center_and_size(u16 modelID, f32* centerX, f32* centerY, f32* centerZ, f32* sizeX, f32* sizeY, f32* sizeZ) {
    Model* model = get_model_from_list_index(get_model_list_index_from_tree_index(modelID));
    ModelNode* node = model->modelNode;
    ModelBoundingBox* bb;

    *centerX = model->center.x;
    *centerY = model->center.y;
    *centerZ = model->center.z;

    bb = (ModelBoundingBox*)get_model_property(node, MODEL_PROP_KEY_BOUNDING_BOX);

    if (bb != NULL) {
        *sizeX = bb->halfSizeX;
        *sizeY = bb->halfSizeY;
        *sizeZ = bb->halfSizeZ;
    } else {
        *sizeX = *sizeY = *sizeZ = 0.0f;
    }
}

ModelTransformGroup* get_transform_group(s32 index) {
    return (*gCurrentTransformGroups)[index];
}

// find group info?
void func_8011B1D8(ModelNode* node) {
    ModelNode* currentNode;
    ModelNodeProperty* prop;
    s32 numChildren;
    s32 i;
    u16 currentID;

    // stop searching if node is a model
    if (node->type == SHAPE_TYPE_MODEL) {
        mtg_MaxChild = TreeIterPos;
        return;
    }

    // stop searching if node is a group with GROUP_TYPE_0
    if (node->type == SHAPE_TYPE_GROUP) {
        prop = get_model_property(node, MODEL_PROP_KEY_GROUP_INFO);
        if (prop != NULL && prop->data.s != GROUP_TYPE_0) {
            TreeIterPos += mdl_get_child_count(node);
            mtg_MaxChild = TreeIterPos;
            return;
        }
    }

    if (node->groupData != NULL) {
        numChildren = node->groupData->numChildren;
        if (numChildren != 0) {
            for (i = 0; i < numChildren; i++) {
                currentNode = node->groupData->childList[i];
                currentID = TreeIterPos;

                if (currentNode->type == SHAPE_TYPE_GROUP) {
                    prop = get_model_property(currentNode, MODEL_PROP_KEY_GROUP_INFO);
                    if (prop != NULL && prop->data.s != GROUP_TYPE_0) {
                        currentID += mdl_get_child_count(currentNode);
                    }
                }
                func_8011B1D8(currentNode);

                if (mtg_FoundModelNode != NULL) {
                    // not possible
                    return;
                }

                // the current model is the one we're looking for
                if (mtg_SearchModelID == TreeIterPos) {
                    mtg_FoundModelNode = currentNode;
                    mtg_MinChild = currentID;
                    return;
                }

                TreeIterPos++;
            }
        }
    }
}

void mdl_make_transform_group(u16 modelID) {
    TreeIterPos = 0;
    mtg_FoundModelNode = NULL;
    mtg_SearchModelID = modelID;
    mtg_MaxChild = 0;
    mtg_MinChild = 0;
    func_8011B1D8(*gCurrentModelTreeRoot);

    if (mtg_FoundModelNode != NULL) {
        ModelTransformGroup* newMtg;
        ModelNode* node;
        ModelNodeProperty* prop;
        ModelBoundingBox* bb;
        f32 x, y, z;
        s32 i;

        for (i = 0; i < ARRAY_COUNT(*gCurrentTransformGroups); i++) {
            if ((*gCurrentTransformGroups)[i] == NULL) {
                break;
            }
        }

        (*gCurrentTransformGroups)[i] = newMtg = heap_malloc(sizeof(*newMtg));
        newMtg->flags = TRANSFORM_GROUP_FLAG_VALID;
        newMtg->groupModelID = modelID;
        newMtg->minChildModelIndex = get_model_list_index_from_tree_index(mtg_MinChild);
        newMtg->maxChildModelIndex = get_model_list_index_from_tree_index(mtg_MaxChild);
        newMtg->matrixFreshness = 0;
        newMtg->bakedMtx = NULL;
        newMtg->baseModelNode = mtg_FoundModelNode;
        guMtxIdent(&newMtg->savedMtx);
        newMtg->flags |= TRANSFORM_GROUP_FLAG_IGNORE_MATRIX;
        guMtxIdentF(newMtg->userTransformMtx);

        node = newMtg->baseModelNode;

        if (node->type != SHAPE_TYPE_GROUP) {
            prop = get_model_property(node, MODEL_PROP_KEY_RENDER_MODE);
        } else {
            prop = get_model_property(node, MODEL_PROP_KEY_GROUP_INFO);

            if (prop != NULL) {
                // GROUP_INFO properties always come in pairs, with the second giving the render mode
                prop++;
            }
        }

        if (prop != NULL) {
            newMtg->renderMode = prop->data.s;
        } else {
            newMtg->renderMode = RENDER_MODE_SURFACE_OPA;
        }

        bb = (ModelBoundingBox*)get_model_property(node, MODEL_PROP_KEY_BOUNDING_BOX);
        if (bb != NULL) {
            x = (bb->minX + bb->maxX) * 0.5f;
            y = (bb->minY + bb->maxY) * 0.5f;
            z = (bb->minZ + bb->maxZ) * 0.5f;
        } else {
            x = y = z = 0.0f;
        }

        if (newMtg->bakedMtx != NULL) {
            guMtxXFML(newMtg->bakedMtx, x, y, z, &x, &y, &z);
        }

        newMtg->center.x = x;
        newMtg->center.y = y;
        newMtg->center.z = z;
        enable_transform_group(modelID);
    }
}

void enable_transform_group(u16 modelID) {
    ModelTransformGroup* group = get_transform_group(get_transform_group_index(modelID));
    s32 i;

    group->flags &= ~TRANSFORM_GROUP_FLAG_INACTIVE;

    for (i = group->minChildModelIndex; i <= group->maxChildModelIndex; i++) {
        Model* model = get_model_from_list_index(i);

        model->flags |= MODEL_FLAG_TRANSFORM_GROUP_MEMBER;

        if (model->bakedMtx != NULL) {
            model->flags |= MODEL_FLAG_MATRIX_DIRTY;
        }
    }
}

void disable_transform_group(u16 modelID) {
    ModelTransformGroup* group = get_transform_group(get_transform_group_index(modelID));
    s32 i;

    group->flags |= TRANSFORM_GROUP_FLAG_INACTIVE;

    for (i = group->minChildModelIndex; i <= group->maxChildModelIndex; i++) {
        Model* model = get_model_from_list_index(i);

        model->flags &= ~MODEL_FLAG_TRANSFORM_GROUP_MEMBER;

        if (model->bakedMtx != NULL) {
            model->flags |= MODEL_FLAG_MATRIX_DIRTY;
        }
    }
}

void clone_model(u16 srcModelID, u16 newModelID) {
    Model* srcModel = get_model_from_list_index(get_model_list_index_from_tree_index(srcModelID));
    Model* newModel;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(*gCurrentModels); i++) {
        if ((*gCurrentModels)[i] == NULL) {
            break;
        }
    }

    (*gCurrentModels)[i] = newModel = heap_malloc(sizeof(*newModel));
    *newModel = *srcModel;
    newModel->modelID = newModelID;
}

void mdl_group_set_visibility(u16 treeIndex, s32 flags, s32 mode) {
    s32 maxGroupIndex = -1;
    s32 minGroupIndex;
    s32 modelIndex = (*gCurrentModelTreeNodeInfo)[treeIndex].modelIndex;
    s32 siblingIndex;
    s32 i;

    if (modelIndex < MAX_MODELS - 1) {
        minGroupIndex = maxGroupIndex = modelIndex;
    } else {
        s32 treeDepth = (*gCurrentModelTreeNodeInfo)[treeIndex].treeDepth;
        for (i = treeIndex - 1; i >= 0; i--) {
            if ((*gCurrentModelTreeNodeInfo)[i].treeDepth <= treeDepth) {
                break;
            }

            siblingIndex = (*gCurrentModelTreeNodeInfo)[i].modelIndex;

            if (siblingIndex < MAX_MODELS - 1) {
                if (maxGroupIndex == -1) {
                    maxGroupIndex = siblingIndex;
                }
                minGroupIndex = siblingIndex;
            }
        }
    }

    if (mode < 2) {
        for (i = minGroupIndex; i <= maxGroupIndex; i++) {
            Model* model = (*gCurrentModels)[i];
            if (mode != MODEL_GROUP_HIDDEN) {
                model->flags &= ~flags;
            } else {
                model->flags |= flags;
            }
        }
    } else {
        for (i = 0; i < minGroupIndex; i++) {
            Model* model = (*gCurrentModels)[i];
            if (mode == MODEL_GROUP_OTHERS_VISIBLE) {
                model->flags &= ~flags;
            } else {
                model->flags |= flags;
            }
        }
        for (i = maxGroupIndex + 1; i < MAX_MODELS; i++) {
            Model* model = (*gCurrentModels)[i];
            if (model != NULL) {
                if (mode == MODEL_GROUP_OTHERS_VISIBLE) {
                    model->flags &= ~flags;
                } else {
                    model->flags |= flags;
                }
            }
        }
    }
}

void mdl_group_set_custom_gfx(u16 groupModelID, s32 customGfxIndex, s32 tintType, b32 invertSelection) {
    s32 maxGroupIndex = -1;
    s32 i;
    s32 minGroupIndex;
    s32 modelIndex = (*gCurrentModelTreeNodeInfo)[groupModelID].modelIndex;
    s32 siblingIndex;
    s32 maskLow, maskHigh, packed;

    if (modelIndex < MAX_MODELS - 1) {
        minGroupIndex = maxGroupIndex = modelIndex;
    } else {
        s32 treeDepth = (*gCurrentModelTreeNodeInfo)[groupModelID].treeDepth;
        for (i = groupModelID - 1; i >= 0; i--) {
            if ((*gCurrentModelTreeNodeInfo)[i].treeDepth <= treeDepth) {
                break;
            }

            siblingIndex = (*gCurrentModelTreeNodeInfo)[i].modelIndex;

            if (siblingIndex < MAX_MODELS - 1) {
                if (maxGroupIndex == -1) {
                    maxGroupIndex = siblingIndex;
                }
                minGroupIndex = siblingIndex;
            }
        }
    }

    maskLow = maskHigh = 0;

    if (customGfxIndex < 0) {
        maskLow = 0xF;
        customGfxIndex = 0;
    }

    if (tintType < 0) {
        maskHigh = 0xF0;
        tintType = 0;
    }

    packed = customGfxIndex + (tintType << 4);

    if (!invertSelection) {
        for (i = minGroupIndex; i <= maxGroupIndex; i++) {
            Model* model = (*gCurrentModels)[i];
            model->customGfxIndex = (model->customGfxIndex & (maskLow + maskHigh)) + packed;
        }
    } else {
        for (i = 0; i < minGroupIndex; i++) {
            Model* model = (*gCurrentModels)[i];
            model->customGfxIndex = (model->customGfxIndex & (maskLow + maskHigh)) + packed;
        }
        for (i = maxGroupIndex + 1; i < MAX_MODELS; i++) {
            Model* model = (*gCurrentModels)[i];
            if (model != NULL) {
                model->customGfxIndex = (model->customGfxIndex & (maskLow + maskHigh)) + packed;
            }
        }
    }
}

void mdl_reset_transform_flags(void) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(*gCurrentModels); i++) {
        Model* model = (*gCurrentModels)[i];

        if (model != NULL) {
            model->flags &= ~MODEL_FLAG_HAS_TRANSFORM;
        }
    }

    for (i = 0; i < ARRAY_COUNT(*gCurrentTransformGroups); i++) {
        ModelTransformGroup* transformGroup = (*gCurrentTransformGroups)[i];

        if (transformGroup != NULL) {
            transformGroup->flags &= ~TRANSFORM_GROUP_FLAG_HAS_TRANSFORM;
        }
    }
}

void enable_world_fog(void) {
    gFogSettings->enabled = TRUE;
}

void disable_world_fog(void) {
    gFogSettings->enabled = FALSE;
}

void set_world_fog_dist(s32 start, s32 end) {
    gFogSettings->startDistance = start;
    gFogSettings->endDistance = end;
}

void set_world_fog_color(s32 r, s32 g, s32 b, s32 a) {
    gFogSettings->color.r = r;
    gFogSettings->color.g = g;
    gFogSettings->color.b = b;
    gFogSettings->color.a = a;
}

s32 is_world_fog_enabled(void) {
    return gFogSettings->enabled;
}

void get_world_fog_distance(s32* start, s32* end) {
    *start = gFogSettings->startDistance;
    *end = gFogSettings->endDistance;
}

void get_world_fog_color(s32* r, s32* g, s32* b, s32* a) {
    *r = gFogSettings->color.r;
    *g = gFogSettings->color.g;
    *b = gFogSettings->color.b;
    *a = gFogSettings->color.a;
}

void set_tex_panner(Model* model, s32 texPannerID) {
    model->texPannerID = texPannerID;
}

void set_main_pan_u(s32 texPannerID, s32 value) {
    texPannerMainU[texPannerID] = value;
}

void set_main_pan_v(s32 texPannerID, s32 value) {
    texPannerMainV[texPannerID] = value;
}

void set_aux_pan_u(s32 texPannerID, s32 value) {
    texPannerAuxU[texPannerID] = value;
}

void set_aux_pan_v(s32 texPannerID, s32 value) {
    texPannerAuxV[texPannerID] = value;
}

void set_mdl_custom_gfx_set(Model* model, s32 customGfxIndex, u32 tintType) {
    if (customGfxIndex == -1) {
        customGfxIndex = model->customGfxIndex & 0xF;
    }

    if (tintType == -1) {
        tintType = model->customGfxIndex >> 4;
    }

    model->customGfxIndex = (customGfxIndex & 0xF) + ((tintType & 0xF) << 4);
}

void set_custom_gfx(s32 customGfxIndex, Gfx* pre, Gfx* post) {
    (*gCurrentCustomModelGfxPtr)[customGfxIndex * 2] = pre;
    (*gCurrentCustomModelGfxPtr)[customGfxIndex * 2 + 1] = post;
}

void set_custom_gfx_builders(s32 customGfxIndex, ModelCustomGfxBuilderFunc pre, ModelCustomGfxBuilderFunc post) {
    (*gCurrentCustomModelGfxBuildersPtr)[customGfxIndex * 2] = pre;
    (*gCurrentCustomModelGfxBuildersPtr)[customGfxIndex * 2 + 1] = post;
}

void build_custom_gfx(void) {
    Gfx* gfx = gMainGfxPos;
    ModelCustomGfxBuilderFunc preFunc;
    ModelCustomGfxBuilderFunc postFunc;
    s32 i;

    // placeholder branch
    gSPBranchList(gMainGfxPos++, 0x00000000);

    for (i = 0; i < ARRAY_COUNT(*gCurrentCustomModelGfxPtr) / 2; i++) {
        preFunc = (*gCurrentCustomModelGfxBuildersPtr)[i * 2];

        if (preFunc != NULL) {
            (*gCurrentCustomModelGfxPtr)[i * 2] = gMainGfxPos;
            preFunc(i);
            gSPEndDisplayList(gMainGfxPos++);
        }

        postFunc = (*gCurrentCustomModelGfxBuildersPtr)[i * 2 + 1];
        if (postFunc != NULL) {
            (*gCurrentCustomModelGfxPtr)[i * 2 + 1] = gMainGfxPos;
            postFunc(i);
            gSPEndDisplayList(gMainGfxPos++);
        }
    }

    // overwrite placeholder with final branch address
    gSPBranchList(gfx, gMainGfxPos);
}

// weird temps necessary to match
/// @returns TRUE if mtx is NULL or identity.
s32 is_identity_fixed_mtx(Mtx* mtx) {
    s32* mtxIt = (s32*)mtx;
    s32* identityIt;
    s32 i;

    if (mtx == NULL) {
        return TRUE;
    }

    identityIt = (s32*)&ReferenceIdentityMtx;

    for (i = 0; i < 16; i++, mtxIt++, identityIt++) {
        if (*mtxIt != *identityIt) {
            return FALSE;
        }
    }

    return TRUE;
}

void mdl_set_shroud_tint_params(u8 r, u8 g, u8 b, u8 a) {
    ShroudTintR = r;
    ShroudTintG = g;
    ShroudTintB = b;
    ShroudTintAmt = a;
}

void mdl_get_shroud_tint_params(u8* r, u8* g, u8* b, u8* a) {
    *r = ShroudTintR;
    *g = ShroudTintG;
    *b = ShroudTintB;
    *a = ShroudTintAmt;
}

void mdl_set_depth_tint_params(u8 primR, u8 primG, u8 primB, u8 primA, u8 fogR, u8 fogG, u8 fogB, s32 fogStart, s32 fogEnd) {
    DepthTintBaseR = primR;
    DepthTintBaseG = primG;
    DepthTintBaseB = primB;
    DepthTintBaseA = primA;
    DepthTintColR = fogR;
    DepthTintColG = fogG;
    DepthTintColB = fogB;
    DepthTintStart = fogStart;
    DepthTintEnd = fogEnd;
}

void mdl_get_depth_tint_params(u8* primR, u8* primG, u8* primB, u8* primA, u8* fogR, u8* fogG, u8* fogB,
                                    s32* fogStart, s32* fogEnd) {
    *primR = DepthTintBaseR;
    *primG = DepthTintBaseG;
    *primB = DepthTintBaseB;
    *primA = DepthTintBaseA;
    *fogR = DepthTintColR;
    *fogG = DepthTintColG;
    *fogB = DepthTintColB;
    *fogStart = DepthTintStart;
    *fogEnd = DepthTintEnd;
}

void mdl_set_remap_tint_params(u8 maxR, u8 maxG, u8 maxB, u8 minR, u8 minG, u8 minB) {
    RemapTintMaxR = maxR;
    RemapTintMaxG = maxG;
    RemapTintMaxB = maxB;
    RemapTintMinR = minR;
    RemapTintMinG = minG;
    RemapTintMinB = minB;
}

void mdl_get_remap_tint_params(u8* primR, u8* primG, u8* primB, u8* envR, u8* envG, u8* envB) {
    *primR = RemapTintMaxR;
    *primG = RemapTintMaxG;
    *primB = RemapTintMaxB;
    *envR = RemapTintMinR;
    *envG = RemapTintMinG;
    *envB = RemapTintMinB;
}

void mdl_get_vertex_count(Gfx* gfx, s32* numVertices, Vtx** baseVtx, s32* gfxCount, Vtx* baseAddr) {
    s8 stuff[2];

    s32 vtxCount;
    u32 w0, w1;
    u32 cmd;
    u32 vtxEndAddr;
    s32 minVtx;
    s32 maxVtx;
    u32 vtxStartAddr;

    minVtx = 0;
    maxVtx = 0;

    if (gfx == NULL) {
        *numVertices = 0;
        *baseVtx = NULL;
    } else {
        Gfx* baseGfx = gfx;

        do {
            w0 = gfx->words.w0;
            w1 = gfx->words.w1;
            cmd = _SHIFTR(w0,24,8);

            if (cmd == G_VTX) {
                vtxStartAddr = w1;
                if (baseAddr != NULL) {
                    vtxStartAddr = (vtxStartAddr & 0xFFFF) + (s32)baseAddr;
                }
                vtxCount = _SHIFTR(w0,12,8);
                if (minVtx == 0) {
                    minVtx = vtxStartAddr;
                    maxVtx = vtxStartAddr + (vtxCount * sizeof(Vtx));
                }
                vtxEndAddr = vtxStartAddr + (vtxCount * sizeof(Vtx));
                if (maxVtx < vtxEndAddr) {
                    maxVtx = vtxEndAddr;
                }
                if (minVtx > vtxEndAddr) {
                    minVtx = vtxEndAddr;
                }
            }
            gfx++;
        } while (cmd != G_ENDDL);

        *numVertices = (maxVtx - minVtx) >> 4;
        *baseVtx = (Vtx*)minVtx;
        *gfxCount = gfx - baseGfx;
        w1 = 64; // TODO required to match -- can be any operation that stores w1
    }
}

void mdl_local_gfx_update_vtx_pointers(Gfx *nodeDlist, Vtx *baseVtx, Gfx *arg2, Vtx *arg3) {
    u32 w0;
    Vtx* w1;
    do {
        w0 = (*((unsigned long long*)nodeDlist)) >> 0x20; // TODO required to match
        w1 = (Vtx*)nodeDlist->words.w1;
        if (w0 >> 0x18 == G_VTX) {
            w1 = arg3 + (w1 - baseVtx);
        }
        arg2->words.w0 = w0;
        arg2->words.w1 = (u32)w1;
        nodeDlist++;
        arg2++;
    } while (w0 >> 0x18 != G_ENDDL);
}

void mdl_local_gfx_copy_vertices(Vtx* src, s32 num, Vtx* dest) {
    u32 i;

    for (i = 0; i < num * sizeof(*src); i++) {
        ((u8*)dest)[i] = ((u8*)src)[i];
    }
}

void mdl_make_local_vertex_copy(s32 copyIndex, u16 modelID, s32 isMakingCopy) {
    s32 numVertices;
    Vtx* baseVtx;
    s32 gfxCount;
    Gfx* nodeDlist;
    Model* model;
    ModelLocalVertexCopy* copy;
    s32 i;

    model = get_model_from_list_index(get_model_list_index_from_tree_index(modelID));
    nodeDlist = model->modelNode->displayData->displayList;
    mdl_get_vertex_count(nodeDlist, &numVertices, &baseVtx, &gfxCount, NULL);

    copy = (*gCurrentModelLocalVtxBuffers)[copyIndex] = heap_malloc(sizeof(*copy));

    if (isMakingCopy) {
        for (i = 0; i < ARRAY_COUNT(copy->gfxCopy); i++) {
            copy->gfxCopy[i] = heap_malloc(gfxCount * sizeof(*copy->gfxCopy[i]));
            copy->vtxCopy[i] = heap_malloc(numVertices * sizeof(*copy->vtxCopy[i]));
            mdl_local_gfx_update_vtx_pointers(nodeDlist, baseVtx, copy->gfxCopy[i], copy->vtxCopy[i]);
            mdl_local_gfx_copy_vertices(baseVtx, numVertices, copy->vtxCopy[i]);
        }
        model->flags |= MODEL_FLAG_HAS_LOCAL_VERTEX_COPY;
    } else {
        for (i = 0; i < ARRAY_COUNT(copy->gfxCopy); i++) {
            copy->gfxCopy[i] = NULL;
            copy->vtxCopy[i] = NULL;
        }
        model->flags |= MODEL_FLAG_HIDDEN;
    }

    copy->selector = 0;
    copy->numVertices = numVertices;
    copy->minVertexAddr = baseVtx;
}

void mdl_get_copied_vertices(s32 copyIndex, Vtx** firstVertex, Vtx** copiedVertices, s32* numCopied) {
    ModelLocalVertexCopy* mlvc = (*gCurrentModelLocalVtxBuffers)[copyIndex];
    s32 selector = mlvc->selector;

    *firstVertex = mlvc->minVertexAddr;
    *copiedVertices = mlvc->vtxCopy[selector];
    *numCopied = mlvc->numVertices;
}

Gfx* mdl_get_copied_gfx(s32 copyIndex) {
    ModelLocalVertexCopy* mlvc = (*gCurrentModelLocalVtxBuffers)[copyIndex];
    s32 selector = mlvc->selector;
    Gfx* gfxCopy = mlvc->gfxCopy[selector];

    mlvc->selector++;
    if (mlvc->selector > ARRAY_COUNT(mlvc->gfxCopy) - 1) {
        mlvc->selector = 0;
    }

    return gfxCopy;
}

void mdl_project_tex_coords(s32 modelID, Gfx* outGfx, Matrix4f arg2, Vtx* arg3) {
    s32 sp18;
    Vtx* baseVtx;
    s32 sp20;
    f32 v1tc1;
    f32 v2tc1;
    f32 sp2C;
    f32 v0tc1;
    f32 sp40;
    f32 v1tc0;
    f32 v1ob2;
    f32 ob2;
    f32 ob1;
    f32 v0ob0;
    f32 v0ob2;
    f32 v0tc0;
    f32 v2ob0;
    f32 v2tc0;
    f32 v1ob0;
    f32 v2ob2;
    f32 ob0;
    f32 var_f10;
    f32 var_f24;
    f32 var_f26;
    f32 tc1;
    f32 var_f30;
    f32 tc0;
    f32 var_f6_2;
    s32 i;
    u32 cnB;
    u32 cnG;
    u32 cnR;
    f32 var_f20;

    s32 listIndex;
    Model* model;
    Gfx* dlist;
    s32 cmd;
    Vtx* tempVert;

    s8 zero = 0; // TODO needed to match

    listIndex = get_model_list_index_from_tree_index(modelID & 0xFFFF);
    model = get_model_from_list_index(listIndex);
    dlist = model->modelNode->displayData->displayList;

    while (TRUE) {
        cmd = dlist->words.w0 >> 0x18;
        tempVert = (Vtx*)dlist->words.w1;
        if (cmd == G_ENDDL) {
            break;
        }
        if (cmd == G_VTX) {
            baseVtx = tempVert;
            break;
        }
        dlist++;
    }

    v0ob0 = baseVtx[zero].v.ob[0];
    v0ob2 = baseVtx[zero].v.ob[2];
    v0tc0 = baseVtx[zero].v.tc[0];
    v0tc1 = baseVtx[zero].v.tc[1];

    v1ob0 = baseVtx[1].v.ob[0];
    v1ob2 = baseVtx[1].v.ob[2];
    v1tc0 = baseVtx[1].v.tc[0];
    v1tc1 = baseVtx[1].v.tc[1];

    v2ob0 = baseVtx[2].v.ob[0];
    v2ob2 = baseVtx[2].v.ob[2];
    v2tc0 = baseVtx[2].v.tc[0];
    v2tc1 = baseVtx[2].v.tc[1];

    cnR = baseVtx[0].v.cn[0];
    cnG = baseVtx[0].v.cn[1];
    cnB = baseVtx[0].v.cn[2];

    if (v0ob0 != v1ob0) {
        f32 f2 = v0ob0 - v2ob0;
        f32 f14 = v0ob0 - v1ob0;
        f32 f8 = v0tc0 - v1tc0;
        f32 f2a = f2 / f14;
        f32 f0 = f2a * f8;
        f32 f12 = v0ob2 - v1ob2;
        f32 f10 = f2a * f12;
        f32 f4 = v0tc0 - v2tc0;
        f32 f6 = v0ob2 - v2ob2;
        f32 f0a = f0 - f4;
        f32 f10a = f10 - f6;

        f32 f0b, f4a, f2b, f8a, f6a, f0c, f8b, f2c, f12a, f2d, f4b, f0d, f6b, f0e;

        sp40 = f0a / f10a; // used later
        f0b = f12 * sp40;
        f4a = v0tc1 - v1tc1;
        f2b = f2a * f4a;
        f8a = f8 - f0b;
        var_f30 = f8a / f14; // used later
        f6a = var_f30 * v0ob0;
        f0c = v0tc1 - v2tc1;
        f2c = f2b - f0c;
        var_f26 = f2c / f10a; // used later
        f12a = f12 * var_f26;
        var_f24 = (f4a - f12a) / f14; // used later
        sp2C = v0tc0 - f6a - sp40 * v0ob2; // used later
        var_f20 = v0tc1 - var_f24 * v0ob0 - var_f26 * v0ob2; // used later
    } else {
        f32 f2 = v0ob2 - v2ob2;
        f32 f14 = v0ob2 - v1ob2;
        f32 f8 = v0tc0 - v1tc0;
        f32 f12 = v0ob0 - v1ob0;
        f32 f4 = v0tc0 - v2tc0;
        f32 f6 = v0ob0 - v2ob0;
        f32 f0 = f2 / f14 * f8;
        f32 f10 = f2 / f14 * f12;

        f32 f0b, f4a, f2b, f8a, f6a, f0c, f8b, f2c, f12a, f2d, f4b, f0d, f6b, f0e;

        var_f30 = (f0 - f4) / (f10 - f6); // used later
        f0b = f12 * var_f30;
        f6a = var_f30 * v0ob0;
        f4a = v0tc1 - v1tc1;
        f2b = f2 / f14 * f4a;
        f8a = f8 - f0b;
        sp40 = f8a / f14; // used later
        f8b = sp40 * v0ob2;
        f0c = v0tc1 - v2tc1;
        var_f24 = (f2b - f0c) / (f10 - f6); // used later
        var_f26 = (f4a - f12 * var_f24) / f14; // used later
        sp2C = v0tc0 - f6a - f8b; // used later
        var_f20 = v0tc1 - var_f24 * v0ob0 - var_f26 * v0ob2; // used later
    }

    mdl_get_vertex_count(outGfx, &sp18, &baseVtx, &sp20, arg3);

    for (i = 0; i < sp18; i++) {
        ob0 = baseVtx->v.ob[0];
        ob1 = baseVtx->v.ob[1];
        ob2 = baseVtx->v.ob[2];
        if (arg2 != NULL) {
            var_f10 = (arg2[0][0] * ob0) + (arg2[1][0] * ob1) + (arg2[2][0] * ob2) + arg2[3][0];
            var_f6_2 = (arg2[0][2] * ob0) + (arg2[1][2] * ob1) + (arg2[2][2] * ob2) + arg2[3][2];
        } else {
            var_f10 = ob0;
            var_f6_2 = ob2;
        }
        tc0 = (var_f30 * var_f10) + (sp40 * var_f6_2) + sp2C;
        tc1 = (var_f24 * var_f10) + (var_f26 * var_f6_2) + var_f20;
        if (tc0 < 0.0f) {
            tc0 -= 0.5;
        } else if (tc0 > 0.0f) {
            tc0 += 0.5;
        }

        if (tc1 < 0.0f) {
            tc1 -= 0.5;
        } else if (tc1 > 0.0f) {
            tc1 += 0.5;
        }

        baseVtx->v.tc[0] = tc0;
        baseVtx->v.tc[1] = tc1;
        baseVtx->v.cn[0] = cnR;
        baseVtx->v.cn[1] = cnG;
        baseVtx->v.cn[2] = cnB;
        baseVtx++;
    }
}

// Checks if the center of a model is visible.
// If `depthQueryID` is nonnegative, the depth buffer is checked to see if the model's center is occluded by geometry.
//   Otherwise, the occlusion check is skipped.
// `depthQueryID` must be between 0 and the size of `DepthCopyBuffer` minus 1.
// Every nonnegative value of `depthQueryID` must be unique within a frame, otherwise the result will corrupt the data
//   of the previous query that shared the same ID.
// Occlusion visibility checks are always one frame out of date, as they reference the previous frame's depth buffer.
s32 is_model_center_visible(u16 modelID, s32 depthQueryID, f32* screenX, f32* screenY) {
    Camera* camera = &gCameras[gCurrentCameraID];
    Model* model = get_model_from_list_index(get_model_list_index_from_tree_index(modelID));
    f32 outX;
    f32 outY;
    f32 outZ;
    f32 outW;

    s32 depthExponent;
    s32 depthMantissa;
    u32 shiftedMantissa, mantissaBias;
    u32 decodedDepth;
    s32 scaledDepth;

    // If an invalid depth query id was provided, return false.
    if (depthQueryID >= ARRAY_COUNT(DepthCopyBuffer)) {
        return FALSE;
    }
    // Transform the model's center into clip space.
    transform_point(camera->mtxPerspective, model->center.x, model->center.y, model->center.z, 1.0f, &outX, &outY, &outZ, &outW);
    if (outW == 0.0f) {
        *screenX = 0.0f;
        *screenY = 0.0f;
        return TRUE;
    }
    // Perform the perspective divide (divide xyz by w) to convert to normalized device coords.
    // Normalized device coords have a range of (-1, 1) on each axis.
    outW = 1.0f / outW;
    outX *= outW;
    outY *= -outW;
    outZ *= outW;
    // Perform the viewport transform for x and y (convert normalized device coords to viewport coords).
    // Viewport coords have a range of (0, Width) for x and (0, Height) for y.
    outX = (outX * camera->viewportW + camera->viewportW) * 0.5;
    outX += camera->viewportStartX;
    outY = (outY * camera->viewportH + camera->viewportH) * 0.5;
    outY += camera->viewportStartY;
    // Convert depth from (-1, 1) to (0, 1).
    outZ = (outZ + 1.0f) * 0.5;
    // Write out the calculated x and y values.
    *screenX = (s32)outX;
    *screenY = (s32)outY;
    // If a depth query wasn't requested, simply check if the point is within the view frustum.
    if (depthQueryID < 0) {
        if (outZ > 0.0f) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
    if (outX >= 0.0f && outY >= 0.0f && outX < 320.0f && outY < 240.0f) {
        gDPPipeSync(gMainGfxPos++);
        // Load a 4x1 pixel tile of the depth buffer
        gDPLoadTextureTile(gMainGfxPos++, osVirtualToPhysical(&nuGfxZBuffer[(s32) outY * SCREEN_WIDTH]), G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, 1,
            (s32) outX, 0, (s32) outX + 3, 0,
            0,
            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
            9, G_TX_NOMASK,
            G_TX_NOLOD, G_TX_NOLOD);
        gDPPipeSync(gMainGfxPos++);
        // Set the current color image to the buffer where copied depth values are stored.
        gDPSetColorImage(gMainGfxPos++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, DepthCopyBuffer);
        gDPPipeSync(gMainGfxPos++);
        // Set up 1 cycle mode and all other relevant othermode params.
        // One cycle mode must be used here because only one pixel is copied, and copy mode only supports multiples of 4 pixels.
        gDPSetCycleType(gMainGfxPos++, G_CYC_1CYCLE);
        gDPSetRenderMode(gMainGfxPos++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPSetCombineMode(gMainGfxPos++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gDPSetTextureFilter(gMainGfxPos++, G_TF_POINT);
        gDPSetTexturePersp(gMainGfxPos++, G_TP_NONE);
        gSPTexture(gMainGfxPos++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetTextureLUT(gMainGfxPos++, G_TT_NONE);
        gDPSetTextureDetail(gMainGfxPos++, G_TD_CLAMP);
        gDPSetTextureLOD(gMainGfxPos++, G_TL_TILE);
        // Adjust the scissor to only draw to the specified pixel.
        gDPSetScissor(gMainGfxPos++, G_SC_NON_INTERLACE, depthQueryID, 0, depthQueryID + 1, 1);
        // Draw a texrect to copy one pixel of the loaded depth tile to the output buffer.
        gSPTextureRectangle(gMainGfxPos++, depthQueryID << 2, 0 << 2, 4 << 2, 1 << 2, G_TX_RENDERTILE, (s32) outX << 5, 0, 1 << 10, 1 << 10);
        // Sync and swap the color image back to the current framebuffer.
        gDPPipeSync(gMainGfxPos++);
        gDPSetColorImage(gMainGfxPos++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, osVirtualToPhysical(nuGfxCfb_ptr));
        gDPPipeSync(gMainGfxPos++);
        // Reconfigure the frame's normal scissor.
        gDPSetScissor(gMainGfxPos++, G_SC_NON_INTERLACE, camera->viewportStartX, camera->viewportStartY, camera->viewportStartX + camera->viewportW, camera->viewportStartY + camera->viewportH);

        // The following code will use last frame's depth value, since the copy that was just written won't be executed until the current frame is drawn.

        // Extract the exponent and mantissa from the depth buffer value.
        depthExponent = DepthCopyBuffer[depthQueryID] >> DEPTH_EXPONENT_SHIFT;
        depthMantissa = (DepthCopyBuffer[depthQueryID] & (DEPTH_MANTISSA_MASK | DEPTH_DZ_MASK)) >> DEPTH_MANTISSA_SHIFT;
        // Convert the exponent and mantissa into a fixed-point value.
        shiftedMantissa = depthMantissa << DepthFloatLookupTable[depthExponent].shift;
        mantissaBias = DepthFloatLookupTable[depthExponent].bias;
        // Remove the 3 fractional bits of precision.
        decodedDepth = (shiftedMantissa + mantissaBias) >> 3;
        // Convert the calculated screen depth into viewport depth.
        scaledDepth = outZ * MAX_VIEWPORT_DEPTH;
        if (decodedDepth < scaledDepth) {
            return FALSE;
        }
    }
    return outZ > 0.0f;
}

// Checks if a point is visible on screen.
// If `depthQueryID` is nonnegative, the depth buffer is checked to see if the point is occluded by geometry.
//   Otherwise, the occlusion check is skipped.
// `depthQueryID` must be between 0 and the size of `DepthCopyBuffer` minus 1.
// Every nonnegative value of `depthQueryID` must be unique within a frame, otherwise the result will corrupt the data
//   of the previous query that shared the same ID.
// Occlusion visibility checks are always one frame out of date, as they reference the previous frame's depth buffer.
OPTIMIZE_OFAST b32 is_point_visible(f32 x, f32 y, f32 z, s32 depthQueryID, f32* screenX, f32* screenY) {
    Camera* camera = &gCameras[gCurrentCameraID];
    f32 outX;
    f32 outY;
    f32 outZ;
    f32 outW;

    s32 depthExponent;
    s32 depthMantissa;
    u32 shiftedMantissa, mantissaBias;
    u32 decodedDepth;
    s32 scaledDepth;

    // If an invalid depth query id was provided, return false.
    if (depthQueryID >= ARRAY_COUNT(DepthCopyBuffer)) {
        return FALSE;
    }
    // Transform the point into clip space.
    transform_point(camera->mtxPerspective, x, y, z, 1.0f, &outX, &outY, &outZ, &outW);
    if (outW == 0.0f) {
        *screenX = 0.0f;
        *screenY = 0.0f;
        return TRUE;
    }
    // Perform the perspective divide (divide xyz by w) to convert to normalized device coords.
    // Normalized device coords have a range of (-1, 1) on each axis.
    outW = 1.0f / outW;
    outX *= outW;
    outY *= -outW;
    outZ *= outW;
    // Perform the viewport transform for x and y (convert normalized device coords to viewport coords).
    // Viewport coords have a range of (0, Width) for x and (0, Height) for y.
    outX = (outX * camera->viewportW + camera->viewportW) * 0.5;
    outX += camera->viewportStartX;
    outY = (outY * camera->viewportH + camera->viewportH) * 0.5;
    outY += camera->viewportStartY;
    // Convert depth from (-1, 1) to (0, 1).
    outZ = (outZ + 1.0f) * 0.5;
    // Write out the calculated x and y values.
    *screenX = outX;
    *screenY = outY;
    // If a depth query wasn't requested, simply check if the point is within the view frustum.
    if (depthQueryID < 0) {
        return outZ > 0.0f;
    }
    if (outX >= 0.0f && outY >= 0.0f && outX < 320.0f && outY < 240.0f) {
        gDPPipeSync(gMainGfxPos++);
        // Load a 4x1 pixel tile of the depth buffer
        gDPLoadTextureTile(gMainGfxPos++, osVirtualToPhysical(&nuGfxZBuffer[(s32) outY * SCREEN_WIDTH]), G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, 1,
            (s32) outX, 0, (s32) outX + 3, 0,
            0,
            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
            9, G_TX_NOMASK,
            G_TX_NOLOD, G_TX_NOLOD);
        gDPPipeSync(gMainGfxPos++);
        // Set the current color image to the buffer where copied depth values are stored.
        gDPSetColorImage(gMainGfxPos++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, DepthCopyBuffer);
        gDPPipeSync(gMainGfxPos++);
        // Set up 1 cycle mode and all other relevant othermode params.
        // One cycle mode must be used here because only one pixel is copied, and copy mode only supports multiples of 4 pixels.
        gDPSetCycleType(gMainGfxPos++, G_CYC_1CYCLE);
        gDPSetRenderMode(gMainGfxPos++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPSetCombineMode(gMainGfxPos++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gDPSetTextureFilter(gMainGfxPos++, G_TF_POINT);
        gDPSetTexturePersp(gMainGfxPos++, G_TP_NONE);
        gSPTexture(gMainGfxPos++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPSetTextureLUT(gMainGfxPos++, G_TT_NONE);
        gDPSetTextureDetail(gMainGfxPos++, G_TD_CLAMP);
        gDPSetTextureLOD(gMainGfxPos++, G_TL_TILE);
        // Adjust the scissor to only draw to the specified pixel.
        gDPSetScissor(gMainGfxPos++, G_SC_NON_INTERLACE, depthQueryID, 0, depthQueryID + 1, 1);
        // Draw a texrect to copy one pixel of the loaded depth tile to the output buffer.
        gSPTextureRectangle(gMainGfxPos++, depthQueryID << 2, 0 << 2, (depthQueryID + 1) << 2, 1 << 2, G_TX_RENDERTILE, (s32) outX << 5, 0, 1 << 10, 1 << 10);
        // Sync and swap the color image back to the current framebuffer.
        gDPPipeSync(gMainGfxPos++);
        gDPSetColorImage(gMainGfxPos++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, osVirtualToPhysical(nuGfxCfb_ptr));
        gDPPipeSync(gMainGfxPos++);
        // Reconfigure the frame's normal scissor.
        gDPSetScissor(gMainGfxPos++, G_SC_NON_INTERLACE, camera->viewportStartX, camera->viewportStartY, camera->viewportStartX + camera->viewportW, camera->viewportStartY + camera->viewportH);

        // The following code will use last frame's depth value, since the copy that was just written won't be executed until the current frame is drawn.

        // Extract the exponent and mantissa from the depth buffer value.
        depthExponent = DepthCopyBuffer[depthQueryID] >> DEPTH_EXPONENT_SHIFT;
        depthMantissa = (DepthCopyBuffer[depthQueryID] & (DEPTH_MANTISSA_MASK | DEPTH_DZ_MASK)) >> DEPTH_MANTISSA_SHIFT;
        // Convert the exponent and mantissa into a fixed-point value.
        shiftedMantissa = depthMantissa << DepthFloatLookupTable[depthExponent].shift;
        mantissaBias = DepthFloatLookupTable[depthExponent].bias;
        // Remove the 3 fractional bits of precision.
        decodedDepth = (shiftedMantissa + mantissaBias) >> 3;
        // Convert the calculated screen depth into viewport depth.
        scaledDepth = outZ * MAX_VIEWPORT_DEPTH;
        if (decodedDepth < scaledDepth) {
            return FALSE;
        }
    }
    return outZ > 0.0f;
}

void mdl_draw_hidden_panel_surface(Gfx** arg0, u16 treeIndex) {
    Model* model = get_model_from_list_index(get_model_list_index_from_tree_index(treeIndex));
    Model copied = *model;
    Gfx* oldGfxPos;
    s32 flag;

    if (*arg0 == gMainGfxPos) {
        flag = 1;
    }

    oldGfxPos = gMainGfxPos;
    gMainGfxPos = *arg0;

    copied.flags = MODEL_FLAG_HAS_LOCAL_VERTEX_COPY | MODEL_FLAG_VALID;
    appendGfx_model(&copied);

    *arg0 = gMainGfxPos;

    if (flag == 0) {
        gMainGfxPos = oldGfxPos;
    }
}

void* mdl_get_next_texture_address(s32 size) {
    u32 offset = TextureHeapPos - TextureHeapBase + 0x3F;

    offset = (offset >> 6) << 6;

    if (size + offset > WORLD_TEXTURE_MEMORY_SIZE + BATTLE_TEXTURE_MEMORY_SIZE) {
        return NULL;
    } else {
        return TextureHeapBase + offset;
    }
}

void mdl_set_all_tint_type(s32 tintType) {
    ModelList* modelList = gCurrentModels;
    Model* model;
    s32 type = tintType; // weirdness here and the next line needed to match
    s32 i = tintType;

    for (i = 0; i < ARRAY_COUNT(*modelList); i++) {
        model = (*modelList)[i];

        if (model != NULL) {
            set_mdl_custom_gfx_set(model, CUSTOM_GFX_NONE, type);
        }
    }
}

void clear_render_tasks(void) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(ClearRenderTaskLists); i++) {
        RenderTaskLists[i] = ClearRenderTaskLists[i];
    }

    for (i = 0; i < ARRAY_COUNT(RenderTaskCount); i++) {
        RenderTaskCount[i] = 0;
    }
}

RenderTask* queue_render_task(RenderTask* task) {
    s32 dist = RenderTaskBasePriorities[task->renderMode] - task->dist;
    s32 listIdx = RENDER_TASK_LIST_MID;
    if (dist >= 3000000) listIdx = RENDER_TASK_LIST_FAR;
    else if (dist < 800000) listIdx = RENDER_TASK_LIST_NEAR;

    RenderTask* ret = RenderTaskLists[listIdx];

    ASSERT(RenderTaskCount[listIdx] < ARRAY_COUNT(*ClearRenderTaskLists));

    ret = &ret[RenderTaskCount[listIdx]++];

    ret->renderMode = RENDER_TASK_FLAG_ENABLED;
    if (task->renderMode == RENDER_MODE_CLOUD_NO_ZCMP) {
        ret->renderMode |= RENDER_TASK_FLAG_20;
    }

    ret->appendGfxArg = task->appendGfxArg;
    ret->appendGfx = task->appendGfx;
    ret->dist = dist;

    return ret;
}

OPTIMIZE_OFAST void execute_render_tasks(void) {
    s32 i, j;
    s32 sorteds[NUM_RENDER_TASK_LISTS][ARRAY_COUNT(*ClearRenderTaskLists)];
    s32* sorted;
    RenderTask* taskList;
    RenderTask* task;
    Matrix4f mtxFlipY;
    void (*appendGfx)(void*);
    s32 tmp;

    for (s32 j = 0; j < ARRAY_COUNT(RenderTaskCount); j++) {
        for (i = 0; i < RenderTaskCount[j]; i++) {
            sorteds[j][i] = i;
        }
    }

    // sort in ascending order of dist
    taskList = RenderTaskLists[RENDER_TASK_LIST_MID];
    sorted = sorteds[RENDER_TASK_LIST_MID];
#define LESS(i, j) taskList[sorted[i]].dist < taskList[sorted[j]].dist
#define SWAP(i, j) tmp = sorted[i], sorted[i] = sorted[j], sorted[j] = tmp
    QSORT(RenderTaskCount[RENDER_TASK_LIST_MID], LESS, SWAP);

    // tasks with dist >= 3M sort in descending order
    taskList = RenderTaskLists[RENDER_TASK_LIST_FAR];
    sorted = sorteds[RENDER_TASK_LIST_FAR];
#define LESS(i, j) taskList[sorted[i]].dist > taskList[sorted[j]].dist
    QSORT(RenderTaskCount[RENDER_TASK_LIST_FAR], LESS, SWAP);

    // tasks with dist <= 800k sort in descending order
    taskList = RenderTaskLists[RENDER_TASK_LIST_NEAR];
    sorted = sorteds[RENDER_TASK_LIST_NEAR];
    QSORT(RenderTaskCount[RENDER_TASK_LIST_NEAR], LESS, SWAP);

    gLastRenderTaskCount = RenderTaskCount[RENDER_TASK_LIST_MID] + RenderTaskCount[RENDER_TASK_LIST_FAR] + RenderTaskCount[RENDER_TASK_LIST_NEAR];
    if (gOverrideFlags & GLOBAL_OVERRIDES_ENABLE_FLOOR_REFLECTION) {
        Mtx* dispMtx;
        Gfx* savedGfxPos = NULL;

        guScaleF(mtxFlipY, 1.0f, -1.0f, 1.0f);
        guMtxF2L(mtxFlipY, &gDisplayContext->matrixStack[gMatrixListPos]);
        dispMtx = &gDisplayContext->matrixStack[gMatrixListPos++];
        for (j = 0; j < NUM_RENDER_TASK_LISTS; j++) {
            for (i = 0; i < RenderTaskCount[j]; i++) {
                task = &RenderTaskLists[j][sorteds[j][i]];
                appendGfx = task->appendGfx;

                if (task->renderMode & RENDER_TASK_FLAG_REFLECT_FLOOR) {
                    savedGfxPos = gMainGfxPos++;
                }

                appendGfx(task->appendGfxArg);

                if (task->renderMode & RENDER_TASK_FLAG_REFLECT_FLOOR) {
                    gSPEndDisplayList(gMainGfxPos++);
                    gSPBranchList(savedGfxPos, gMainGfxPos);
                    gSPDisplayList(gMainGfxPos++, savedGfxPos + 1);
                    gSPMatrix(gMainGfxPos++, dispMtx, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
                    gSPDisplayList(gMainGfxPos++, savedGfxPos + 1);
                    gSPMatrix(gMainGfxPos++, &gDisplayContext->camPerspMatrix[gCurrentCamID], G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
                }
            }
        }
    } else {
        for (j = 0; j < NUM_RENDER_TASK_LISTS; j++) {
            for (i = 0; i < RenderTaskCount[j]; i++) {
                task = &RenderTaskLists[j][sorteds[j][i]];
                appendGfx = task->appendGfx;
                appendGfx(task->appendGfxArg);
            }
        }
    }

    RenderTaskCount[RENDER_TASK_LIST_MID] = 0;
    RenderTaskCount[RENDER_TASK_LIST_FAR] = 0;
    RenderTaskCount[RENDER_TASK_LIST_NEAR] = 0;
}

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif
