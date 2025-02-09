#include "global.h"
#include "PlayDataManager.h"
#include "RageFileManager.h"
#include "NoteDataUtil.h"
#include "RageFile.h"
#include "Player.h"
#include "json/value.h"
#include "json/writer.h"
#include "LifeMeterBar.h"
#include "GameState.h"
#include "ScreenOptionsMasterPrefs.h"
#include "CryptManager.h"

static const char* PlayDataEventNames[] = {
	"SongStart",
	"SongEnd",
};
XToString(PlayDataEvent);

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

void BindNull(sqlite3_stmt* statement, int index)
{
	const auto bindResult = sqlite3_bind_null(statement, index + 1);
	ASSERT(bindResult == SQLITE_OK);
}

PlayDataManager* PLAYDATA = NULL;
const RString PLAY_DATA_DB_FILE = "Save/PlayData.sqlite";

const char* CreateDb =
"CREATE TABLE Profiles ("
"	Id	INTEGER NOT NULL,"
"	Guid	TEXT NOT NULL,"
"	DisplayName	TEXT,"
"	CreateDateUtc	TEXT NOT NULL,"
"	LastUsedHighScoreName	TEXT,"
"	LastActivationDateUtc	TEXT NOT NULL,"
"	PRIMARY KEY(Id AUTOINCREMENT)"
");"
"CREATE TABLE GameEvents ("
"	Id	INTEGER NOT NULL,"
"	EventDateUtc	TEXT NOT NULL,"
"	EventType	TEXT NOT NULL,"
"	Details	TEXT NULL,"
"	PRIMARY KEY(Id AUTOINCREMENT)"
");"
"CREATE TABLE SongsPlayed("
"	Id	INTEGER,"
"	Guid	TEXT NOT NULL,"
"	ProfileId	INTEGER NULL,"
"	PlayerNumber	TEXT NOT NULL,"
"	PlayMode	TEXT NOT NULL,"
"	GameType	TEXT NOT NULL,"
"	GameStyle	TEXT NOT NULL,"
"	StartDateUtc	TEXT NOT NULL,"
"	EndDateUtc	TEXT NOT NULL,"
"	ClearResult	TEXT NOT NULL,"
"	ChartHash	INTEGER NOT NULL,"
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

void sqliteErrorCallback(void* argument, int errorCode, const char* message)
{
	LOG->Trace(message);
}

