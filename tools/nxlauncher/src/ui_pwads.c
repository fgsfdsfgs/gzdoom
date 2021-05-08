#include "config.h"
#include "utils.h"
#include "files.h"
#include "input.h"
#include "screen.h"
#include "ui.h"

void UI_MenuFiles_Init(void);
void UI_MenuFiles_Update(void);
void UI_MenuFiles_Draw(void);
void UI_MenuFiles_Reload(void);

static struct Option pwads_opts[] =
{
    { OPT_FILE, "Custom file 1" },
    { OPT_FILE, "Custom file 2" },
    { OPT_FILE, "Custom file 3" },
    { OPT_FILE, "Custom file 4" },
    { OPT_FILE, "Custom file 5" },
    { OPT_FILE, "Custom file 6" },
    { OPT_FILE, "Custom file 7" },
    { OPT_FILE, "Custom file 8" },
    { OPT_FILE, "Custom file 9" },
    { OPT_FILE, "Custom file 10" },
    { OPT_FILE, "DEH file 1" },
    { OPT_FILE, "DEH file 2" },
    { OPT_FILE, "Demo" },
    { OPT_FILE, "Load saved game" },
    { OPT_FILE, "Override response file" },
    { OPT_FILE, "Override INI file" },
};

struct Menu ui_menu_pwads =
{
    MENU_PWADS,
    "Files",
    "Custom content settings",
    NULL, 0, 0, 0,
    UI_MenuFiles_Init,
    UI_MenuFiles_Update,
    UI_MenuFiles_Draw,
    UI_MenuFiles_Reload,
};

static struct Menu *self = &ui_menu_pwads;

void UI_MenuFiles_Init(void)
{
    UI_MenuFiles_Reload();
}

void UI_MenuFiles_Update(void)
{

}

void UI_MenuFiles_Draw(void)
{

}

void UI_MenuFiles_Reload(void)
{
    for (int i = 0; i < 10; ++i)
    {
        pwads_opts[i].codevar = fs_profiles[ui_profile].pwads[i];
        pwads_opts[i].file.dir = BASEDIR "/pwads";
        pwads_opts[i].file.ext[0] = "wad";
        pwads_opts[i].file.ext[1] = "pk3";
        pwads_opts[i].file.ext[2] = "pk7";
        pwads_opts[i].file.ext[3] = "lmp";
        pwads_opts[i].file.ext[4] = "zip";
        pwads_opts[i].file.ext[5] = NULL;
    }

    for (int i = 10; i < 12; ++i)
    {
        pwads_opts[i].codevar = fs_profiles[ui_profile].dehs[i-10];
        pwads_opts[i].file.dir = BASEDIR "/pwads";
        pwads_opts[i].file.ext[0] = "deh";
        pwads_opts[i].file.ext[1] = "bex";
        pwads_opts[i].file.ext[2] = NULL;
    }

    pwads_opts[12].codevar = fs_profiles[ui_profile].demo;
    pwads_opts[12].file.dir = BASEDIR "/pwads";
    pwads_opts[12].file.ext[0] = "lmp";
    pwads_opts[12].file.ext[1] = NULL;

    pwads_opts[13].codevar = fs_profiles[ui_profile].load;
    pwads_opts[13].file.dir = BASEDIR "/save";
    pwads_opts[13].file.ext[0] = "zds";
    pwads_opts[13].file.ext[1] = NULL;

    pwads_opts[14].codevar = fs_profiles[ui_profile].rsp;
    pwads_opts[14].file.dir = BASEDIR "/pwads";
    pwads_opts[14].file.ext[0] = "rsp";
    pwads_opts[14].file.ext[1] = NULL;

    pwads_opts[15].codevar = fs_profiles[ui_profile].ini;
    pwads_opts[15].file.dir = BASEDIR;
    pwads_opts[15].file.ext[0] = "ini";
    pwads_opts[15].file.ext[1] = NULL;

    self->opts = pwads_opts;
    self->numopts = 16;
}
