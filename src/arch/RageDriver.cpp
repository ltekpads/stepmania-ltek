#include "global.h"
#include "RageDriver.h"

void DriverList::Add( const istring &sName, const istring& sDisplayName, CreateRageDriverFn pfn )
{
	if( m_pRegistrees == NULL )
		m_pRegistrees = new map<istring, CreateRageDriverFn>;
	if (m_pDisplayNames == NULL)
		m_pDisplayNames = new map<istring, istring>;
	
	ASSERT( m_pRegistrees->find(sName) == m_pRegistrees->end() );
	ASSERT(m_pDisplayNames->find(sName) == m_pDisplayNames->end() );
	(*m_pRegistrees)[sName] = pfn;
	if(sDisplayName.length() > 0)
		(*m_pDisplayNames)[sName] = sDisplayName;
}

const RString DriverList::FindDisplayName(const RString& sDriverName)
{
	if( m_pDisplayNames == NULL )
		return "";

	auto match = m_pDisplayNames->find(istring(sDriverName));
	return match != m_pDisplayNames->end()
		? RString(match->second.c_str())
		: sDriverName;
}

RageDriver *DriverList::Create( const RString &sDriverName )
{
	if( m_pRegistrees == NULL )
		return NULL;

	map<istring, CreateRageDriverFn>::const_iterator iter = m_pRegistrees->find( istring(sDriverName) );
	if( iter == m_pRegistrees->end() )
		return NULL;
	return (iter->second)();
}

RegisterRageDriver::RegisterRageDriver( DriverList *pDriverList, const istring &sName, const istring &sDisplayName, CreateRageDriverFn pfn )
{
	pDriverList->Add( sName, sDisplayName, pfn );
}

/*
 * (c) 2006 Glenn Maynard
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
