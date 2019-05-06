#include "config.h"
#include "utils.h"
#include "input.h"
#include "screen.h"
#include "ui.h"
#include "files.h"
#include "configs.h"
#include "net.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static int init_level = 0;

void userAppInit(void)
{
    romfsInit();
}

void userAppExit(void)
{
    romfsExit();
}

void I_Cleanup(void)
{
    if (init_level >= 5)
      CFG_SaveAll();
    R_Free();
    NET_Free();
    IN_Free();
    UI_Free();
    FS_Free();
    CFG_FreeAll();
}

void I_FatalError(const char *error, ...)
{
    static char buf[4096];

    va_list argptr;
    va_start(argptr, error);
    vsnprintf(buf, sizeof(buf), error, argptr);
    va_end(argptr);

    FILE *ferr = fopen(BASEDIR "/launcher_error.log", "w");
    if (ferr)
    {
        fprintf(ferr, "%s\n\n", buf);
        fflush(ferr);
        fclose(ferr);
    }

    if (init_level > 0)
    {
        R_BeginDrawing();
        R_Clear(C_BLACK);
        R_PrintScaled(P_XCENTER, SCR_CX, SCR_CY-64, 2.f, C_RED, "FATAL ERROR");
        R_Print(P_XCENTER, SCR_CX, SCR_CY, C_WHITE, buf);
        R_Print(P_XCENTER | P_ABOTTOM, SCR_CX, SCR_H-32, C_LTGREY, "Press PLUS to exit");
        R_EndDrawing();
        IN_WaitForButton(B_PLUS);
    }

    I_Cleanup();
    exit(0);
}

void I_Error(const char *error, ...)
{
    static char buf[4096];

    va_list argptr;
    va_start(argptr, error);
    vsnprintf(buf, sizeof(buf), error, argptr);
    va_end(argptr);

    R_BeginDrawing();
    R_Clear(C_BLACK);
    R_PrintScaled(P_XCENTER, SCR_CX, SCR_CY-64, 2.f, C_ORANGE, "ERROR");
    R_Print(P_XCENTER, SCR_CX, SCR_CY, C_WHITE, buf);
    R_Print(P_XCENTER | P_ABOTTOM, SCR_CX, SCR_H-32, C_LTGREY, "Press A to continue");
    R_EndDrawing();
    IN_WaitForButton(B_A);
}

int I_Question(const char *text, ...)
{
    static char buf[4096];

    va_list argptr;
    va_start(argptr, text);
    vsnprintf(buf, sizeof(buf), text, argptr);
    va_end(argptr);

    R_BeginDrawing();
    R_Clear(C_BLACK);
    R_PrintScaled(P_XCENTER, SCR_CX, SCR_CY-64, 2.f, C_YELLOW, "WARNING");
    R_Print(P_XCENTER, SCR_CX, SCR_CY, C_WHITE, buf);
    R_Print(P_XCENTER | P_ABOTTOM, SCR_CX, SCR_H-32, C_LTGREY, "Press A to confirm");
    R_EndDrawing();
 
    return (IN_GetFirstButton() == B_A);
}

int main(void)
{
    if (R_Init()) I_FatalError("R_Init(): failed");
    init_level++;
    if (NET_Init()) I_FatalError("NET_Init(): failed");
    init_level++;
    if (FS_Init()) I_FatalError("FS_Init(): %s", FS_Error());
    init_level++;
    if (IN_Init()) I_FatalError("IN_Init(): failed");
    init_level++;
    if (CFG_LoadAll() > 6) I_FatalError("CFG_LoadAll(): failed");
    init_level++;
    if (UI_Init()) I_FatalError("UI_Init(): failed");
    init_level++;

    int finish = 0;
    do
    {
        IN_Update();
        finish = UI_Update();
        UI_Redraw();
    }
    while (!finish);

    // save any changes to profiles
    FS_SaveProfiles();
    I_Cleanup();
    return 0;
}
