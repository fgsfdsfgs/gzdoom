#pragma once

enum ConfigVarTypes
{
    CVAR_UNKNOWN,
    CVAR_INTEGER,
    CVAR_DOUBLE,
    CVAR_STRING,
};

int CFG_LoadAll(void);
int CFG_SaveAll(void);
void CFG_FreeAll(void);

int CFG_Load(int game);
int CFG_Save(int game);

int CFG_ReadVar(int game, const char *name, void *dst);
int CFG_WriteVar(int game, const char *name, void *src);
int CFG_VarType(int game, const char *name);
