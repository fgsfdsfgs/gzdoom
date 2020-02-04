#pragma once

#define MAX_FNAME 512
#define MAX_PWADS 10
#define MAX_DEHS  2
#define MAX_PROFILES 32
#define MAX_ARGV 4096
#define BUILTIN_PROFILES 13

struct FileList
{
    char **files;
    int numfiles;
};

struct Profile
{
    char name[MAX_FNAME];
    char iwad[MAX_FNAME];
    int present;
    int loaded;

    char pwads[MAX_PWADS][MAX_FNAME];
    char dehs[MAX_DEHS][MAX_FNAME];
    char demo[MAX_FNAME];
    char rsp[MAX_FNAME];
    char ini[MAX_FNAME];
    char load[MAX_FNAME];

    int numpwads;
    int numdehs;

    int netgame;
    int netmode;
    char joinaddr[MAX_FNAME];
    char gmode[32];

    int log;
    int skill;
    int record;
    int timer;
    int maxplayers;
    char monsters[2];
    char warp[64];
    char charclass[64];
};

extern struct Profile fs_profiles[MAX_PROFILES];
extern int fs_maxprofile;

int FS_Init(void);
void FS_Free(void);
char *FS_Error(void);

int FS_HaveGame(int game);

int FS_ListDir(struct FileList *flist, const char *path, const char *ext);
void FS_FreeFileList(struct FileList *flist);

int FS_LoadProfiles(void);
int FS_SaveProfiles(void);
int FS_DeleteProfile(int game);
int FS_AddProfile(char *name, char *iwad);

void FS_ExecGame(int game);
