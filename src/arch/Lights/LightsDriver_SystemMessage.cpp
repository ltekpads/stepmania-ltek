#include "global.h"
#include "LightsDriver_SystemMessage.h"
#include "ScreenManager.h"
#include "InputMapper.h"
#include "PrefsManager.h"

REGISTER_LIGHTS_DRIVER_CLASS(SystemMessage, "Debug");

LightsDriver_SystemMessage::LightsDriver_SystemMessage()
{
}

LightsDriver_SystemMessage::~LightsDriver_SystemMessage()
{
}

LightsDeviceType LightsDriver_SystemMessage::GetDeviceType() const
{
	return LIGHTSDEVICE_SOFTWARE;
}

void LightsDriver_SystemMessage::Set( const LightsState *ls )
{
	if( !PREFSMAN->m_bDebugLights )
		return;

	RString s;

	s += LightsModeToString(LIGHTSMAN->GetLightsMode()) + "\n";

	s += "Cabinet: ";
	FOREACH_CabinetLight( cl )
	{
		s += ls->m_bCabinetLights[cl] ? '1' : '0';
	}
	s += "\n";

	FOREACH_ENUM( GameController,  gc )
	{
		s += ssprintf("Controller%d: ",gc+1);
		FOREACH_ENUM( GameButton,  gb )
		{
			s += ls->m_bGameButtonLights[gc][gb] ? '1' : '0';
		}
		s += "\n";
	}
	FOREACH_ENUM(PlayerNumber, pn)
	{
		const LifebarState* bar = &ls->m_cLifeBarLights[pn];
		s += ssprintf("Lifebar%d: %i\n", pn + 1, bar->present ? (bar->lives > 0 ? bar->lives : bar->percent) : 0);
	}

	s += ssprintf("Beat: %d\n", ls->m_beat);

	SCREENMAN->SystemMessageNoAnimate( s );
}

/*
 * (c) 2003-2004 Chris Danford
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
