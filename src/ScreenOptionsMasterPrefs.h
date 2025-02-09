#ifndef SCREEN_OPTIONS_MASTER_PREFS_H
#define SCREEN_OPTIONS_MASTER_PREFS_H

static const int MAX_OPTIONS=16;
#define OPT_SAVE_PREFERENCES			(1<<0)
#define OPT_APPLY_GRAPHICS				(1<<1)
#define OPT_APPLY_THEME					(1<<2)
#define OPT_CHANGE_GAME					(1<<3)
#define OPT_APPLY_SOUND					(1<<4)
#define OPT_APPLY_SONG					(1<<5)
#define OPT_APPLY_ASPECT_RATIO			(1<<6)
#define OPT_APPLY_LIGHTS			(1<<7)

struct ConfOption
{
	static ConfOption *Find( RString name );

	// Name of this option.
	RString name;

	// Name of the preference this option affects.
	RString m_sPrefName;

	typedef void (*MoveData_t)( int &sel, bool ToSel, const ConfOption *pConfOption );
	MoveData_t MoveData;
	int m_iEffects;
	bool m_bAllowThemeItems;

	/* For dynamic options, update the options. Since this changes the available
	 * options, this may invalidate the offsets returned by Get() and Put(). */
	void UpdateAvailableOptions();

	/* Return the list of available selections; Get() and Put() use indexes into
	 * this array. UpdateAvailableOptions() should be called before using this. */
	void MakeOptionsList( vector<RString> &out ) const;

	inline int Get() const { int sel; MoveData( sel, true, this ); return sel; }
	inline void Put( int sel ) const { MoveData( sel, false, this ); }
	int GetEffects() const;

	ConfOption( const char *n, MoveData_t m,
		const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL,
		const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL,
		const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL, const char *c20=NULL,
		const char *c21=NULL, const char *c22=NULL, const char *c23=NULL, const char *c24=NULL, const char *c25=NULL, const char *c26=NULL, const char *c27=NULL,
		const char *c28=NULL, const char *c29=NULL, const char *c30=NULL, const char *c31=NULL, const char *c32=NULL, const char *c33=NULL, const char *c34=NULL)
	{
		name = n;
		m_sPrefName = name; // copy from name (not n), to allow refcounting
		MoveData = m;
		MakeOptionsListCB = NULL;
		m_iEffects = 0;
		m_bAllowThemeItems = true;
#define PUSH( c )	if(c) names.push_back(c);
		PUSH(c0); PUSH(c1); PUSH(c2); PUSH(c3); PUSH(c4); PUSH(c5); PUSH(c6);
		PUSH(c7); PUSH(c8); PUSH(c9); PUSH(c10); PUSH(c11); PUSH(c12); PUSH(c13);
		PUSH(c14); PUSH(c15); PUSH(c16); PUSH(c17); PUSH(c18); PUSH(c19); PUSH(c20);
		PUSH(c21); PUSH(c22); PUSH(c23); PUSH(c24); PUSH(c25); PUSH(c26); PUSH(c27);
		PUSH(c28); PUSH(c29); PUSH(c30); PUSH(c31); PUSH(c32); PUSH(c33); PUSH(c34);
	}
	void AddOption( const RString &sName ) { PUSH(sName); }
#undef PUSH

	ConfOption( const char *n, MoveData_t m,
			void (*lst)( vector<RString> &out ) )
	{
		name = n;
		MoveData = m;
		MakeOptionsListCB = lst;
		m_iEffects = 0;
		m_bAllowThemeItems = false; // don't theme dynamic choices
	}


// private:
	vector<RString> names;
	void (*MakeOptionsListCB)( vector<RString> &out );
};

int GetLifeDifficulty();
int GetTimingDifficulty();

#endif

/**
 * @file
 * @author Glenn Maynard (c) 2003-2004
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
