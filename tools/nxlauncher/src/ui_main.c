#include "config.h"
#include "utils.h"
#include "files.h"
#include "input.h"
#include "screen.h"
#include "configs.h"
#include "ui.h"

void UI_MenuMain_Init(void);
void UI_MenuMain_Update(void);
void UI_MenuMain_Draw(void);

struct Menu ui_menu_main =
{
    MENU_MAIN,
    "Profile",
    "Select profile",
    NULL, 0, 0, 0,
    UI_MenuMain_Init,
    UI_MenuMain_Update,
    UI_MenuMain_Draw,
};

static struct Menu *self = &ui_menu_main;

void UI_MenuMain_Init(void)
{
    for (ui_profile = 0; ui_profile < MAX_PROFILES; ++ui_profile)
        if (fs_profiles[ui_profile].present) break;
}

void UI_MenuMain_Update(void)
{
    int prev_selection = ui_profile;

    if (IN_ButtonPressed(B_DDOWN))
    {
        for (ui_profile += 1; ui_profile < MAX_PROFILES; ++ui_profile)
            if (fs_profiles[ui_profile].present) break;
        if (ui_profile >= MAX_PROFILES) ui_profile = prev_selection;
    }
    else if (IN_ButtonPressed(B_DUP))
    {
        for (ui_profile -= 1; ui_profile >= 0; --ui_profile)
            if (fs_profiles[ui_profile].present) break;
        if (ui_profile < 0) ui_profile = prev_selection;
    }

    if (ui_profile != prev_selection)
    {
        // HACK
        int tmp = ui_profile;
        ui_profile = prev_selection;
        UI_SaveOptions();
        ui_profile = tmp;
        UI_ReloadOptions();
    }

    if (IN_ButtonPressed(B_A))
    {
        UI_SaveOptions();
        CFG_SaveAll();
        FS_ExecGame(ui_profile);
    }
}

void UI_MenuMain_Draw(void)
{
    int y = 160;
    for (int i = 0; i < MAX_PROFILES; ++i)
    {
        if (!fs_profiles[i].present) continue;

        unsigned c = (ui_profile == i) ? C_GREEN : C_WHITE;
        R_Print(P_XCENTER, SCR_CX, y, c, "%s (%s)", fs_profiles[i].name, fs_profiles[i].iwad);

        y += 24;
    }
}