PlayDataManager::PlayDataManager()
{
	sqlite3_config(SQLITE_CONFIG_LOG, sqliteErrorCallback, 0);
	const auto shouldInit = !FILEMAN->DoesFileExist(PLAY_DATA_DB_FILE);
	auto dbFile = ResolveDbPath(PLAY_DATA_DB_FILE);

	const auto result = sqlite3_open_v2(dbFile, &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	ASSERT(result == 0);

	if (shouldInit)
		sqlite3_exec(_db, CreateDb, nullptr, nullptr, nullptr);

	//keep a maximum of 10k latest rows:
	sqlite3_exec(_db, "delete from SongsPlayed where Id < (select Id from SongsPlayed order by Id desc limit 1 offset 10000);", nullptr, nullptr, nullptr);
	sqlite3_exec(_db, "delete from GameEvents where Id < (select Id from GameEvents order by Id desc limit 1 offset 10000);", nullptr, nullptr, nullptr);
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

#define GUID_SIZE_BYTES 8

RString MakeGuid()
{
	RString s;
	s.reserve(GUID_SIZE_BYTES * 2);
	unsigned char buf[GUID_SIZE_BYTES];
	CryptManager::GetRandomBytes(buf, GUID_SIZE_BYTES);
	for (unsigned i = 0; i < GUID_SIZE_BYTES; i++)
		s += ssprintf("%02x", buf[i]);
	return s;
}

const char* ResultInsertStatement =
"insert into SongsPlayed"
"("
"	ProfileId, PlayerNumber, GameType,"
"	GameStyle, StartDateUtc, EndDateUtc,"
"	ClearResult, ChartHash, SongInfo,"
"	ChartInfo, PlayResult, StepStats,"
"	DifficultyInfo, PlayMode, Guid"
") values("
"	?,?,?,"
"	?,?,?,"
"	?,?,?,"
"	?,?,?,"
"	?,?,?"
");";

void PlayDataManager::SaveResult(const Profile* profile, const PlayResult& result)
{
	const auto query = Prepare(ResultInsertStatement);
	StatementFinalizer f(query);

	auto profileId = profile && profile->m_Type == ProfileType_Normal && !profile->m_sDisplayName.empty() ? GetOrCreateProfile(profile->m_sGuid) : 0;

	RString notesCompressed;
	NoteDataUtil::GetSMNoteDataString(*result.notes, notesCompressed);

	if (profileId > 0)
		BindInt(query, 0, profileId);
	else
		BindNull(query, 0);
	BindInt(query, 1, result.playerNumber);
	BindText(query, 2, result.gameType);
	BindText(query, 3, result.gameStyle);

	auto startDate = result.startDate.GetString();
	BindText(query, 4, startDate);

	auto endDate = result.endDate.GetString();
	BindText(query, 5, endDate);

	BindText(query, 6, ClearResultToText(result.result));
	BindInt(query, 7, GetHashForString(notesCompressed));

	auto songInfo = result.ToSongInfo();
	BindText(query, 8, songInfo);

	auto chartInfo = result.ToChartInfo();
	BindText(query, 9, chartInfo);

	auto playResult = result.ToPlayResult();
	BindText(query, 10, playResult);

	auto stepStats = result.ToStepStats();
	BindText(query, 11, stepStats);

	auto difficulty = result.ToDifficultyInfo();
	BindText(query, 12, difficulty);

	auto playMode = PlayModeToString(result.playMode);
	BindText(query, 13, playMode);

	const auto guid = MakeGuid();
	BindText(query, 14, guid);

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

void PlayDataManager::ActivateProfile(const Profile* profile)
{
	if (profile->m_Type != ProfileType_Normal || profile->m_sDisplayName.empty())
		return;
	const auto query = Prepare("update Profiles set LastActivationDateUtc=?, DisplayName = ?, LastUsedHighScoreName = ? where Id = ?;");
	StatementFinalizer f(query);

	RString now = DateTime::GetNowDateTimeUtc().GetString();
	const auto id = GetOrCreateProfile(profile->m_sGuid);
	const auto displayName = profile->m_sDisplayName;
	const auto highScoreName = profile->m_sLastUsedHighScoreName;
	BindText(query, 0, now);
	BindText(query, 1, !displayName.empty() ? displayName.c_str() : nullptr);
	BindText(query, 2, !highScoreName.empty() ? highScoreName.c_str() : nullptr);
	BindInt(query, 3, id);

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
	auto query = Prepare("insert into Profiles(Guid, CreateDateUtc, LastActivationDateUtc) values(?, ?, ?);");
	StatementFinalizer f(query);

	RString now = DateTime::GetNowDateTimeUtc().GetString();
	BindText(query, 0, guid.c_str());
	BindText(query, 1, now);
	BindText(query, 2, now);
	const auto result = sqlite3_step(query);
	ASSERT(result == SQLITE_DONE);
}

PlayDataManager::~PlayDataManager()
{
	sqlite3_close_v2(_db);
	_db = nullptr;
}

RString WriteJson(Json::Value& root)
{
	Json::FastWriter writer;
	return writer.write(root);
}

RString SerializeSongInfo(const Song* song)
{
	Json::Value root;

	root["title"] = song->m_sMainTitle;
	root["subtitle"] = song->m_sSubTitle;
	root["artist"] = song->m_sArtist;
	root["titleTranslit"] = song->GetTranslitMainTitle();
	root["subtitleTranslit"] = song->GetTranslitSubTitle();
	root["artistTranslit"] = song->GetTranslitArtist();
	root["credit"] = song->m_sCredit;
	root["genre"] = song->m_sGenre;
	root["origin"] = song->m_sOrigin;

	return WriteJson(root);
}

RString PlayResult::ToSongInfo() const
{
	return SerializeSongInfo(song);
}

RString PlayResult::ToChartInfo() const
{
	Json::Value root;
	root["stepsType"] = steps->m_StepsTypeStr;
	root["difficulty"] = DifficultyToString(steps->GetDifficulty());
	root["meter"] = steps->GetMeter();
	root["description"] = steps->GetDescription();
	root["credit"] = steps->GetCredit();
	root["chartName"] = steps->GetChartName();
	root["chartStyle"] = steps->GetChartStyle();

	return WriteJson(root);
}

double roundFloat(float value)
{
	return ((int)roundf(value * 10000))/10000.0;
}

RString PlayResult::ToDifficultyInfo() const
{
	Json::Value root;

	Json::Value timing;

	timing["timingDifficulty"] = GetTimingDifficulty();
	timing["timingWindowAdd"] = roundFloat(Player::GetWindowAdd());
	timing["timingWindowJump"] = roundFloat(Player::GetWindowJump());
	timing["timingWindowScale"] = roundFloat(Player::GetWindowScale());
	timing["timingWindowSecondsAttack"] = roundFloat(Player::GetWindowSeconds(TW_Attack));
	timing["timingWindowSecondsCheckpoint"] = roundFloat(Player::GetWindowSeconds(TW_Checkpoint));
	timing["timingWindowSecondsHold"] = roundFloat(Player::GetWindowSeconds(TW_Hold));
	timing["timingWindowSecondsMine"] = roundFloat(Player::GetWindowSeconds(TW_Mine));
	timing["timingWindowSecondsRoll"] = roundFloat(Player::GetWindowSeconds(TW_Roll));
	timing["timingWindowSecondsW1"] = roundFloat(Player::GetWindowSeconds(TW_W1));
	timing["timingWindowSecondsW2"] = roundFloat(Player::GetWindowSeconds(TW_W2));
	timing["timingWindowSecondsW3"] = roundFloat(Player::GetWindowSeconds(TW_W3));
	timing["timingWindowSecondsW4"] = roundFloat(Player::GetWindowSeconds(TW_W4));
	timing["timingWindowSecondsW5"] = roundFloat(Player::GetWindowSeconds(TW_W5));
	root["timing"] = timing;

	Json::Value life;
	LifeMeterBar bar;

	life["lifeDifficulty"] = GetTimingDifficulty();
	life["harshHotLifePenalty"] = (bool)PREFSMAN->m_HarshHotLifePenalty;
	life["lifeDifficultyScale"] = (float)PREFSMAN->m_fLifeDifficultyScale;
	life["progressiveLifebar"] = (int)PREFSMAN->m_iProgressiveLifebar;
	life["progressiveStageLifebar"] = (int)PREFSMAN->m_iProgressiveStageLifebar;
	life["initialValue"] = roundFloat(bar.GetLifeInitialValue());
	life["minStayAlive"] = TapNoteScoreToString(bar.GetMinStayAlive());
	life["forceLifeDifficultyOnExtraStage"] = bar.GetForceLifeDifficultyOnExtraStage();
	life["extraStageLifeDifficulty"] = roundFloat(bar.GetExtraStageLifeDifficulty());
	life["lifePercentChangeW1"] = roundFloat(bar.GetLifePercentChange(SE_W1));
	life["lifePercentChangeW2"] = roundFloat(bar.GetLifePercentChange(SE_W2));
	life["lifePercentChangeW3"] = roundFloat(bar.GetLifePercentChange(SE_W3));
	life["lifePercentChangeW4"] = roundFloat(bar.GetLifePercentChange(SE_W4));
	life["lifePercentChangeW5"] = roundFloat(bar.GetLifePercentChange(SE_W5));
	life["lifePercentChangeMiss"] = roundFloat(bar.GetLifePercentChange(SE_Miss));
	life["lifePercentChangeHitMine"] = roundFloat(bar.GetLifePercentChange(SE_HitMine));
	life["lifePercentChangeHeld"] = roundFloat(bar.GetLifePercentChange(SE_Held));
	life["lifePercentChangeLetGo"] = roundFloat(bar.GetLifePercentChange(SE_LetGo));
	life["lifePercentChangeMissedHold"] = roundFloat(bar.GetLifePercentChange(SE_Missed));
	life["lifePercentChangeCheckpointMiss"] = roundFloat(bar.GetLifePercentChange(SE_CheckpointMiss));
	life["lifePercentChangeCheckpointHit"] = roundFloat(bar.GetLifePercentChange(SE_CheckpointHit));
	root["life"] = life;

	return WriteJson(root);
}

RString PlayResult::ToStepStats() const
{
	Json::Value root;
	const auto timing = GAMESTATE->GetProcessedTimingData();
	GAMESTATE->SetProcessedTimingData(steps->GetTimingData());
	root["taps"] = notes->GetNumTapNotes();
	root["jumps"] = notes->GetNumJumps();
	root["holds"] = notes->GetNumHoldNotes();
	root["mines"] = notes->GetNumMines();
	root["rolls"] = notes->GetNumRolls();
	root["lifts"] = notes->GetNumLifts();
	root["fakes"] = notes->GetNumFakes();
	GAMESTATE->SetProcessedTimingData(timing);

	return WriteJson(root);
}

RString PlayResult::ToPlayResult() const
{
	const RadarValues& radar = steps->GetRadarValues(playerNumber);

	Json::Value root;
	root["percentageScore"] = roundFloat(stats->GetPercentDancePoints());
	root["pointsScore"] = stats->m_iActualDancePoints;
	root["casualScore"] = stats->m_iScore;
	root["grade"] = (int)stats->GetGrade();

	Json::Value tapScores;
	tapScores["W1"] = stats->m_iTapNoteScores[TNS_W1];
	tapScores["W2"] = stats->m_iTapNoteScores[TNS_W2];
	tapScores["W3"] = stats->m_iTapNoteScores[TNS_W3];
	tapScores["W4"] = stats->m_iTapNoteScores[TNS_W4];
	tapScores["W5"] = stats->m_iTapNoteScores[TNS_W5];
	tapScores["miss"] = stats->m_iTapNoteScores[TNS_Miss];
	tapScores["avoidMine"] = (int)radar[RadarCategory_Mines]-stats->m_iTapNoteScores[TNS_HitMine];
	tapScores["hitMine"] = stats->m_iTapNoteScores[TNS_HitMine];
	tapScores["checkpointHit"] = stats->m_iTapNoteScores[TNS_CheckpointHit];
	tapScores["checkpointMiss"] = stats->m_iTapNoteScores[TNS_CheckpointMiss];
	root["tapNoteScores"] = tapScores;

	Json::Value holdScores;
	holdScores["held"] = stats->m_iHoldNoteScores[HNS_Held];
	holdScores["letGo"] = stats->m_iHoldNoteScores[HNS_LetGo];
	holdScores["missed"] = stats->m_iHoldNoteScores[HNS_Missed];
	root["holdNoteScores"] = holdScores;

	Json::Value stepStats;
	stepStats["tapsAndHolds"] = (int)radar[RadarCategory_TapsAndHolds];
	stepStats["jumps"] = (int)radar[RadarCategory_Jumps];
	stepStats["holds"] = (int)radar[RadarCategory_Holds];
	stepStats["mines"] = (int)radar[RadarCategory_Mines];
	stepStats["rolls"] = (int)radar[RadarCategory_Rolls];
	stepStats["lifts"] = (int)radar[RadarCategory_Lifts];
	stepStats["fakes"] = (int)radar[RadarCategory_Fakes];
	root["stepStats"] = stepStats;

	root["maxCombo"] = stats->GetMaxCombo().GetStageCnt();

	const RString fullCombo = stats->FullCombo() ? TapNoteScoreToString(stats->GetBestFullComboTapNoteScore()) : "";
	root["fullComboTier"] = fullCombo;

	auto modifiers = playerOptions->GetCurrent().GetString();
	root["modifiers"] = modifiers;
	root["disqualified"] = stats->IsDisqualified();

	return WriteJson(root);
}

void PlayDataManager::StartSong(const Song* song)
{
	auto info = SerializeSongInfo(song);
	SaveEvent(PlayEvent_SongStart, info);
}

void PlayDataManager::EndSong(const Song* song)
{
	auto info = SerializeSongInfo(song);
	SaveEvent(PlayEvent_SongEnd, info);
}

void PlayDataManager::SaveEvent(const PlayDataEvent playEvent, const RString& details)
{
	auto query = Prepare("insert into GameEvents(EventDateUtc, EventType, Details) values(?, ?, ?);");
	StatementFinalizer f(query);

	RString now = DateTime::GetNowDateTimeUtc().GetString();
	BindText(query, 0, now);

	const auto eventName = PlayDataEventToString(playEvent);
	BindText(query, 1, eventName);
	BindText(query, 2, details);
	const auto result = sqlite3_step(query);
	ASSERT(result == SQLITE_DONE);
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
