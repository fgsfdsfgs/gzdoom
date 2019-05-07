#include "config.h"
#include "utils.h"
#include "input.h"
#include "screen.h"
#include "files.h"
#include "configs.h"
#include "ui.h"

#include <stdio.h>
#include <string.h>

#define SCROLL_SZ 16
#define OSK_INPUT_MAX 1024

extern struct Menu ui_menu_main;
extern struct Menu ui_menu_pwads;
extern struct Menu ui_menu_misc;
extern struct Menu ui_menu_net;

static struct Menu *ui_menus[MENU_COUNT] =
{
    &ui_menu_main,
    &ui_menu_pwads,
    &ui_menu_misc,
    &ui_menu_net,
};

static int ui_tab_x[MENU_COUNT];
static int ui_tab_w = 0;

int ui_current_menu = MENU_MAIN;
int ui_profile = -1;

// file dialog stuff

struct FileDialog
{
    const char *title;
    const char *dir;
    const char **exts;

    struct FileList flist;
    char *output;
    int sel;
    int scroll;
};

static int ui_file_select = 0;
static struct FileDialog ui_fd;

static char ui_input_text[OSK_INPUT_MAX + 1];
static int ui_keyboard = 0;

// OSK stuff

void UI_TouchKeyboardOpen(const char *hint, char *out, int outlen)
{
    if (!out || !outlen) return;

    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_FAILED(rc)) return;

    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetHeaderText(&kbd, hint);
    swkbdConfigSetGuideText(&kbd, hint);
    swkbdConfigSetInitialText(&kbd, out);

    ui_input_text[0] = 0;
    ui_keyboard = 1;

    rc = swkbdShow(&kbd, ui_input_text, outlen-1);
    swkbdClose(&kbd);

    if (R_SUCCEEDED(rc))
        strncpy(out, ui_input_text, outlen-1);

    ui_keyboard = 0;
}

// file dialog stuff

static int FileDialogStart(const char *title, const char *dir, const char **exts, char *output)
{
    if (ui_file_select) return 1;

    ui_fd.title = title;
    ui_fd.dir = dir;
    ui_fd.exts = exts;
    ui_fd.output = output;
    ui_fd.sel = ui_fd.scroll = 0;

    for (const char **ext = exts; ext && *ext; ++ext)
        FS_ListDir(&ui_fd.flist, ui_fd.dir, *ext);

    ui_file_select = 1;
    return 0;
}

static void FileDialogFinish(int success)
{
    if (!ui_file_select) return;

    if (!ui_fd.flist.numfiles) success = 0;

    if (ui_fd.output && success)
    {
        strncpy(
            ui_fd.output,
            ui_fd.flist.files[ui_fd.sel],
            MAX_STROPT
        );
    }

    FS_FreeFileList(&ui_fd.flist);

    ui_fd.output = NULL;
    ui_file_select = 0;
}

static void FileDialogUpdate(void)
{
    if (IN_ButtonPressed(B_DDOWN))
    {
        ui_fd.sel++;
        if (ui_fd.sel >= ui_fd.flist.numfiles) ui_fd.sel = 0;
    }
    else if (IN_ButtonPressed(B_DUP))
    {
        ui_fd.sel--;
        if (ui_fd.sel < 0) ui_fd.sel = ui_fd.flist.numfiles - 1;
    }

    if (ui_fd.sel < ui_fd.scroll)
        ui_fd.scroll = ui_fd.sel;
    else if (ui_fd.sel >= ui_fd.scroll + SCROLL_SZ)
        ui_fd.scroll = ui_fd.sel - SCROLL_SZ + 1;

    if (IN_ButtonPressed(B_A))
        FileDialogFinish(1);
    else if (IN_ButtonPressed(B_B))
        FileDialogFinish(0);
}

