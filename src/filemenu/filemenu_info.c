#include "common.h"
#include "filemenu.h"
#include "hud_element.h"

#if VERSION_PAL
extern u8 D_filemenu_802508FC[];
extern u8 D_filemenu_80250934[];
extern u8 D_filemenu_80250948[];
extern u8 D_filemenu_8025094C[];
extern s8 D_filemenu_80250950[];
extern u8 D_filemenu_80250968[];
#endif

#if VERSION_IQUE
#define CREATE_SUCCESS_NUMBER_X 49
#define NUMBER_OFFSET_Y 1
#else
#define NUMBER_OFFSET_Y 0
#define CREATE_SUCCESS_NUMBER_X 48
#endif

u8 filemenu_info_gridData[] = {
    0, 0, 0, 0
};

MenuWindowBP filemenu_info_windowBPs[] = {
    {
        .windowID = WIN_FILES_MESSAGE,
        .unk_01 = 0,
        .pos = { .x = 0, .y = 0 },
        .width = 0,
        .height = 0,
        .priority = WINDOW_PRIORITY_64,
        .fpDrawContents = &filemenu_info_draw_message_contents,
        .tab = NULL,
        .parentID = -1,
        .fpUpdate = { WINDOW_UPDATE_HIDE },
        .extraFlags = 0,
        .style = { .customStyle = &filemenu_windowStyles[20] }
    },
};

MenuPanel filemenu_info_menuBP = {
    .initialized = FALSE,
    .col = 0,
    .row = 0,
    .selected = 0,
    .state = 0,
    .numCols = 1,
    .numRows = 1,
    .numPages = 0,
    .gridData = filemenu_info_gridData,
    .fpInit = &filemenu_info_init,
    .fpHandleInput = &filemenu_info_handle_input,
    .fpUpdate = &filemenu_info_update,
    .fpCleanup = &filemenu_info_cleanup
};

void filemenu_info_draw_message_contents(
    MenuPanel* menu,
    s32 baseX, s32 baseY,
    s32 width, s32 height,
    s32 opacity, s32 darkening
) {
#if VERSION_PAL
    s32 xOffset;

    switch (menu->page) {
        case FM_MESSAGE_DELETED:
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_28), baseX + 10, baseY + 4, 255, 0, 0);
            xOffset = D_filemenu_80250934[gCurrentLanguage] + 10;
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + xOffset, baseY + 4, 255, 0, 0);
            xOffset += D_filemenu_802508FC[gCurrentLanguage];
            draw_number(filemenu_menus[FILE_MENU_MAIN]->selected + 1, baseX + xOffset, baseY + 6, 0, 0, 255, 3);
            xOffset++;
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_HAS_BEEN_DELETED), baseX + xOffset, baseY + 4, 255, 0, 0);
            break;
        case FM_MESSAGE_COPIED:
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_COPY_FROM), baseX + 10, baseY + 4, 255, 0, 0);
            xOffset = D_filemenu_80250948[gCurrentLanguage] + 10;
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + xOffset, baseY + 4, 255, 0, 0);
            xOffset += D_filemenu_802508FC[gCurrentLanguage];
            draw_number(filemenu_CopyFromFileIdx + 1, baseX + xOffset, baseY + 6, 0, 0, 255, 3);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_TO), baseX + 10, baseY + 18, 255, 0, 0);
            xOffset = D_filemenu_8025094C[gCurrentLanguage] + 10;
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + xOffset, baseY + 18, 255, 0, 0);
            xOffset += D_filemenu_802508FC[gCurrentLanguage];
            draw_number(filemenu_CopyToFileIdx + 1, baseX + xOffset, baseY + 0x14, 0, 0, 255, 3);
            xOffset += D_filemenu_80250950[gCurrentLanguage];
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_HAS_BEEN_CREATED), baseX + xOffset, baseY + 18, 255, 0, 0);
            break;
        case FM_MESSAGE_CREATED:
            filemenu_draw_message(filemenu_get_menu_message(0x20), baseX + 10, baseY + 4, 255, 0, 0);
            xOffset = D_filemenu_80250968[gCurrentLanguage] + 10;
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + xOffset, baseY + 4, 255, 0, 0);
            xOffset += D_filemenu_802508FC[gCurrentLanguage];
            draw_number(filemenu_menus[FILE_MENU_MAIN]->selected + 1, baseX + xOffset, baseY + 6, 0, 0, 255, 3);
            xOffset++;
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_PAL_UNK1), baseX + xOffset, baseY + 4, 255, 0, 0);
            break;
    }
