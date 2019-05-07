#include "config.h"
#include "utils.h"
#include "files.h"
#include "net.h"
#include "screen.h"

#define INI_STOP_ON_FIRST_ERROR 1
#include "ini.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

static char fs_error[4096];
static char fs_cwd[MAX_FNAME];

// a bunch of profiles for potentially available IWADs
// the ones you don't have won't be shown in the menu

int fs_maxprofile = -1;
struct Profile fs_profiles[MAX_PROFILES] =
{
    { "Doom (Shareware)", "doom1.wad" },
    { "Doom", "doom.wad" },
    { "FreeDM", "freedm.wad" },
    { "FreeDoom: Phase 1", "freedoom1.wad" },
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
        SetError("No supported IWADs were found in the IWADs directory.\n" \
                 "Make sure you have at least one supported IWAD.");
        return -1;
    }

    // load profiles from disk
    FS_LoadProfiles();

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

static int kvhandler(void *user, const char *section, const char *key, const char *val) {
    // FIXME: this is really fucking slow but whatever

    struct Profile *p = NULL;
    for (int i = 0; i < MAX_PROFILES; ++i)
    {
        if (!strncmp(section, fs_profiles[i].name, sizeof(fs_profiles[i].name)-1))
        {
          p = fs_profiles + i;
          p->loaded = 1; // will be saved back to the file later
          break;
        }
    }

    if (!p)
    {
        // don't overwrite builtins
        if (fs_maxprofile < BUILTIN_PROFILES)
            fs_maxprofile = BUILTIN_PROFILES;
        if (++fs_maxprofile >= MAX_PROFILES)
        {
            fs_maxprofile = MAX_PROFILES-1;
            SetError("Too many profiles (max %d)", MAX_PROFILES);
            return 0;
        }
        p = fs_profiles + fs_maxprofile;
        p->present = 1; // custom profiles are always present
        p->loaded = 1; // will be saved back to the file later
        strncpy(p->name, section, sizeof(p->name)-1);
    }

    if (!p->present) return 1; // no point parsing profiles that can't be played

    /*  */ if (!strncasecmp(key, "iwad", 4)) {
        strncpy(p->iwad, val, sizeof(p->iwad)-1);
    } else if (!strncasecmp(key, "file", 4)) {
        if (p->numpwads >= MAX_PWADS) {
            SetError("Too many custom files (max %d)", MAX_PWADS);
            return 0;
        }
        strncpy(p->pwads[p->numpwads++], val, sizeof(p->pwads[0])-1);
    } else if (!strncasecmp(key, "dehfile", 7)) {
        if (p->numdehs >= MAX_DEHS) {
            SetError("Too many DEH files (max %d)", MAX_DEHS);
            return 0;
        }
        strncpy(p->dehs[p->numdehs++], val, sizeof(p->dehs[0])-1);
    } else if (!strncasecmp(key, "demo", 4)) {
      strncpy(p->demo, val, sizeof(p->demo)-1);
    } else if (!strncasecmp(key, "overridersp", 11)) {
      strncpy(p->rsp, val, sizeof(p->rsp)-1);
    } else if (!strncasecmp(key, "overrideini", 11)) {
      strncpy(p->ini, val, sizeof(p->ini)-1);
    } else if (!strncasecmp(key, "loadgame", 8)) {
      strncpy(p->load, val, sizeof(p->load)-1);
    } else if (!strncasecmp(key, "netmode", 7)) {
      p->netmode = atoi(val);
    } else if (!strncasecmp(key, "joinaddr", 8)) {
      strncpy(p->joinaddr, val, sizeof(p->joinaddr)-1);
    } else if (!strncasecmp(key, "gamemode", 8)) {
      strncpy(p->gmode, val, sizeof(p->gmode)-1);
    } else if (!strncasecmp(key, "writelog", 8)) {
      p->log = atoi(val);
    } else if (!strncasecmp(key, "skill", 5)) {
      p->skill = atoi(val);
    } else if (!strncasecmp(key, "recorddemo", 10)) {
      p->record = atoi(val);
    } else if (!strncasecmp(key, "timer", 5)) {
      p->timer = atoi(val);
    } else if (!strncasecmp(key, "maxplayers", 10)) {
      p->maxplayers = atoi(val);
    } else if (!strncasecmp(key, "monsters", 8)) {
      p->monsters[0] = val[0];
    } else if (!strncasecmp(key, "startmap", 8)) {
      strncpy(p->warp, val, sizeof(p->warp)-1);
    } else if (!strncasecmp(key, "playerclass", 11)) {
      strncpy(p->charclass, val, sizeof(p->charclass)-1);
    }

    return 1;
}

