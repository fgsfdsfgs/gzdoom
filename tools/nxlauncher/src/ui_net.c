#include "config.h"
#include "utils.h"
#include "files.h"
#include "input.h"
#include "screen.h"
#include "configs.h"
#include "ui.h"

void UI_MenuNet_Init(void);
void UI_MenuNet_Update(void);
void UI_MenuNet_Draw(void);
void UI_MenuNet_Reload(void);

static void DoJoinNet(int);
static void DoHost(int);

static const char *mode_labels[] = 
{
    "Cooperative",
    "Deathmatch",
    "Deathmatch 2.0",
};

static const char *mode_values[] = 
{
    "", "deathmatch", "altdeath",
};

static struct Option net_opts[] =
{
    { OPT_BOOLEAN, "Client/server mode" },
    { OPT_STRING, "Game address" },
    { OPT_CALLBACK, "Connect to address", .cb = DoJoinNet },
    {
        OPT_CHOICE,
        "Server game mode",
        NULL, NULL,
        .choice =
        {
            mode_labels, mode_values,
            3, 0,
        },
    },
    { OPT_INTEGER, "Number of players", .inum = { 1, 8, 1, 1 } },
    { OPT_CALLBACK, "Host game", .cb = DoHost },
};

struct Menu ui_menu_net =
{
    MENU_PWADS,
    "Net",
    "Network game settings",
    net_opts, 1, 0, 0,
    UI_MenuNet_Init,
    UI_MenuNet_Update,
    UI_MenuNet_Draw,
    UI_MenuNet_Reload,
};

static struct Menu *self = &ui_menu_net;

void UI_MenuNet_Init(void)
{
    UI_MenuNet_Reload();
    // HACK: dynamic vars don't get loaded properly for Doom because it's
    //       selected on start, so got to do this stupid shit
    strncpy(net_opts[1].string, fs_profiles[ui_profile].joinaddr, MAX_STROPT);
}

void UI_MenuNet_Update(void)
{

}

void UI_MenuNet_Draw(void)
{

}

void UI_MenuNet_Reload(void)
{
    net_opts[0].codevar = &(fs_profiles[ui_profile].netmode);
    net_opts[1].codevar = fs_profiles[ui_profile].joinaddr;
    net_opts[3].codevar = fs_profiles[ui_profile].gmode;
    net_opts[4].codevar = &fs_profiles[ui_profile].maxplayers;

    self->opts = net_opts;
    self->numopts = 6;
}

static void NetStart(int server)
{
    if (!I_Question("The game will appear frozen until connection\n"
                    "is established and all players have joined.\n"
                    "You can press B to shut the game down at any\n"
                    "time before that happens.\n\n"
                    "Are you sure you want to continue?"))
        return;
    fs_profiles[ui_profile].netgame = server + 1;
    UI_SaveOptions();
    CFG_SaveAll();
    FS_ExecGame(ui_profile);
}

static void DoJoinNet(int arg)
{
    NetStart(0);
}

static void DoHost(int arg)
{
    NetStart(1);
}
