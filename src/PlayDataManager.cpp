#include "global.h"
#include "PlayDataManager.h"
#include "RageFileManager.h"
#include "RageFile.h"
//you need to manuallly create a sqlite3 visual studio project and add a dependency to a static sqlite3 library in order to compile this code under visual studio
//pull requests adding sqlite3 to makefile definition are welcomed :)

void BindText(sqlite3_stmt* statement, int index, const char* text)
{
	const auto bindResult = text != nullptr
		? sqlite3_bind_text(statement, index + 1, text, strlen(text), 0)
		: sqlite3_bind_null(statement, index + 1);
	ASSERT(bindResult == SQLITE_OK);
}

void BindInt(sqlite3_stmt* statement, int index, int value)
{
	const auto bindResult = sqlite3_bind_int(statement, index + 1, value);
	ASSERT(bindResult == SQLITE_OK);
}


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

class StatementFinalizer
{
public:
	sqlite3_stmt* _statement;
	StatementFinalizer(sqlite3_stmt* stmt);
	~StatementFinalizer();
};

StatementFinalizer::StatementFinalizer(sqlite3_stmt* statement)
{
	ASSERT(statement != NULL);
	_statement = statement;
}

StatementFinalizer::~StatementFinalizer()
{
	if (_statement)
		sqlite3_finalize(_statement);
	_statement = nullptr;
}

PlayDataManager::PlayDataManager()
{
	const auto shouldInit = !FILEMAN->DoesFileExist(PLAY_DATA_DB_FILE);
	auto dbFile = ResolveDbPath(PLAY_DATA_DB_FILE);

	const auto result = sqlite3_open_v2(dbFile, &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	ASSERT(result == 0);

	if (shouldInit)
		sqlite3_exec(_db, CreateDb, nullptr, nullptr, nullptr);

	sqlite3_exec(_db, "udpate Profiles set IsAvailable=0 where IsAvailable=1;", nullptr, nullptr, nullptr);
}

const char* ClearResultToText(PlayDataClearResult result)
{
	switch (result)
	{
		case ClearResult_Cleared: return "cleared";
		case ClearResult_Exited: return "exited";
		case ClearResult_FailedEndOfSong: return "failedEndOfSong";
		case ClearResult_FailedImmediate: return "failedImmediate";
		default: ASSERT(false);
	}
}

void PlayDataManager::SaveResult(const RString& guid, const PlayResult& result)
{
	const auto query = Prepare("insert into SongsPlayed(ProfileId, PlayerNumber, GameType, GameStyle, StartDate, EndDate, ClearResult, ChartHash, SongInfo, ChartInfo, PlayResult, StepStats, DifficultyInfo) values(?,?,?,?,?,?,?,?,?,?,?,?,?);");
	StatementFinalizer f(query);

	auto profileId = GetOrCreateProfile(guid);
	auto startDate = result.startDate.GetString();
	auto endDate = result.endDate.GetString();

	BindInt(query, 0, profileId);
	BindInt(query, 1, result.playerNumber);
	BindText(query, 2, result.gameType);
	BindText(query, 3, result.gameStyle);
	BindText(query, 4, startDate);
	BindText(query, 5, endDate);
	BindText(query, 6, ClearResultToText(result.result));
	BindText(query, 7, "hash - todo");
	BindText(query, 8, "song info - todo");
	BindText(query, 9, "chart info - todo");
	BindText(query, 10, "play result - todo");
	BindText(query, 11, "step stats - todo");
	BindText(query, 12, "difficulty info - todo");

	const auto sqliteResult = sqlite3_step(query);
	ASSERT(sqliteResult == SQLITE_DONE);
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

void PlayDataManager::DeactivateProfile(const RString& guid)
{
	const auto id = GetProfile(guid);
	if (id < 0)
		return;
	const auto query = Prepare("update Profiles set IsAvailable=0 where Id = ?;");
	StatementFinalizer f(query);
	BindInt(query, 0, id);

	const auto result = sqlite3_step(query);
	ASSERT(result == SQLITE_DONE);
}

void PlayDataManager::ActivateProfile(const RString& guid, const RString& displayName, const RString& highScoreName)
{
	const auto query = Prepare("update Profiles set IsAvailable=1, DisplayName = ?, LastUsedHighScoreName = ? where Id = ?;");
	StatementFinalizer f(query);

	const auto id = GetOrCreateProfile(guid);
	BindText(query, 0, !displayName.empty() ? displayName.c_str() : nullptr);
	BindText(query, 1, !highScoreName.empty() ? highScoreName.c_str() : nullptr);
	BindInt(query, 2, id);

	const auto result = sqlite3_step(query);
	ASSERT(result == SQLITE_DONE);
}

sqlite3_stmt* PlayDataManager::Prepare(const char* query)
{
	sqlite3_stmt* queryHandle;
	const auto result = sqlite3_prepare_v2(_db, query, -1, &queryHandle, nullptr);
	ASSERT(result == SQLITE_OK);
	return queryHandle;
}

int PlayDataManager::GetOrCreateProfile(const RString& guid)
{
	auto existing = GetProfile(guid);
	if (!existing)
		CreateProfile(guid);
	const auto id = GetProfile(guid);
	ASSERT(id > 0);
	return id;
}


int PlayDataManager::GetProfile(const RString& guid)
{
	auto query = Prepare("select Id from Profiles where Guid = ? limit 1;");
	StatementFinalizer f(query);
	BindText(query, 0, guid.c_str());

	const auto queryResult = sqlite3_step(query);
	if (queryResult == SQLITE_DONE)
		return 0;

	ASSERT(queryResult == SQLITE_ROW);
	return sqlite3_column_int(query, 0);
}

void PlayDataManager::CreateProfile(const RString& guid)
{
	auto query = Prepare("insert into Profiles(Guid) values(?);");
	StatementFinalizer f(query);

	BindText(query, 0, guid.c_str());
	const auto result = sqlite3_step(query);
	ASSERT(result == SQLITE_DONE);
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
