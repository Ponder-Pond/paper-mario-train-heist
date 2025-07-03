#include "common.h"
#include "ld_addrs.h"
#include "npc.h"
#include "hud_element.h"
#include "rumble.h"
#include "sprite.h"
#include "model.h"
#include "gcc/string.h"
#include "dx/debug_menu.h"
#include "world/surfaces.h"

#ifdef SHIFT
#define ASSET_TABLE_ROM_START (s32) mapfs_ROM_START
#elif VERSION_JP
#define ASSET_TABLE_ROM_START 0x1E00000
#else
#define ASSET_TABLE_ROM_START 0x1E40000
#endif

#define ASSET_TABLE_HEADER_SIZE 0x20
#define ASSET_TABLE_FIRST_ENTRY (ASSET_TABLE_ROM_START + ASSET_TABLE_HEADER_SIZE)

BSS MapConfig* gMapConfig;
BSS MapSettings gMapSettings;

char wMapHitName[0x18];
char wMapShapeName[0x18];
char wMapTexName[0x18];
char wMapBgName[0x14];

s32 WorldReverbModeMapping[] = { 0, 1, 2, 3 };

typedef struct {
    /* 0x00 */ char name[16];
    /* 0x10 */ u32 offset;
    /* 0x14 */ u32 compressedLength;
    /* 0x18 */ u32 decompressedLength;
} AssetHeader; // size = 0x1C

void fio_deserialize_state(void);
void load_map_hit_asset(void);

#if defined(SHIFT) || VERSION_IQUE
#define shim_general_heap_create_obfuscated general_heap_create
#endif

extern ShapeFile gMapShapeData;

void load_map_script_lib(void) {
    DMA_COPY_SEGMENT(world_script_api);
}