static void FileDialogDraw(void)
{
    R_Print(P_XCENTER, SCR_CX, 80, C_WHITE, ui_fd.title);
    R_Print(P_XCENTER, SCR_CX, 104, C_LTGREY, ui_fd.dir);

    if (!ui_fd.flist.numfiles)
    {
        R_Print(0, 160, 160, C_LTGREY, "No files.");
        return;
    }

    int y = 160;
    int scroll = ui_fd.scroll;
    int numfiles = ui_fd.flist.numfiles;
    int stop = scroll + SCROLL_SZ;

    if (scroll > 0)
        R_Print(0, 160, 160 - 24, C_LTGREY, "...");

    for (int i = scroll; i < numfiles && i < stop; ++i)
    {
        unsigned c = (ui_fd.sel == i) ? C_GREEN : C_WHITE;
        R_Print(0, 160, y, c, ui_fd.flist.files[i]);
        y += 24;
    }

    if (stop < numfiles)
        R_Print(0, 160, y, C_LTGREY, "...");
}

int UI_BlockingFileDialog(const char *title, const char *dir, const char **exts, char *output)
{
    if (FileDialogStart(title, dir, exts, output))
        return -1;

    while (ui_file_select)
    {
        IN_Update();
        FileDialogUpdate();
        R_BeginDrawing();
        R_Clear(C_BLACK);
        FileDialogDraw();
        R_EndDrawing();
    }

    return 0;
}

// options stuff

static void OptScroll(struct Option *opt, int dir)
{
    switch (opt->type)
    {
        case OPT_BOOLEAN:
            opt->boolean = !opt->boolean;
            break;

        case OPT_CHOICE:
            opt->choice.val += dir;
            if (opt->choice.val < 0) opt->choice.val = opt->choice.count - 1;
            else if (opt->choice.val >= opt->choice.count) opt->choice.val = 0;
            break;

        case OPT_INTEGER:
            opt->inum.val += opt->inum.step * dir;
            if (opt->inum.val < opt->inum.min) opt->inum.val = opt->inum.min;
            else if (opt->inum.val > opt->inum.max) opt->inum.val = opt->inum.max;
            break;

        case OPT_DOUBLE:
            opt->dnum.val += opt->dnum.step * dir;
            if (opt->dnum.val < opt->dnum.min) opt->dnum.val = opt->dnum.min;
            else if (opt->dnum.val > opt->dnum.max) opt->dnum.val = opt->dnum.max;
            break;

        default:
            break;
    }
}

static void OptActivate(struct Option *opt)
{
    if (opt->type == OPT_BUTTON)
    {
        opt->button = -2;
        UI_Redraw();
        opt->button = IN_GetFirstButton();
        if (opt->button == B_PLUS)
            opt->button = -1;
    }
    else if (opt->type == OPT_STRING)
    {
        UI_TouchKeyboardOpen(opt->name, opt->string, MAX_STROPT);
    }
    else if (opt->type == OPT_FILE)
    {
        FileDialogStart(
            "Select file",
            opt->file.dir,
            opt->file.ext,
            opt->file.val
        );
    }
    else if (opt->type == OPT_CALLBACK)
    {
        if (opt->cb) opt->cb(0);
    }
    else
    {
        OptScroll(opt, 1);
    }
}

static void OptReset(struct Option *opt)
{
    if (opt->type == OPT_STRING)
    {
        opt->string[0] = 0;
    }
    else if (opt->type == OPT_FILE)
    {
        opt->file.val[0] = 0;
    }
}

static void OptsUpdate(struct Menu *menu)
{
    struct Option *opts = menu->opts;
    int numopts = menu->numopts;

    if (IN_ButtonPressed(B_DDOWN))
    {
        menu->sel++;
        if (menu->sel >= numopts) menu->sel = 0;
    }
    else if (IN_ButtonPressed(B_DUP))
    {
        menu->sel--;
        if (menu->sel < 0) menu->sel = numopts - 1;
    }

    if (menu->sel < menu->scroll)
        menu->scroll = menu->sel;
    else if (menu->sel >= menu->scroll + SCROLL_SZ)
        menu->scroll = menu->sel - SCROLL_SZ + 1;

    if (IN_ButtonPressed(B_A))
        OptActivate(opts + menu->sel);
    else if (IN_ButtonPressed(B_X))
        OptReset(opts + menu->sel);
    else if (IN_ButtonPressed(B_DLEFT))
        OptScroll(opts + menu->sel, -1);
    else if (IN_ButtonPressed(B_DRIGHT))
        OptScroll(opts + menu->sel, 1);
}

