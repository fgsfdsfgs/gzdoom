/*
** st_start.cpp
** Handles the startup screen.
**
**---------------------------------------------------------------------------
** Copyright 2006-2007 Randy Heit
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

// HEADER FILES ------------------------------------------------------------

#include <unistd.h>
#include <sys/time.h>
#include <switch.h>

#include "st_start.h"
#include "i_system.h"
#include "c_cvars.h"
#include "engineerrors.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

class FSwitchStartupScreen : public FStartupScreen
{
	public:
		FSwitchStartupScreen(int max_progress);
		~FSwitchStartupScreen();

		void Progress();
		void NetInit(const char *message, int num_players);
		void NetProgress(int count);
		void NetMessage(const char *format, ...);	// cover for printf
		void NetDone();
		bool NetLoop(bool (*timer_callback)(void *), void *userdata);
	protected:
		PadState Pad;
		bool DidNetInit;
		int NetMaxPos, NetCurPos;
		const char *TheNetMessage;
};

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

FStartupScreen *StartScreen;

CUSTOM_CVAR(Int, showendoom, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
{
	if (self < 0) self = 0;
	else if (self > 2) self=2;
}

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static const char SpinnyProgressChars[4] = { '|', '/', '-', '\\' };

// CODE --------------------------------------------------------------------

//==========================================================================
//
// FStartupScreen :: CreateInstance
//
// Initializes the startup screen for the detected game.
// Sets the size of the progress bar and displays the startup screen.
//
//==========================================================================

FStartupScreen *FStartupScreen::CreateInstance(int max_progress)
{
	return new FSwitchStartupScreen(max_progress);
}

//===========================================================================
//
// FSwitchStartupScreen Constructor
//
// Sets the size of the progress bar and displays the startup screen.
//
//===========================================================================

FSwitchStartupScreen::FSwitchStartupScreen(int max_progress)
	: FStartupScreen(max_progress)
{
	DidNetInit = false;
	NetMaxPos = 0;
	NetCurPos = 0;
	TheNetMessage = NULL;
	padInitializeDefault(&Pad);
}

//===========================================================================
//
// FSwitchStartupScreen Destructor
//
// Called just before entering graphics mode to deconstruct the startup
// screen.
//
//===========================================================================

FSwitchStartupScreen::~FSwitchStartupScreen()
{
	NetDone();	// Just in case it wasn't called yet and needs to be.
}

//===========================================================================
//
// FSwitchStartupScreen :: Progress
//
//===========================================================================

void FSwitchStartupScreen::Progress()
{
	if (CurPos < MaxPos)
	{
		++CurPos;
	}
}

//===========================================================================
//
// FSwitchStartupScreen :: NetInit
//
// Sets stdin for unbuffered I/O, displays the given message, and shows
// a progress meter.
//
//===========================================================================

void FSwitchStartupScreen::NetInit(const char *message, int numplayers)
{
	if (!DidNetInit)
	{
		fprintf (stderr, "Press 'B' to abort network game synchronization.");
		DidNetInit = true;
	}
	if (numplayers == 1)
	{
		// Status message without any real progress info.
		fprintf (stderr, "\n%s.", message);
	}
	else
	{
		fprintf (stderr, "\n%s: ", message);
	}
	fflush (stderr);
	TheNetMessage = message;
	NetMaxPos = numplayers;
	NetCurPos = 0;
	NetProgress(1);		// You always know about yourself
}

//===========================================================================
//
// FSwitchStartupScreen :: NetDone
//
// Restores the old stdin tty settings.
//
//===========================================================================

void FSwitchStartupScreen::NetDone()
{	
	// Restore stdin settings
	if (DidNetInit)
	{
		printf ("\n");
		DidNetInit = false;
	}	
}

//===========================================================================
//
// FSwitchStartupScreen :: NetMessage
//
// Call this between NetInit() and NetDone() instead of Printf() to
// display messages, because the progress meter is mixed in the same output
// stream as normal messages.
//
//===========================================================================

void FSwitchStartupScreen::NetMessage(const char *format, ...)
{
	FString str;
	va_list argptr;
	
	va_start (argptr, format);
	str.VFormat (format, argptr);
	va_end (argptr);
	fprintf (stderr, "\r%-40s\n", str.GetChars());
}

//===========================================================================
//
// FSwitchStartupScreen :: NetProgress
//
// Sets the network progress meter. If count is 0, it gets bumped by 1.
// Otherwise, it is set to count.
//
//===========================================================================

void FSwitchStartupScreen::NetProgress(int count)
{
	int i;

	if (count == 0)
	{
		NetCurPos++;
	}
	else if (count > 0)
	{
		NetCurPos = count;
	}
	if (NetMaxPos == 0)
	{
		// Spinny-type progress meter, because we're a guest waiting for the host.
		fprintf (stderr, "\r%s: %c", TheNetMessage, SpinnyProgressChars[NetCurPos & 3]);
		fflush (stderr);
	}
	else if (NetMaxPos > 1)
	{
		// Dotty-type progress meter.
		fprintf (stderr, "\r%s: ", TheNetMessage);
		for (i = 0; i < NetCurPos; ++i)
		{
			fputc ('.', stderr);
		}
		fprintf (stderr, "%*c[%2d/%2d]", NetMaxPos + 1 - NetCurPos, ' ', NetCurPos, NetMaxPos);
		fflush (stderr);
	}
}

//===========================================================================
//
// FSwitchStartupScreen :: NetLoop
//
// The timer_callback function is called at least two times per second
// and passed the userdata value. It should return true to stop the loop and
// return control to the caller or false to continue the loop.
//
// ST_NetLoop will return true if the loop was halted by the callback and
// false if the loop was halted because the user wants to abort the
// network synchronization.
//
//===========================================================================

bool FSwitchStartupScreen::NetLoop(bool (*timer_callback)(void *), void *userdata)
{
	while (appletMainLoop())
	{
		// don't flood the network with packets
		// we can't select on input, so just sleep
		svcSleepThread(250000000);
		if (timer_callback (userdata))
		{
			fputc ('\n', stderr);
			return true;
		}
		// check if the user wants to abort netgame
		padUpdate(&Pad);
		if (padGetButtons(&Pad) & HidNpadButton_B)
		{
			fprintf (stderr, "\nNetwork game synchronization aborted.");
			return false;
		}
	}
	return false;
}

void ST_Endoom()
{
	throw CExitEvent(0);
}