void load_map_by_IDs(s16 areaID, s16 mapID, s16 loadType) {
    s32 skipLoadingAssets = 0;
    MapConfig* mapConfig;
    MapSettings* mapSettings;
    char texStr[17];
    s32 decompressedSize;

    sfx_stop_env_sounds();
    gOverrideFlags &= ~GLOBAL_OVERRIDES_40;
    gOverrideFlags &= ~GLOBAL_OVERRIDES_ENABLE_FLOOR_REFLECTION;

    gGameStatusPtr->playerSpriteSet = PLAYER_SPRITES_MARIO_WORLD;
    surface_set_walk_effect(SURFACE_WALK_FX_STANDARD);
    phys_set_player_sliding_check(NULL);
    phys_set_landing_adjust_cam_check(NULL);

#if !VERSION_IQUE
    load_obfuscation_shims();
#endif
    shim_general_heap_create_obfuscated();

#if VERSION_JP
    reset_max_rumble_duration();
#endif
    clear_render_tasks();
    clear_worker_list();
    clear_script_list();

    switch (loadType) {
        case LOAD_FROM_MAP:
            clear_area_flags();
            gGameStatusPtr->loadType = LOAD_FROM_MAP;
            break;
        case LOAD_FROM_FILE_SELECT:
            fio_deserialize_state();
            areaID = gGameStatusPtr->areaID;
            mapID = gGameStatusPtr->mapID;
            gGameStatusPtr->prevArea = areaID;
            gGameStatusPtr->loadType = LOAD_FROM_FILE_SELECT;
            break;
    }

    gGameStatusPtr->mapShop = NULL;

    ASSERT_MSG(areaID < ARRAY_COUNT(gAreas) - 1, "Invalid area ID %d", areaID);
    ASSERT_MSG(mapID < gAreas[areaID].mapCount, "Invalid map ID %d in %s", mapID, gAreas[areaID].id);
    mapConfig = &gAreas[areaID].maps[mapID];

    #if DX_DEBUG_MENU
    dx_debug_set_map_info(mapConfig->id, gGameStatus.entryID);
    #endif

    sprintf(wMapShapeName, "%s_shape", mapConfig->id);
    sprintf(wMapHitName, "%s_hit", mapConfig->id);
    strcpy(texStr, mapConfig->id);
    texStr[3] = '\0';
    sprintf(wMapTexName, "%s_tex", texStr);

    gMapConfig = mapConfig;
    if (mapConfig->bgName != NULL) {
        strcpy(wMapBgName, mapConfig->bgName);
    }
    load_map_script_lib();

    if (mapConfig->dmaStart != NULL) {
        dma_copy(mapConfig->dmaStart, mapConfig->dmaEnd, mapConfig->dmaDest);
    }

    gMapSettings = *mapConfig->settings;

    mapSettings = &gMapSettings;
    if (mapConfig->init != NULL) {
        skipLoadingAssets = mapConfig->init();
    }

    if (!skipLoadingAssets) {
        ShapeFile* shapeFile = &gMapShapeData;
        void* yay0Asset = load_asset_by_name(wMapShapeName, &decompressedSize);

        decode_yay0(yay0Asset, shapeFile);
        general_heap_free(yay0Asset);

        mapSettings->modelTreeRoot = shapeFile->header.root;
        mapSettings->modelNameList = shapeFile->header.modelNames;
        mapSettings->colliderNameList = shapeFile->header.colliderNames;
        mapSettings->zoneNameList = shapeFile->header.zoneNames;
    }

    if (mapConfig->bgName != NULL) {
        load_map_bg(wMapBgName);
    }

#if !VERSION_IQUE
    load_obfuscation_shims();
#endif
    shim_general_heap_create_obfuscated();

    sfx_clear_env_sounds(0);
    clear_worker_list();
    clear_script_list();
    create_cameras();
    spr_init_sprites(gGameStatusPtr->playerSpriteSet);
    clear_animator_list();
    clear_entity_models();
    clear_npcs();
    hud_element_clear_cache();
    clear_trigger_data();
    clear_model_data();
    clear_sprite_shading_data();
    reset_background_settings();

    if (gGameStatusPtr->introPart == INTRO_PART_NONE) {
        func_80138188();
    }

    if (!skipLoadingAssets) {
        initialize_collision();
        load_map_hit_asset();
    }

    reset_battle_status();
    clear_encounter_status();
    clear_entity_data(TRUE);
    clear_effect_data();
    clear_player_status();
    player_reset_data();
    partner_reset_data();
    clear_printers();
    clear_item_entity_data();

    gPlayerStatus.targetYaw = gPlayerStatus.curYaw;

    sfx_set_reverb_mode(WorldReverbModeMapping[*(s32*)mapConfig->unk_1C & 0x3]);
    sfx_reset_door_sounds();

    if (!skipLoadingAssets) {
        s32 texturesOffset = get_asset_offset(wMapTexName, &decompressedSize);

        if (mapSettings->modelTreeRoot != NULL) {
            load_data_for_models(mapSettings->modelTreeRoot, texturesOffset, decompressedSize);
        }
    }

    if (mapSettings->background != NULL) {
        set_background(mapSettings->background);
    } else {
        set_background_size(296, 200, 12, 20);
    }

    gCurrentCameraID = CAM_DEFAULT;
    gCameras[CAM_DEFAULT].flags |= CAMERA_FLAG_DISABLED;
    gCameras[CAM_BATTLE].flags |= CAMERA_FLAG_DISABLED;
    gCameras[CAM_TATTLE].flags |= CAMERA_FLAG_DISABLED;
    gCameras[CAM_HUD].flags |= CAMERA_FLAG_DISABLED;

    if (gGameStatusPtr->introPart == INTRO_PART_NONE) {
        set_cam_viewport(CAM_DEFAULT, 12, 20, 296, 200);
    } else {
        set_cam_viewport(CAM_DEFAULT, 29, 28, 262, 162);
    }

    initialize_status_bar();
    gGameStatusPtr->unk_90 = 1000;
    gGameStatusPtr->unk_92 = 1000;
    gGameStatusPtr->mainScriptID = start_script_in_group(mapSettings->main, EVT_PRIORITY_0, 0, EVT_GROUP_NEVER_PAUSE)->id;
}

MapConfig* get_current_map_config(void) {
    return gMapConfig;
}

MapSettings* get_current_map_settings(void) {
    return &gMapSettings;
}

