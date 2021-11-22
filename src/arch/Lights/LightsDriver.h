#ifndef LightsDriver_H
#define LightsDriver_H

#include "LightsManager.h"
#include "arch/RageDriver.h"

typedef bool (*CheckAvailabilityFn)();

struct AvailabilityList
{
	void Add(const istring& sName, CheckAvailabilityFn pfn);
	bool IsAvailable(const RString& sDriverName);
	map<istring, CheckAvailabilityFn>* m_pRegistrees;
};

struct RegisterAvailabilityCheck
{
	RegisterAvailabilityCheck(AvailabilityList* pList, const istring& sName, CheckAvailabilityFn pfn);
};

struct LightsState;
/** @brief Controls the lights. */
class LightsDriver: public RageDriver
{
public:
	static void Create( const RString &sDriver, vector<LightsDriver *> &apAdd );
	static const RString FindAvailable();
	static const void ListDrivers(vector<RString>& drivers);
	static const RString FindDisplayName(const RString& driverName);
	static DriverList m_pDriverList;
	static AvailabilityList m_pAvailabilityList;

	LightsDriver() {};
	virtual ~LightsDriver() {};

	virtual void Set( const LightsState *ls ) = 0;
};

#define REGISTER_LIGHTS_DRIVER_CLASS2( name, x, displayName ) \
	static RegisterRageDriver register_##x( &LightsDriver::m_pDriverList, #name, displayName, CreateClass<LightsDriver_##x, RageDriver> )
#define REGISTER_LIGHTS_DRIVER_CLASS( name, displayName ) REGISTER_LIGHTS_DRIVER_CLASS2( name, name, displayName )

#define REGISTER_LIGHTS_DRIVER_AVAILABILITY_CHECK2( name, x ) \
	static RegisterAvailabilityCheck check_##x( &LightsDriver::m_pAvailabilityList, #name, Check_##x )
#define REGISTER_LIGHTS_DRIVER_AVAILABILITY_CHECK( name ) REGISTER_LIGHTS_DRIVER_AVAILABILITY_CHECK2( name, name )

#endif


/**
 * @file
 * @author Chris Danford (c) 2003-2004
 * @section LICENSE
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
