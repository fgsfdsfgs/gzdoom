#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

int isdir(const char *path);
void *fload(const char *path, size_t *fsize);
int fexists(const char *path);
const char *fext(char *filename);

char *strdup(const char *str);
char *strparse(char *data, char *token);

void I_Cleanup(void);
void I_Error(const char *error, ...);
void I_FatalError(const char *error, ...);
int I_Question(const char *text, ...);
