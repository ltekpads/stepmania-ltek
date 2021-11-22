#include "global.h"
#include "LightsDriver_Win32LTek.h"
#include "windows.h"
#include "RageUtil.h"

REGISTER_LIGHTS_DRIVER_CLASS(Win32LTek, "L-Tek");
const int Vid = 0x483;
const int Pid = 0x5711;
const int HidCollection = 2;
const int ReportTypeSetLights = 10;

USBDevice* FindDevice()
{
	USBDevice* device = new USBDevice;
	if (device->Open(Vid, Pid, sizeof(HidReport<LTekLightsReport>), HidCollection-1, nullptr, USB_WRITE))
		return device;
	SAFE_DELETE(device);
	return nullptr;
}

static bool Check_Win32LTek()
{
	USBDevice* device = FindDevice();
	bool available = device;
	SAFE_DELETE(device);
	return available;
}

REGISTER_LIGHTS_DRIVER_AVAILABILITY_CHECK(Win32LTek);

LightsDriver_Win32LTek::LightsDriver_Win32LTek()
{
	m_pDevice = FindDevice();
}

LightsDriver_Win32LTek::~LightsDriver_Win32LTek()
{
	FreeDevice();
}

void LightsDriver_Win32LTek::FreeDevice()
{
	if (!m_pDevice)
		return;
	SAFE_DELETE(m_pDevice);
	m_pDevice = nullptr;
}

static RageTimer reconnectTimer;

char LifebarStateToByte(const LifebarState& state)
{
	if (!state.present)
		return 255;
	if (state.mode == LIFEBARMODE_PERCENTAGE)
		return state.percent;

	return 100 + state.lives;
}

char PackArray(const bool* values, int length)
{
	ASSERT(length < 8);
	char result = 0;
	for (int a = 0; a < length; a++)
		result |= (values[a] ? 1 << a : 0);
	return result;
}

void LightsDriver_Win32LTek::Set( const LightsState *ls )
{
	if (!m_pDevice)
	{
		if(reconnectTimer.Ago() < 1)
			return;
		m_pDevice = FindDevice();
		reconnectTimer.Touch();
		if (!m_pDevice)
			return;
	}

	HidReport<LTekLightsReport> report;
	ZERO( report );
	report.reportType = ReportTypeSetLights;
	LTekLightsReport* lights = &report.data;

	lights->beat = ls->m_beat;
	lights->command = LTekCommand::SET_LIGHTS;
	lights->commandFlags = 0;
	lights->lifeBarP1 = LifebarStateToByte(ls->m_cLifeBarLights[0]);
	lights->lifeBarP2 = LifebarStateToByte(ls->m_cLifeBarLights[1]);
	lights->lightsCabinet = PackArray(ls->m_bCabinetLights, NUM_CabinetLight);
	lights->lightsP1Game = PackArray(ls->m_bGameButtonLights[0] + DANCE_BUTTON_LEFT, 6);
	lights->lightsP1System = PackArray(ls->m_bGameButtonLights[0], 6);
	lights->lightsP2Game = PackArray(ls->m_bGameButtonLights[1] + DANCE_BUTTON_LEFT, 6);
	lights->lightsP2System = PackArray(ls->m_bGameButtonLights[1], 6);

	if (!m_pDevice->m_IO.write((char*)&report, sizeof(HidReport<LTekLightsReport>)))
	{
		//device was probably disconnected
		reconnectTimer.Touch();
		FreeDevice();
	}
}

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
