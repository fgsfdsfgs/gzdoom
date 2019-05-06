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

// static struct Menu *self = &ui_menu_main;

void UI_MenuMain_Init(void)
{
    for (ui_profile = 0; ui_profile < MAX_PROFILES; ++ui_profile)
        if (fs_profiles[ui_profile].present) break;
}

static inline void ScrollProfile(int down)
{
    int prev_selection = ui_profile;

    if (down)
    {
        for (ui_profile += 1; ui_profile < MAX_PROFILES; ++ui_profile)
            if (fs_profiles[ui_profile].present) break;
        if (ui_profile >= MAX_PROFILES) ui_profile = prev_selection;
    }
    else
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
}

void UI_MenuMain_Update(void)
{
    if (IN_ButtonPressed(B_DDOWN))
        ScrollProfile(1);
    else if (IN_ButtonPressed(B_DUP))
        ScrollProfile(0);

    if (IN_ButtonPressed(B_A))
    {
        UI_SaveOptions();
        CFG_SaveAll();
        FS_ExecGame(ui_profile);
    }

    if (IN_ButtonPressed(B_X))
    {
        if (ui_profile < BUILTIN_PROFILES)
        {
            I_Error("This is a built-in profile; it cannot be deleted.");
        }
        else if (I_Question("Are you sure you want to delete this profile?"))
        {
            FS_DeleteProfile(ui_profile);
            ScrollProfile(0);
        }
    }

    if (IN_ButtonPressed(B_Y))
    {
        if (fs_maxprofile >= MAX_PROFILES-1)
        {
            I_Error("You cannot add any more profiles.");
        }
        else
        {
            char name[MAX_FNAME] = { 0 };
            char iwad[MAX_FNAME] = { 0 };
            const char *exts[] = { "wad", "pk3", "iwad", "ipk3", "ipk7", NULL };
            UI_TouchKeyboardOpen("Profile name", name, MAX_FNAME-1);
            if (!name[0]) return;
            UI_BlockingFileDialog("Select IWAD", BASEDIR "/iwads", exts, iwad);
            if (iwad[0]) FS_AddProfile(name, iwad);
        }
    }
}

void UI_MenuMain_Draw(void)
{
    R_Print(P_XCENTER, SCR_CX, 104, C_LTGREY, "(A) select   (X) delete   (Y) add");

    int y = 176;
    for (int i = 0; i < MAX_PROFILES; ++i)
    {
        if (!fs_profiles[i].present) continue;

        unsigned c = (ui_profile == i) ? C_GREEN : C_WHITE;
        R_Print(P_XCENTER, SCR_CX, y, c, "%s (%s)", fs_profiles[i].name, fs_profiles[i].iwad);

        y += 24;
    }
}