static void OptDraw(struct Option *opt, int x, int y, int sel)
{
    unsigned c = sel ? C_GREEN : C_WHITE;
    const char *tmp;

    R_Print(0, x, y, c, opt->name);

    x += 960;
    switch (opt->type)
    {
        case OPT_BOOLEAN:
            tmp = opt->boolean ? "On" : "Off";
            R_Print(P_ARIGHT, x, y, c, tmp);
            break;

        case OPT_CHOICE:
            R_Print(P_ARIGHT, x, y, c, opt->choice.names[opt->choice.val]);
            break;

        case OPT_INTEGER:
            R_Print(P_ARIGHT, x, y, c, "%d", opt->inum.val);
            break;

        case OPT_DOUBLE:
            R_Print(P_ARIGHT, x, y, c, "%g", opt->dnum.val);
            break;

        case OPT_STRING:
            R_Print(P_ARIGHT, x, y, c, "%s", opt->string);
            break;

        case OPT_FILE:
            if (opt->file.val[0])
                R_Print(P_ARIGHT, x, y, c, "%s", opt->file.val);
            else
                R_Print(P_ARIGHT, x, y, c, "None");
            break;

        case OPT_BUTTON:
            if (opt->button == -1)
                R_Print(P_ARIGHT, x, y, c, "None");
            else if (opt->button == -2)
                R_Print(P_ARIGHT, x, y, c, "...");
            else
                R_DrawButton(x - 16, y - 16, c, opt->button);
            break;

        default:
            break;
    }
}

static void OptsDraw(struct Menu *menu)
{
    struct Option *opts = menu->opts;
    int numopts = menu->numopts;
    int sel = menu->sel;
    int scroll = menu->scroll;
    int stop = scroll + SCROLL_SZ;

    if (scroll > 0)
        R_Print(0, 160, 160 - 24, C_LTGREY, "...");

    int y = 160;
    for (int i = scroll; i < numopts && i < stop; ++i)
    {
        OptDraw(opts + i, 160, y, i == sel);
        y += 24;
    }

    if (stop < numopts)
        R_Print(0, 160, y, C_LTGREY, "...");
}