NODISCARD s32 get_map_IDs_by_name(const char* mapName, s16* areaID, s16* mapID) {
    s32 i;
    s32 j;
    MapConfig* maps;

    // TODO: Potentially a fake match? Difficult to not set the temp in the for conditional.
    for (i = 0; (maps = gAreas[i].maps) != NULL; i++) {
        for (j = 0; j < gAreas[i].mapCount; j++) {
            if (strcmp(maps[j].id, mapName) == 0) {
                *areaID = i;
                *mapID = j;
                return TRUE;
            }
        }
    }

    return FALSE;
}

void get_map_IDs_by_name_checked(const char* mapName, s16* areaID, s16* mapID) {
    ASSERT_MSG(get_map_IDs_by_name(mapName, areaID, mapID), "Map not found: %s", mapName);
}

void* load_asset_by_name(const char* assetName, u32* decompressedSize) {
    AssetHeader firstHeader;
    AssetHeader* assetTableBuffer;
    AssetHeader* curAsset;
    void* ret;

    dma_copy((u8*) ASSET_TABLE_FIRST_ENTRY, (u8*) ASSET_TABLE_FIRST_ENTRY + sizeof(AssetHeader), &firstHeader);
    assetTableBuffer = heap_malloc(firstHeader.offset);
    curAsset = &assetTableBuffer[0];
    dma_copy((u8*) ASSET_TABLE_FIRST_ENTRY, (u8*) ASSET_TABLE_FIRST_ENTRY + firstHeader.offset, assetTableBuffer);
    while (strcmp(curAsset->name, assetName) != 0) {
        ASSERT_MSG(strcmp(curAsset->name, "end_data") != 0, "Asset not found: %s", assetName);
        curAsset++;
    }
    *decompressedSize = curAsset->decompressedLength;
    ret = general_heap_malloc(curAsset->compressedLength);
    dma_copy((u8*) ASSET_TABLE_FIRST_ENTRY + curAsset->offset,
             (u8*) ASSET_TABLE_FIRST_ENTRY + curAsset->offset + curAsset->compressedLength, ret);
    heap_free(assetTableBuffer);
    return ret;
}

s32 get_asset_offset(char* assetName, s32* compressedSize) {
    AssetHeader firstHeader;
    AssetHeader* assetTableBuffer;
    AssetHeader* curAsset;
    s32 ret;

    dma_copy((u8*) ASSET_TABLE_FIRST_ENTRY, (u8*) ASSET_TABLE_FIRST_ENTRY + sizeof(AssetHeader), &firstHeader);
    assetTableBuffer = heap_malloc(firstHeader.offset);
    curAsset = &assetTableBuffer[0];
    dma_copy((u8*) ASSET_TABLE_FIRST_ENTRY, (u8*) ASSET_TABLE_FIRST_ENTRY + firstHeader.offset, assetTableBuffer);
    while (strcmp(curAsset->name, assetName) != 0) {
        ASSERT_MSG(strcmp(curAsset->name, "end_data") != 0, "Asset not found: %s", assetName);
        curAsset++;
    }
    *compressedSize = curAsset->compressedLength;
    ret = ASSET_TABLE_FIRST_ENTRY + curAsset->offset;
    heap_free(assetTableBuffer);
    return ret;
}

#define AREA(area, jp_name) { ARRAY_COUNT(area##_maps), area##_maps, "area_" #area, jp_name }

#define MAP(map) \
    .id = #map, \
    .settings = &map##_settings, \
    .dmaStart = map##_ROM_START, \
    .dmaEnd = map##_ROM_END, \
    .dmaDest = map##_VRAM \

#define MAP_WITH_INIT(map) \
    MAP(map), \
    .init = &map##_map_init \

s32 map_init_mac_tex(void) {
    sprintf(wMapTexName, "mac_tex");
    return FALSE;
}

/// Credits
#include "area_end/end.h"
MapConfig end_maps[] = {
    { MAP_WITH_INIT(end_00) },
    { MAP_WITH_INIT(end_01) },
};

/// Game Over
#include "area_gv/gv.h"
MapConfig gv_maps[] = {
    { MAP(gv_01) },
};

/// Train
#include "area_trn/trn.h"
MapConfig trn_maps[] = {
    { MAP(trn_01) },
};


AreaConfig gAreas[] = {
    AREA(end, "エンディング"),  // endingu [Ending]
    AREA(gv, "ゲームオーバー"),  // ge-mu o-ba- [Game Over]
    AREA(trn, "Train"),
    {},
};
