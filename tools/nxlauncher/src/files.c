#include "config.h"
#include "utils.h"
#include "files.h"
#include "net.h"
#include "screen.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

static char fs_error[4096];
static char fs_cwd[1024];

// a bunch of profiles for potentially available IWADs
// the ones you don't have won't be shown in the menu

int fs_maxprofile = -1;
struct Profile fs_profiles[MAX_PROFILES] =
{
    { "Doom (Shareware)", "doom1.wad" },
    { "Doom", "doom.wad" },
    { "FreeDM", "freedm.wad" },
    { "FreeDoom: Phase 1", "freedoom1.wad" },
    { "FreeDoom: Phase 2", "freedoom2.wad" },
    { "Chex Quest", "chex.wad" },
    { "Doom II", "doom2.wad" },
    { "Final Doom: TNT Evilution", "tnt.wad" },
    { "Final Doom: The Plutonia Experiment", "plutonia.wad" },
    { "FreeDoom: Phase 2", "freedoom2.wad" },
    { "Heretic (Shareware)", "heretic1.wad" },
    { "Heretic", "heretic.wad" },
    { "Hexen", "hexen.wad" },
    { "Strife", "strife1.wad" },
    // custom profiles go here
};

static void SetError(const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(fs_error, sizeof(fs_error), fmt, argptr);
    va_end(argptr);
}

static int CheckForGame(int g)
{
    static char buf[512];
    snprintf(buf, sizeof(buf), BASEDIR "/%s", fs_profiles[g].iwad);
    int res = fexists(buf);
    snprintf(buf, sizeof(buf), BASEDIR "/iwads/%s", fs_profiles[g].iwad);
    return res || fexists(buf);
}

int FS_Init(void)
{
    mkdir(BASEDIR, 0755);
    mkdir(TMPDIR, 0755);
    mkdir(BASEDIR "/pwads", 0755);

    for (int i = 0; i < MAX_PROFILES; ++i)
    {
        fs_profiles[i].present = CheckForGame(i);
        fs_profiles[i].monsters[0] = '0';
        snprintf(fs_profiles[i].joinaddr, MAX_FNAME, "%s:5029", net_my_ip);
        if (fs_profiles[i].present) fs_maxprofile = i;
    }

    if (fs_maxprofile < 0)
    {
        SetError("No supported games were found in the GZDoom directory.\n" \
                 "Make sure you have installed at least one game properly.");
        return -1;
    }

    getcwd(fs_cwd, sizeof(fs_cwd)-1);

    return 0;
}

void FS_Free(void)
{

} 

char *FS_Error(void)
{
    return fs_error;
}

int FS_HaveGame(int game)
{
    if (game < 0 || game >= MAX_PROFILES) return 0;
    return fs_profiles[game].present;
}

int FS_ListDir(struct FileList *flist, const char *path, const char *ext)
{
    static char buf[512];

    DIR *dp = opendir(path);
    if (!dp) return -1;

    struct dirent *ep;
    while ((ep = readdir(dp)))
    {
        if (!ep->d_name || ep->d_name[0] == '.')
            continue;

        char *fname = ep->d_name;
        snprintf(buf, sizeof(buf), "%s/%s", path, fname);

        if (!isdir(buf) && !strcasecmp(fext(fname), ext))
        {
            flist->files = realloc(flist->files, (flist->numfiles + 1) * sizeof(char*));
            if (!flist->files) return -2;
            flist->files[flist->numfiles] = strdup(fname);
            flist->numfiles++;
        }
    }

    closedir(dp);

    return flist->numfiles;
}

void FS_FreeFileList(struct FileList *flist)
{
    for (int i = 0; i < flist->numfiles; ++i)
      free(flist->files[i]);

    free(flist->files);
    flist->files = NULL;
    flist->numfiles = 0;
}

static void WriteResponseFile(int game, const char *fname)
{
    struct Profile *g = fs_profiles + game;

    FILE *f = fopen(fname, "w");
    if (!f) I_Error("Could not create file\n" TMPDIR "/chocolat.rsp");

    if (g->netgame)
    {
        if (g->netmode)
            fprintf(f, "-netmode %d\n", g->netmode);
        if (g->netgame == 1)
        {
            fprintf(f, "-join %s\n", g->joinaddr);
        }
        else if (g->netgame == 2)
        {
            fprintf(f, "-host %d\n", g->maxplayers);
            if (g->gmode[0])
                fprintf(f, "-%s\n", g->gmode);
        }
    }

    fprintf(f, "-iwad %s\n", g->iwad);

    int deh = 0;
    for (int i = 0; i < MAX_DEHS; ++i)
        if (g->dehs[i][0]) { deh = 1; break; }
    if (deh)
    {
        fprintf(f, "-deh");
        for (int i = 0; i < MAX_DEHS; ++i)
            if (g->dehs[i][0])
                fprintf(f, " %s", g->dehs[i]);
        fprintf(f, "\n");
    }

    int file = 0;
    for (int i = 0; i < MAX_PWADS; ++i)
        if (g->pwads[i][0]) { file = 1; break; }
    if (file)
    {
        fprintf(f, "-file");

        for (int i = 0; i < MAX_PWADS; ++i)
            if (g->pwads[i][0])
                fprintf(f, " %s", g->pwads[i]);

        fprintf(f, "\n");
    }

    if (g->skill)
        fprintf(f, "-skill %d\n", g->skill);

    if (g->timer)
        fprintf(f, "-timer %d\n", g->timer);

    if (g->warp[0])
        fprintf(f, "+map %s\n", g->warp);

    if (g->charclass[0])
        fprintf(f, "+playerclass %s\n", g->charclass);

    if (g->monsters[0] == '1')
        fprintf(f, "-nomonsters\n");
    else if (g->monsters[0] == '2')
        fprintf(f, "-fast\n");
    else if (g->monsters[0] == '4')
        fprintf(f, "-respawn\n");
    else if (g->monsters[0] == '6')
        fprintf(f, "-fast\n-respawn\n");

    if (g->record)
    {
        fprintf(f, "-record %s/mydemo\n", RELATIVE_TMPDIR);
    }
    else if (g->demo[0])
    {
        fprintf(f, "-file %s\n", g->demo);
        char *dot = strrchr(g->demo, '.');
        if (dot && *(dot+1) != '.' && *(dot+1) != '/')
            *dot = '\0'; // playdemo doesn't want extensions
        fprintf(f, "-playdemo %s\n", g->demo);
    }

    if (g->log)
        fprintf(f, "+logfile gzdoom.log\n");

    if (g->ini[0])
        fprintf(f, "-config %s/%s\n", fs_cwd, g->ini);

    fclose(f);
}

void FS_ExecGame(int game)
{
    if (game < 0 || game >= MAX_PROFILES) return;

    struct Profile *g = fs_profiles + game;
    static char exe[1024];
    static char rsp[1024];
    static char argv[2048];

    if (g->rsp[0])
    {
        snprintf(rsp, sizeof(rsp), "@%s/%s", fs_cwd, g->rsp);
    }
    else
    {
        WriteResponseFile(game, TMPDIR "/args.rsp");
        snprintf(rsp, sizeof(rsp)-1, "@" RELATIVE_TMPDIR "/args.rsp");
    }

    // absolute path required
    snprintf(exe, sizeof(exe), "%s/%s/gzdoom.nro", fs_cwd, BASEDIR);
    snprintf(argv, sizeof(argv), "%s %s", exe, rsp);

    R_BeginDrawing();
    R_Clear(C_BLACK);
    R_EndDrawing();

    I_Cleanup();
    envSetNextLoad(exe, argv);
    exit(0);
}