int FS_LoadProfiles(void)
{
    FILE *f = fopen(BASEDIR "/launcher.ini", "r");
    if (!f) return -666;

    int ret = ini_parse_file(f, kvhandler, NULL);
    fclose(f);

    if (ret <= 0)
      return ret; // a file/memory error or success

    // parsing error on line ret
    I_Error("Error parsing launcher.ini (line %d):\n%s", ret, FS_Error());
    return -667;
}

static inline void SaveProfile(FILE *f, struct Profile *p)
{
    fprintf(f, "[%s]\n", p->name);
    fprintf(f, "IWAD = %s\n", p->iwad);

    for (int i = 0; i < MAX_PWADS; ++i)
        if (p->pwads[i][0])
            fprintf(f, "File = %s\n", p->pwads[i]);
    for (int i = 0; i < MAX_DEHS; ++i)
        if (p->dehs[i][0])
            fprintf(f, "DEHFile = %s\n", p->dehs[i]);

    if (p->demo[0]) fprintf(f, "Demo = %s\n", p->demo);
    if (p->rsp[0]) fprintf(f, "OverrideRSP = %s\n", p->rsp);
    if (p->ini[0]) fprintf(f, "OverrideINI = %s\n", p->ini);
    if (p->load[0]) fprintf(f, "LoadGame = %s\n", p->load);

    if (p->netmode) fprintf(f, "NetMode = %d\n", p->netmode);
    if (p->joinaddr[0]) fprintf(f, "JoinAddr = %s\n", p->joinaddr);
    if (p->gmode[0]) fprintf(f, "GameMode = %s\n", p->gmode);
    if (p->maxplayers) fprintf(f, "MaxPlayers = %d\n", p->maxplayers);

    if (p->log) fprintf(f, "WriteLog = %d\n", p->log);
    if (p->skill) fprintf(f, "Skill = %d\n", p->skill);
    if (p->record) fprintf(f, "RecordDemo = %d\n", p->record);
    if (p->timer) fprintf(f, "Timer = %d\n", p->timer);

    if (p->monsters[0]) fprintf(f, "Monsters = %s\n", p->monsters);
    if (p->warp[0]) fprintf(f, "StartMap = %s\n", p->warp);
    if (p->charclass[0]) fprintf(f, "PlayerClass = %s\n", p->charclass);

    fprintf(f, "\n");
}

int FS_SaveProfiles(void)
{
    FILE *f = fopen(BASEDIR "/launcher.ini", "w");
    if (!f) return -1;

    for (struct Profile *p = fs_profiles; p <= fs_profiles + fs_maxprofile; ++p)
    {
        // no point in saving nonexistent profiles
        if (!p->present && !p->loaded) continue;
        SaveProfile(f, p);
    }

    fclose(f);
    return 0;
}

int FS_DeleteProfile(int game)
{
    if (game < BUILTIN_PROFILES || game >= MAX_PROFILES)
        return -1;

    if (!fs_profiles[game].present)
        return -2;

    fs_profiles[game].present = 0;
    fs_profiles[game].loaded = 0;

    if (game == fs_maxprofile)
        for (int i = game; i >= 0; --i)
            if (fs_profiles[i].present)
                fs_maxprofile = i;

    return 0;
}

int FS_AddProfile(char *name, char *iwad)
{
    if (!name || !iwad || !name[0] || !iwad[0])
        return -1;

    if (fs_maxprofile >= MAX_PROFILES-1)
        return -2;

    // don't overwrite builtins
    if (fs_maxprofile < BUILTIN_PROFILES)
        fs_maxprofile = BUILTIN_PROFILES;

    ++fs_maxprofile;
    fs_profiles[fs_maxprofile].present = 1;
    fs_profiles[fs_maxprofile].loaded = 1;
    strncpy(fs_profiles[fs_maxprofile].name, name, sizeof(fs_profiles[fs_maxprofile].name)-1);
    strncpy(fs_profiles[fs_maxprofile].iwad, iwad, sizeof(fs_profiles[fs_maxprofile].iwad)-1);

    return 0;
}