#else
    switch (menu->state) {
        case FM_MESSAGE_DELETED:
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + 10, baseY + 4, 255, 0, 0);
            draw_number(filemenu_menus[FILE_MENU_MAIN]->selected + 1, baseX + 48, baseY + 6 + NUMBER_OFFSET_Y, DRAW_NUMBER_CHARSET_NORMAL, MSG_PAL_WHITE, 255, DRAW_NUMBER_STYLE_MONOSPACE | DRAW_NUMBER_STYLE_ALIGN_RIGHT);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_HAS_BEEN_DELETED), baseX + 49, baseY + 4, 255, 0, 0);
            break;
        case FM_MESSAGE_COPIED:
#if VERSION_IQUE
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_COPY_FROM), baseX + 10, baseY + 7, 255, 0, 0);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + 42, baseY + 7, 255, 0, 0);
            draw_number(filemenu_CopyFromFileIdx + 1, baseX + 84, baseY + 10, DRAW_NUMBER_CHARSET_NORMAL, MSG_PAL_WHITE, 255, DRAW_NUMBER_STYLE_MONOSPACE | DRAW_NUMBER_STYLE_ALIGN_RIGHT);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_TO), baseX + 84, baseY + 7, 255, 0, 0);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + 100, baseY + 7, 255, 0, 0);
            draw_number(filemenu_CopyToFileIdx + 1, baseX + 140, baseY + 10, DRAW_NUMBER_CHARSET_NORMAL, MSG_PAL_WHITE, 255, DRAW_NUMBER_STYLE_MONOSPACE | DRAW_NUMBER_STYLE_ALIGN_RIGHT);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_PERIOD_34), baseX + 140, baseY + 7, 255, 0, 0);
#else
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_COPY_FROM), baseX + 10, baseY + 4, 255, 0, 0);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + 84, baseY + 4, 255, 0, 0);
            draw_number(filemenu_CopyFromFileIdx + 1, baseX + 122, baseY + 6, DRAW_NUMBER_CHARSET_NORMAL, MSG_PAL_WHITE, 255, DRAW_NUMBER_STYLE_MONOSPACE | DRAW_NUMBER_STYLE_ALIGN_RIGHT);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_TO), baseX + 10, baseY + 18, 255, 0, 0);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + 30, baseY + 18, 255, 0, 0);
            draw_number(filemenu_CopyToFileIdx + 1, baseX + 68, baseY + 20, DRAW_NUMBER_CHARSET_NORMAL, MSG_PAL_WHITE, 255, DRAW_NUMBER_STYLE_MONOSPACE | DRAW_NUMBER_STYLE_ALIGN_RIGHT);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_PERIOD_34), baseX + 65, baseY + 18, 255, 0, 0);
#endif
            break;
        case FM_MESSAGE_CREATED:
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_FILE_26), baseX + 10, baseY + 4, 255, 0, 0);
            draw_number(filemenu_menus[FILE_MENU_MAIN]->selected + 1, baseX + CREATE_SUCCESS_NUMBER_X, baseY + 6 + NUMBER_OFFSET_Y, DRAW_NUMBER_CHARSET_NORMAL, MSG_PAL_WHITE, 255, DRAW_NUMBER_STYLE_MONOSPACE | DRAW_NUMBER_STYLE_ALIGN_RIGHT);
            filemenu_draw_message(filemenu_get_menu_message(FILE_MESSAGE_HAS_BEEN_CREATED), baseX + 49, baseY + 4, 255, 0, 0);
            break;
    }
#endif
    filemenu_set_cursor_alpha(0);
}

void filemenu_info_init(MenuPanel* tab) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(filemenu_info_windowBPs); i++) {
        filemenu_info_windowBPs[i].tab = tab;
    }

    setup_pause_menu_tab(filemenu_info_windowBPs, ARRAY_COUNT(filemenu_info_windowBPs));
    tab->initialized = TRUE;
}

void filemenu_info_handle_input(MenuPanel* menu) {
    if (filemenu_pressedButtons & (BUTTON_A | BUTTON_B)) {
        MenuPanel* menu = filemenu_menus[FILE_MENU_MAIN];

        filemenu_currentMenu = FILE_MENU_MAIN;

        switch (menu->state) {
            case FILE_MENU_CONFIRM:
                menu->state = FILE_MENU_MAIN;
                set_window_update(WIN_FILES_STEREO, (s32)filemenu_update_show_options_left);
                set_window_update(WIN_FILES_MONO, (s32)filemenu_update_show_options_right);
                set_window_update(WIN_FILES_OPTION_LEFT, (s32)filemenu_update_show_options_bottom);
                set_window_update(WIN_FILES_OPTION_RIGHT, (s32)filemenu_update_show_options_bottom);
                filemenu_set_selected(menu, 0, 2);
                break;
            case FILE_MENU_MESSAGE:
                menu->state = FILE_MENU_MESSAGE;
                filemenu_set_selected(menu, 1, 2);
                break;
        }
        set_window_update(WIN_FILES_MESSAGE, WINDOW_UPDATE_HIDE);
    }
}

void filemenu_info_update(MenuPanel* menu) {
}

void filemenu_info_cleanup(MenuPanel* menu) {
}
