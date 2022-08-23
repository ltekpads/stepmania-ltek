#include "global.h"
#include "LightsDriver_Win32LTek.h"
#include "windows.h"
#include "RageUtil.h"

struct VidPid { int vid; int pid; };
const VidPid DeviceIds[]{ {0x03EB,0x8004}, {0x03EB,0x800A}, {0x03EB,0x800B} };

REGISTER_LIGHTS_DRIVER_CLASS(Win32LTek, "L-TEK");
const int ReportTypeSetLights = 10;

bool HasDevice(int vid, int pid, int index, const vector<DeviceInfo>& devices)
{
	for (const auto& device : devices)
	{
		if (device.index == index && device.pid == pid && device.vid == device.vid)
			return true;
	}
	return false;
}

bool TrySendReport(USBDevice* device, const HidReport<LTekLightsReport>& report);

void FillDevices(int vid, int pid, vector<DeviceInfo>& target)
{
	int it = 0;
	while (true)
	{
		int index = it++;
		if (HasDevice(vid, pid, index, target))
			continue;
		USBDevice* device = new USBDevice;
		if (!device->Open(vid, pid, sizeof(HidReport<LTekLightsReport>), index, nullptr, USB_WRITE))
		{
			SAFE_DELETE(device);
			return;
		}

		HidReport<LTekLightsReport> report;
		ZERO(report);
		report.reportType = ReportTypeSetLights;
		if (!TrySendReport(device, report))
		{
			SAFE_DELETE(device);
			continue;
		}
		DeviceInfo info;
		info.pid = pid;
		info.vid = vid;
		info.present = true;
		info.index = index;
		info.device = device;
		target.push_back(info);
	}
}

void LoadDevices(vector<DeviceInfo>& target)
{
	for (int a = 0; a < ARRAYLEN(DeviceIds); a++)
		FillDevices(DeviceIds[a].vid, DeviceIds[a].pid, target);
}

void FreeDevice(DeviceInfo& device)
{
	if (!device.device)
		return;
	SAFE_DELETE(device.device);
	device.device = nullptr;
	device.present = false;
}

void FreeDevices(vector<DeviceInfo>& devices)
{
	for (auto& device : devices)
		FreeDevice(device);
	devices.clear();
}

static bool Check_Win32LTek()
{
	vector<DeviceInfo> devices;
	LoadDevices(devices);
	bool available = devices.size() > 0;
	FreeDevices(devices);
	return available;
}

REGISTER_LIGHTS_DRIVER_AVAILABILITY_CHECK(Win32LTek);

LightsDriver_Win32LTek::LightsDriver_Win32LTek()
{
	LoadDevices(m_devices);
}

LightsDriver_Win32LTek::~LightsDriver_Win32LTek()
{
	FreeDevices(m_devices);
}

LightsDeviceType LightsDriver_Win32LTek::GetDeviceType() const
{
	return LIGHTSDEVICE_HARDWARE;
}

char LifebarStateToByte(const LifebarState& state)
{
	if (!state.present)
		return 0;
	if (state.mode == LIFEBARMODE_NORMAL)
		return state.percent;

	return state.lives;
}

char PackArray(const bool* values, int length)
{
	ASSERT(length <= 8);
	char result = 0;
	for (int a = 0; a < length; a++)
		result |= (values[a] ? 1 << a : 0);
	return result;
}

char PackGameButtons(const bool* gameButtonLights)
{
	//left, right, up, down, up-left, up-right, down-left, down-right
	bool ltekOrder[] =
	{
		gameButtonLights[DANCE_BUTTON_LEFT],
		gameButtonLights[DANCE_BUTTON_RIGHT],
		gameButtonLights[DANCE_BUTTON_UP],
		gameButtonLights[DANCE_BUTTON_DOWN],
		gameButtonLights[PUMP_BUTTON_UPLEFT],
		gameButtonLights[PUMP_BUTTON_UPRIGHT],
		gameButtonLights[PUMP_BUTTON_DOWNLEFT],
		gameButtonLights[PUMP_BUTTON_DOWNRIGHT],
	};
	return PackArray(ltekOrder, 8);
}

LTekLifebarType LifebarStateToMode(const LifebarState& state)
{
	if (!state.present)
		return LTLT_NOT_PRESENT;
	if (state.mode == LIFEBARMODE_NORMAL)
		return LTLT_NORMAL;
	if (state.mode == LIFEBARMODE_BATTERY)
		return LTLT_BATTERY;
	if (state.mode == LIFEBARMODE_SURVIVAL)
		return LTLT_SURVIVAL;

	return LTLT_NORMAL;
}

char PackSystemButtons(const bool* gameButtonLights)
{
	//center, menu left, menu right, menu up, mnenu down, start, back, select
	bool ltekOrder[] =
	{
		gameButtonLights[PUMP_BUTTON_CENTER],
		gameButtonLights[GAME_BUTTON_MENULEFT],
		gameButtonLights[GAME_BUTTON_MENURIGHT],
		gameButtonLights[GAME_BUTTON_MENUUP],
		gameButtonLights[GAME_BUTTON_MENUDOWN],
		gameButtonLights[GAME_BUTTON_START],
		gameButtonLights[GAME_BUTTON_BACK],
		gameButtonLights[GAME_BUTTON_SELECT],
	};
	return PackArray(ltekOrder, 8);
}

bool TrySendReport(USBDevice* device, const HidReport<LTekLightsReport>& report)
{
	const HidReport<LTekLightsReport>* address = &report;
	return device->m_IO.write((const char*)address, sizeof(HidReport<LTekLightsReport>));

}

bool FindInvalidDevice(const vector<DeviceInfo>& devices, int& index)
{
	for (int a = 0; a < devices.size(); a++)
	{
		if (!devices[a].present)
		{
			index = a;
			return true;
		}
	}
	return false;
}

void LightsDriver_Win32LTek::DevicesChanged()
{
	FreeDevices(m_devices);
	LoadDevices(m_devices);
}

void LightsDriver_Win32LTek::Set( const LightsState *ls )
{
	HidReport<LTekLightsReport> report;
	ZERO( report );
	report.reportType = ReportTypeSetLights;
	LTekLightsReport* lights = &report.data;

	lights->beat = ls->m_beat;
	lights->command = LTekCommand::SET_LIGHTS;
	lights->commandFlags = 0;
	lights->lifeBarP1Type= LifebarStateToMode(ls->m_cLifeBarLights[0]);
	lights->lifeBarP1Value = LifebarStateToByte(ls->m_cLifeBarLights[0]);
	lights->lifeBarP2Type = LifebarStateToMode(ls->m_cLifeBarLights[1]);
	lights->lifeBarP2Value = LifebarStateToByte(ls->m_cLifeBarLights[1]);
	lights->lightsCabinet = PackArray(ls->m_bCabinetLights, NUM_CabinetLight);
	lights->lightsP1Game = PackGameButtons(ls->m_bGameButtonLights[0]);
	lights->lightsP1System = PackSystemButtons(ls->m_bGameButtonLights[0]);
	lights->lightsP2Game = PackGameButtons(ls->m_bGameButtonLights[1]);
	lights->lightsP2System = PackSystemButtons(ls->m_bGameButtonLights[1]);

	for (auto& device : m_devices)
	{
		if (!TrySendReport(device.device, report))
		{
			//device was probably disconnected
			device.present = false;
		}
	}

	int invalid;
	while (FindInvalidDevice(m_devices, invalid))
	{
		FreeDevice(m_devices[invalid]);
		m_devices.erase(m_devices.begin() + invalid);
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
