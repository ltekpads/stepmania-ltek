#ifndef LightsDriver_Win32LTek_H
#define LightsDriver_Win32LTek_H

#include "arch/Lights/LightsDriver.h"
#include "archutils/Win32/USB.h"

class LightsDriver_Win32LTek : public LightsDriver
{
public:
	LightsDriver_Win32LTek();
	virtual ~LightsDriver_Win32LTek();

	virtual void Set( const LightsState *ls );
private:
	USBDevice *m_pDevice;
	void FreeDevice();
};

enum LTekCommand : char
{
	SET_LIGHTS = 1,
};

typedef struct LTekLightsReport {
	/* command number, 1 = set lights */
	LTekCommand command;
	//not used, additional flags for a command
	char commandFlags;
	/*light indicating a beat */
	char beat;
	/* cabinet lights on first six bits */
	char lightsCabinet;
	char reserved1;
	char reserved2;
	/* player1 game input lights on first six bits of a value: left, right, up, down, up-left, up-rigtht */
	char lightsP1Game;
	/* player1 system input lights on first 5 bits: left, right, up, down, start */
	char lightsP1System;
	char reserved3;
	/* same as for p1 */
	char lightsP2Game;
	/* same as for p1 */
	char lightsP2System;
	char reserved4;
	char lifeBarP1;
	char lifeBarP2;
} LTekLightsReport;

#endif

/*
 * (c) 2021 Hikari
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