static char *argptr = NULL;
static int argsize = 0;
static void argprintf(const char *fmt, ...)
{
    static char argbuf[4096];

    va_list vaptr;
    va_start(vaptr, fmt);
    int ret = vsnprintf(argbuf, sizeof(argbuf)-1, fmt, vaptr);
    va_end(vaptr);

    if (ret < 0 || argptr + ret >= argptr + argsize)
        return;

    strncat(argptr, argbuf, argsize);

    argptr += ret;
}

static void WriteArgv(int game, char *argv, int size)
{
    struct Profile *g = fs_profiles + game;

    argptr = argv;
    argsize = size;

    if (g->netgame)
    {
        if (g->netmode)
            argprintf("-netmode %d ", g->netmode);
        if (g->netgame == 1)
        {
            argprintf("-join %s ", g->joinaddr);
        }
        else if (g->netgame == 2)
        {
            argprintf("-host %d ", g->maxplayers);
            if (g->gmode[0])
                argprintf("-%s ", g->gmode);
        }
    }

    argprintf("-iwad \"%s\" ", g->iwad);

    int deh = 0;
    for (int i = 0; i < MAX_DEHS; ++i)
        if (g->dehs[i][0]) { deh = 1; break; }
    if (deh)
    {
        argprintf("-deh");
        for (int i = 0; i < MAX_DEHS; ++i)
            if (g->dehs[i][0])
                argprintf(" \"%s\"", g->dehs[i]);
        argprintf(" ");
    }

    int file = 0;
    for (int i = 0; i < MAX_PWADS; ++i)
        if (g->pwads[i][0]) { file = 1; break; }
    if (file)
    {
        argprintf("-file");

        for (int i = 0; i < MAX_PWADS; ++i)
            if (g->pwads[i][0])
                argprintf(" \"%s\"", g->pwads[i]);

        argprintf(" ");
    }

    if (g->skill)
        argprintf("-skill %d ", g->skill);

    if (g->timer)
        argprintf("-timer %d ", g->timer);

    if (g->warp[0])
        argprintf("+map %s ", g->warp);

    if (g->charclass[0])
        argprintf("+playerclass %s ", g->charclass);

    if (g->monsters[0] == '1')
        argprintf("-nomonsters ");
    else if (g->monsters[0] == '2')
        argprintf("-fast ");
    else if (g->monsters[0] == '4')
        argprintf("-respawn ");
    else if (g->monsters[0] == '6')
        argprintf("-fast -respawn ");

    if (g->record)
    {
        argprintf("-record \"%s/mydemo\" ", RELATIVE_TMPDIR);
    }
    else if (g->demo[0])
    {
        argprintf("-file \"%s\" ", g->demo);
        char *dot = strrchr(g->demo, '.');
        if (dot && *(dot+1) != '.' && *(dot+1) != '/')
            *dot = '\0'; // playdemo doesn't want extensions
        argprintf("-playdemo \"%s\" ", g->demo);
    }

    if (g->log)
        argprintf("+logfile gzdoom.log ");

    if (g->ini[0])
        argprintf("-config \"%s\" ", g->ini);

    if (g->load[0])
        argprintf("-loadgame \"save/%s\" ", g->load);
}

void FS_ExecGame(int game)
{
    if (game < 0 || game >= MAX_PROFILES) return;

    struct Profile *g = fs_profiles + game;
    static char exe[MAX_FNAME + 32];
    static char argsbuf[MAX_ARGV + 1];
    static char argv[MAX_ARGV + MAX_FNAME + 64];

    // save any changes to profiles
    FS_SaveProfiles();

    // pass rsp instead of args if specified, otherwise generate argv
    if (g->rsp[0])
        snprintf(argsbuf, sizeof(argsbuf), "@%s/%s", fs_cwd, g->rsp);
    else
        WriteArgv(game, argsbuf, sizeof(argsbuf));

    // absolute path required
    snprintf(exe, sizeof(exe), "%s/%s/gzdoom.nro", fs_cwd, BASEDIR);
    // argv[0] = nro, argv[rest] = args
    snprintf(argv, sizeof(argv), "%s %s", exe, argsbuf);

    I_Cleanup();
    envSetNextLoad(exe, argv);
    exit(0);
}
