#ifndef PlayDataManager_H
#define PlayDataManager_H
extern "C" {
#include "../extern/sqlite3/sqlite3.h"
}
#include "GameConstantsAndTypes.h"
#include "PlayerStageStats.h"
#include "PlayerState.h"
#include "DateTime.h"
#include "Song.h"
#include "NoteData.h"
#include "Profile.h"

enum PlayDataClearResult
{
	ClearResult_Cleared,
	ClearResult_FailedEndOfSong,
	ClearResult_FailedImmediate,
	ClearResult_Exited,
	NUM_PlayDataClearResult,
	PlayDataClearResult_Invalid,
};

struct PlayResult
{
	PlayerNumber playerNumber;
	const char* gameStyle;
	const char* gameType;
	PlayMode playMode;
	DateTime startDate;
	DateTime endDate;
	PlayDataClearResult result;
	const Steps* steps;
	const Song* song;
	const PlayerStageStats* stats;
	const NoteData* notes;
	const ModsGroup<PlayerOptions>* playerOptions;

	RString ToSongInfo() const;
	RString ToChartInfo() const;
	RString ToDifficultyInfo() const;
	RString ToStepStats() const;
	RString ToPlayResult() const;
};

class PlayDataManager
{
public:
	PlayDataManager();
	~PlayDataManager();

	void ActivateProfile(const Profile* profile);
	void SaveResult(const Profile* profile, const PlayResult& result);

private:
	sqlite3* _db;

	const RString ResolveDbPath(const RString& path);
	int GetProfile(const RString& guid);
	void CreateProfile(const RString& guid);
	int GetOrCreateProfile(const RString& guid);

	sqlite3_stmt* Prepare(const char* query);
};

extern PlayDataManager* PLAYDATA;

#endif

/*
 * (c) 2022 L-Tek
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
