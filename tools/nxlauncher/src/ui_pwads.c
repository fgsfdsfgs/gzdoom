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
    { OPT_FILE, "DEH file 1" },
    { OPT_FILE, "DEH file 2" },
    { OPT_FILE, "DEH file 3" },
    { OPT_FILE, "DEH file 4" },
    { OPT_FILE, "Demo" },
    { OPT_FILE, "Override response file" },
    { OPT_FILE, "Override INI file" },
    { OPT_STRING, "Log file name" },
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
    for (int i = 0; i < 4; ++i)
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

    for (int i = 4; i < 8; ++i)
    {
        pwads_opts[i].codevar = fs_profiles[ui_profile].pwads[i];
        pwads_opts[i].file.dir = BASEDIR "/pwads";
        pwads_opts[i].file.ext[0] = "deh";
        pwads_opts[i].file.ext[1] = "bex";
        pwads_opts[i].file.ext[2] = NULL;
    }

    pwads_opts[8].codevar = fs_profiles[ui_profile].demo;
    pwads_opts[8].file.dir = BASEDIR "/pwads";
    pwads_opts[8].file.ext[0] = "lmp";
    pwads_opts[8].file.ext[1] = NULL;

    pwads_opts[9].codevar = fs_profiles[ui_profile].rsp;
    pwads_opts[9].file.dir = BASEDIR "/pwads";
    pwads_opts[9].file.ext[0] = "rsp";
    pwads_opts[9].file.ext[1] = NULL;

    pwads_opts[10].codevar = fs_profiles[ui_profile].ini;
    pwads_opts[10].file.dir = BASEDIR;
    pwads_opts[10].file.ext[0] = "ini";
    pwads_opts[10].file.ext[1] = NULL;

    pwads_opts[11].codevar = fs_profiles[ui_profile].log;

    self->opts = pwads_opts;
    self->numopts = 12;
}