static void OptLoadVar(struct Option *opt)
{
    int itmp;
    double dtmp;
    char buf[100];

    int vtype = CFG_VarType(ui_profile, opt->cfgvar);

    switch (opt->type)
    {
        case OPT_BOOLEAN:
            if (opt->codevar)
                memcpy(&opt->boolean, opt->codevar, sizeof(int));
            else
                CFG_ReadVar(ui_profile, opt->cfgvar, &opt->boolean);
            break;

        case OPT_CHOICE:
            if (vtype == CVAR_INTEGER)
            {
                CFG_ReadVar(ui_profile, opt->cfgvar, &itmp);
                snprintf(buf, sizeof(buf), "%d", itmp);
            }
            else if (vtype == CVAR_DOUBLE)
            {
                CFG_ReadVar(ui_profile, opt->cfgvar, &dtmp);
                snprintf(buf, sizeof(buf), "%f", (float)dtmp);
            }
            else if (vtype == CVAR_STRING)
            {
                CFG_ReadVar(ui_profile, opt->cfgvar, buf);
            }
            else if (opt->codevar)
            {
                strncpy(buf, opt->codevar, sizeof(buf));
            }
            for (itmp = 0; itmp < opt->choice.count; ++itmp)
            {
                if (!strcmp(opt->choice.values[itmp], buf))
                {
                    opt->choice.val = itmp;
                    break;
                }
            }
            break;

        case OPT_INTEGER:
            if (opt->codevar)
                memcpy(&opt->inum.val, opt->codevar, sizeof(int));
            else
                CFG_ReadVar(ui_profile, opt->cfgvar, &opt->inum.val);
            if (opt->inum.val < opt->inum.min) opt->inum.val = opt->inum.min;
            else if (opt->inum.val > opt->inum.max) opt->inum.val = opt->inum.max;
            break;

        case OPT_DOUBLE:
            if (opt->codevar)
                memcpy(&opt->dnum.val, opt->codevar, sizeof(double));
            else
                CFG_ReadVar(ui_profile, opt->cfgvar, &opt->dnum.val);
            if (opt->dnum.val < opt->dnum.min) opt->dnum.val = opt->dnum.min;
            else if (opt->dnum.val > opt->dnum.max) opt->dnum.val = opt->dnum.max;
            break;

        case OPT_STRING:
            if (opt->codevar)
                strncpy(opt->string, opt->codevar, MAX_STROPT);
            else
                CFG_ReadVar(ui_profile, opt->cfgvar, opt->string);
            break;

        case OPT_FILE:
            if (opt->codevar)
                strncpy(opt->file.val, opt->codevar, MAX_STROPT);
            break;

        case OPT_BUTTON:
            if (opt->codevar)
                memcpy(&itmp, opt->codevar, sizeof(int));
            else
                CFG_ReadVar(ui_profile, opt->cfgvar, &itmp);
            opt->button = IN_KeyToButton(itmp);
            break;

        default:
            break;
    }
}

static void OptWriteVar(struct Option *opt)
{
    int vtype;
    int itmp;
    double dtmp;
    char *stmp;

    switch (opt->type)
    {
        case OPT_BOOLEAN:
            if (opt->codevar)
                memcpy(opt->codevar, &opt->boolean, sizeof(int));
            else
                CFG_WriteVar(ui_profile, opt->cfgvar, &opt->boolean);
            break;

        case OPT_CHOICE:
            vtype = CFG_VarType(ui_profile, opt->cfgvar);
            stmp = (char *)opt->choice.values[opt->choice.val];
            if (vtype == CVAR_INTEGER)
            {
                sscanf(stmp, "%d", &itmp);
                CFG_WriteVar(ui_profile, opt->cfgvar, &itmp);
            }
            else if (vtype == CVAR_DOUBLE)
            {
                sscanf(stmp, "%lf", &dtmp);
                CFG_WriteVar(ui_profile, opt->cfgvar, &dtmp);
            }
            else if (vtype == CVAR_STRING)
            {
                CFG_WriteVar(ui_profile, opt->cfgvar, stmp);
            }
            else if (opt->codevar)
            {
                strcpy(opt->codevar, stmp);
            }
            break;

        case OPT_INTEGER:
            if (opt->codevar)
                memcpy(opt->codevar, &opt->inum.val, sizeof(int));
            else
                CFG_WriteVar(ui_profile, opt->cfgvar, &opt->inum.val);
            break;

        case OPT_DOUBLE:
            if (opt->codevar)
                memcpy(opt->codevar, &opt->dnum.val, sizeof(double));
            else
                CFG_WriteVar(ui_profile, opt->cfgvar, &opt->dnum.val);
            break;

        case OPT_STRING:
            if (opt->codevar)
                strcpy(opt->codevar, opt->string);
            else
                CFG_WriteVar(ui_profile, opt->cfgvar, opt->string);
            break;

        case OPT_FILE:
            if (opt->codevar)
                strcpy(opt->codevar, opt->file.val);
            break;

        case OPT_BUTTON:
            itmp = IN_ButtonToKey(opt->button);
            if (opt->codevar)
                memcpy(opt->codevar, &itmp, sizeof(int));
            else
                CFG_WriteVar(ui_profile, opt->cfgvar, &itmp);
            break;

        default:
            break;
    }
}

