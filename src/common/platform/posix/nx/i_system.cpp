/*
** i_system.cpp
** Main startup code for Nintendo Switch
**
**---------------------------------------------------------------------------
** Copyright 1999-2016 Randy Heit
** Copyright 2019-2020 Christoph Oelckers
** Copyright 2019-2021 fgsfds
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include <dirent.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <unistd.h>

#include <stdarg.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <SDL.h>

#include <switch.h>

#include "version.h"
#include "cmdlib.h"
#include "m_argv.h"
#include "i_sound.h"
#include "i_interface.h"
#include "v_font.h"
#include "c_cvars.h"
#include "palutil.h"
#include "st_start.h"

#ifdef _DEBUG
static int nxlink_sock = -1;
#endif

double PerfToSec, PerfToMillisec;

extern FStartupScreen *StartScreen;

extern "C" void userAppInit(void) {
	socketInitializeDefault();
#ifdef _DEBUG
	nxlink_sock = nxlinkStdio();
#endif
}

extern "C" void userAppExit(void) {
#ifdef _DEBUG
	if (nxlink_sock >= 0)
		close(nxlink_sock);
#endif
	socketExit();
}

void I_SetIWADInfo()
{
}

//
// I_Error
//

void I_ShowFatalError(const char *message)
{
	SDL_Quit();
	FILE *f = fopen("error.log", "w");
	if (!f) return;
	fprintf(f, "FATAL ERROR:\n%s\n", message);
	fclose(f);
}

//
// Show OSK
//

bool I_OnScreenKeyboard(const char *hint, char *out, int outlen)
{
	if (!out || !outlen) return false;

	SwkbdConfig kbd;
	Result rc = swkbdCreate(&kbd, 0);
	if (R_FAILED(rc)) return false;

	swkbdConfigMakePresetDefault(&kbd);
	swkbdConfigSetHeaderText(&kbd, hint);
	swkbdConfigSetGuideText(&kbd, hint);
	swkbdConfigSetInitialText(&kbd, out);

	rc = swkbdShow(&kbd, out, outlen-1);
	swkbdClose(&kbd);

	return R_SUCCEEDED(rc);
}

void CalculateCPUSpeed()
{
}

void I_PrintStr(const char *cp)
{
	const char * srcp = cp;
	FString printData = "";

	while (*srcp != 0)
	{
		if (*srcp != 0x1c && *srcp != 0x1d && *srcp != 0x1e && *srcp != 0x1f)
		{
			printData += *srcp++;
		}
		else
		{
			if (srcp[1] != 0) srcp += 2;
			else break;
		}
	}

	fputs(printData.GetChars(),stdout);
}

int I_PickIWad (WadStuff *wads, int numwads, bool showwin, int defaultiwad)
{
	return defaultiwad;
}

void I_PutInClipboard (const char *str)
{
	SDL_SetClipboardText(str);
}

FString I_GetFromClipboard (bool use_primary_selection)
{
	if(char *ret = SDL_GetClipboardText())
	{
		FString text(ret);
		SDL_free(ret);
		return text;
	}
	return "";
}

// Return a random seed, preferably one with lots of entropy.
unsigned int I_MakeRNGSeed()
{
	// use a crappy seed from time().
	return time(NULL);
}
