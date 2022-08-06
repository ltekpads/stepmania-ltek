#include "global.h"
#include "PlayDataManager.h"
#include "RageFileManager.h"
#include "RageFile.h"
//you need to manuallly create a sqlite3 visual studio project and add a dependency to a static sqlite3 library in order to compile this code under visual studio
//pull requests adding sqlite3 to makefile definition are welcomed :)


PlayDataManager* PLAYDATA = NULL;
const RString PLAY_DATA_DB_FILE = "Save/PlayData.sqlite";

const char* CreateDb =
"CREATE TABLE Profiles ("
"	Id	INTEGER NOT NULL,"
"	Guid	TEXT NOT NULL,"
"	DisplayName	TEXT,"
"	LastUsedHighScoreName	TEXT,"
"	IsAvailable	INTEGER NOT NULL DEFAULT 0,"
"	PRIMARY KEY(Id AUTOINCREMENT)"
");"
"CREATE TABLE SongsPlayed("
"	Id	INTEGER,"
"	ProfileId	INTEGER NOT NULL,"
"	PlayerNumber	TEXT NOT NULL,"
"	GameType	TEXT NOT NULL,"
"	GameStyle	TEXT NOT NULL,"
"	StartDate	TEXT NOT NULL,"
"	EndDate	TEXT NOT NULL,"
"	ClearResult	TEXT NOT NULL,"
"	ChartHash	TEXT NOT NULL,"
"	SongInfo	TEXT NOT NULL,"
"	ChartInfo	TEXT NOT NULL,"
"	PlayResult	TEXT NOT NULL,"
"	StepStats	TEXT NOT NULL,"
"	DifficultyInfo	TEXT NOT NULL,"
"	FOREIGN KEY(ProfileId) REFERENCES Profiles(Id),"
"	PRIMARY KEY(Id AUTOINCREMENT)"
");";

PlayDataManager::PlayDataManager()
{
	const auto shouldInit = !FILEMAN->DoesFileExist(PLAY_DATA_DB_FILE);
	auto dbFile = ResolveDbPath(PLAY_DATA_DB_FILE);

	const auto result = sqlite3_open_v2(dbFile, &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	ASSERT(result == 0);

	if (shouldInit)
		sqlite3_exec(_db, CreateDb, nullptr, nullptr, nullptr);
}

const RString PlayDataManager::ResolveDbPath(const RString& path)
{
	const auto exists = FILEMAN->DoesFileExist(PLAY_DATA_DB_FILE);
	int error;
	const RageFileBasic* testAccess = FILEMAN->Open(path, exists ? RageFile::READ : RageFile::WRITE, error);
	ASSERT(testAccess != NULL);
	const auto fullPath = testAccess->GetDisplayPath();
	delete testAccess;
	return fullPath;
}

PlayDataManager::~PlayDataManager()
{
	sqlite3_close_v2(_db);
	_db = nullptr;
}


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