static void OptsReload(struct Menu *menu)
{
    struct Option *opts = menu->opts;
    int numopts = menu->numopts;

    menu->sel = 0;
    menu->scroll = 0;

    for (int i = 0; i < numopts; ++i)
        OptLoadVar(opts + i);
}

static void OptsWrite(struct Menu *menu)
{
    struct Option *opts = menu->opts;
    int numopts = menu->numopts;

    for (int i = 0; i < numopts; ++i)
        OptWriteVar(opts + i);
}

// general ui stuff

int UI_Init(void)
{
    ui_tab_w = SCR_W / MENU_COUNT;

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        if (ui_menus[i]->opts)
            OptsReload(ui_menus[i]);
        ui_menus[i]->init();
        ui_tab_x[i] = (ui_tab_w / 2) + ui_tab_w * i;
    }

    return 0;
}

int UI_Update(void)
{
    if (ui_keyboard)
        return 0;

    if (ui_file_select)
    {
        FileDialogUpdate();
        return 0;
    }

    struct Menu *menu = ui_menus[ui_current_menu];

    if (IN_ButtonPressed(B_MINUS) || IN_ButtonPressed(B_R))
    {
        ui_current_menu += 1;
        if (ui_current_menu >= MENU_COUNT)
            ui_current_menu = 0;
        return 0;
    }

    if (IN_ButtonPressed(B_L))
    {
        ui_current_menu -= 1;
        if (ui_current_menu < 0)
            ui_current_menu = MENU_COUNT - 1;
        return 0;
    }

    if (IN_ButtonPressed(B_B))
    {
        UI_SaveOptions();
        return 1;
    }

    if (IN_ButtonPressed(B_PLUS))
    {
        UI_SaveOptions();
        CFG_SaveAll();
        FS_ExecGame(ui_profile);
    }

    if (menu->opts)
        OptsUpdate(menu);

    menu->update();

    return 0;
}

static void DrawMenuHint(void)
{

}

static void DrawFileDialogHint(void)
{

}

void UI_Draw(void)
{
    if (ui_file_select)
    {
        FileDialogDraw();
        DrawFileDialogHint();
    }
    else
    {
        struct Menu *menu = ui_menus[ui_current_menu];

        if (menu->opts)
            OptsDraw(menu);

        menu->draw();

        R_Print(P_XCENTER, SCR_CX, 80, C_WHITE, menu->title);

        DrawMenuHint();
    }

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        unsigned c = (ui_current_menu == i) ? C_GREEN : C_WHITE;
        R_Print(P_XCENTER, ui_tab_x[i], 16, c, ui_menus[i]->tabname);
    }

    R_DrawLine(0, 40, SCR_W, 40, C_LTGREY);
    R_DrawLine(0, SCR_H-40, SCR_W, SCR_H-40, C_LTGREY);

    R_Print(
        P_ABOTTOM | P_XCENTER,
        SCR_CX, SCR_H - 4, C_GREY,
        "v%s (%s), build date %s %s",
        VERSION, BUILD_COMMIT,
        __DATE__, __TIME__
    );
}

void UI_Redraw(void)
{
    R_BeginDrawing();
    R_Clear(C_BLACK);
    UI_Draw();
    R_EndDrawing();
}

void UI_ReloadOptions(void)
{
    for (int i = 0; i < MENU_COUNT; ++i)
    {
        if (ui_menus[i]->reload)
            ui_menus[i]->reload();
        if (ui_menus[i]->opts)
            OptsReload(ui_menus[i]);
    }
}

void UI_SaveOptions(void)
{
    for (int i = 0; i < MENU_COUNT; ++i)
    {
        if (ui_menus[i]->opts)
            OptsWrite(ui_menus[i]);
    }
}

void UI_Free(void)
{

}
