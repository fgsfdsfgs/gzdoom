#include "config.h"
#include "utils.h"
#include "files.h"
#include "configs.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_CVARLEN 100
#define MAX_CVARNAME 80

// TODO: GZDoom inis are different from Choc Doom CFGs
//       so we don't have config editing yet

struct ConfigVar
{
    int type;
    char name[MAX_CVARLEN];
    union
    {
        char sval[MAX_CVARLEN];
        int ival;
        double dval;
    };
};

struct Config
{
    int extended;
    int modified;
    const char *name;
    struct ConfigVar *cvars;
    int numcvars;
};

int CFG_LoadAll(void)
{
    int fail = 0;
    return fail;
}

int CFG_SaveAll(void)
{
    int fail = 0;
    return fail;
}

void CFG_FreeAll(void)
{
}

int CFG_Load(int game)
{
    return 0;
}

int CFG_Save(int game)
{
    return 0;
}

int CFG_ReadVar(int game, const char *name, void *dst)
{
    return 0;
}

int CFG_WriteVar(int game, const char *name, void *src)
{
    return 0;
}

int CFG_VarType(int game, const char *name)
{
    return CVAR_UNKNOWN;
}
