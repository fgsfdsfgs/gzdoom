#include "config.h"
#include "utils.h"
#include "files.h"
#include "input.h"
#include "screen.h"
#include "ui.h"

void UI_MenuMisc_Init(void);
void UI_MenuMisc_Update(void);
void UI_MenuMisc_Draw(void);
void UI_MenuMisc_Reload(void);

static const char *mon_labels[] = 
{
    "Default",
    "Disabled",
    "Fast",
    "Respawning",
    "Fast and Respawning",
};

static const char *mon_values[] = 
{
    "0", "1", "2", "4", "6",
};

static struct Option misc_opts[] =
{
    { OPT_CHOICE, "Monsters", .choice = { mon_labels, mon_values, 5, 0 } },
    { OPT_BOOLEAN, "Record demo" },
    { OPT_INTEGER, "Skill", .inum = { 0, 5, 1, 0 } },
    { OPT_STRING, "Starting map" },
    { OPT_INTEGER, "Time limit (min)", .inum = { 0, 120, 1, 0 } },
    { OPT_STRING, "Player class" },
    { OPT_BOOLEAN, "Write log file" },
};

struct Menu ui_menu_misc =
{
    MENU_PWADS,
    "Misc",
    "Other game settings",
    NULL, 0, 0, 0,
    UI_MenuMisc_Init,
    UI_MenuMisc_Update,
    UI_MenuMisc_Draw,
    UI_MenuMisc_Reload,
};

static struct Menu *self = &ui_menu_misc;

void UI_MenuMisc_Init(void)
{
    UI_MenuMisc_Reload();
}

void UI_MenuMisc_Update(void)
{

}

void UI_MenuMisc_Draw(void)
{

}

void UI_MenuMisc_Reload(void)
{
    misc_opts[0].codevar = fs_profiles[ui_profile].monsters;
    misc_opts[1].codevar = &fs_profiles[ui_profile].record;
    misc_opts[2].codevar = &fs_profiles[ui_profile].skill;
    misc_opts[3].codevar = fs_profiles[ui_profile].warp;
    misc_opts[4].codevar = &fs_profiles[ui_profile].timer;
    misc_opts[5].codevar = fs_profiles[ui_profile].charclass;
    misc_opts[6].codevar = &fs_profiles[ui_profile].log;

    self->opts = misc_opts;
    self->numopts = 7;
}
