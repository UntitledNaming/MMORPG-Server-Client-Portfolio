#include <mysql.h>
#include <string>
#include <unordered_map>
#include <array>
#include <set>
#include <vector>
#include <chrono>
#include "LFQMultiLive.h"
#include "CSizeClassMemoryPoolTLS.h"
#include "DBTLS.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "IUser.h"
#include "CUser.h"
#include "ItemUIDAllocator.h"
#include "DBJob.h"

unsigned long long  DBJob::g_TPS[(int)DBJobCount::Max] = {};
LatencyHistogram    DBJob::g_QueryProcTime[(int)DBJobCount::Max] = {};


void* DBJob::operator new(size_t size)
{
	return CSizeClassMemoryPoolTLS::Alloc(size);
}

void  DBJob::operator delete(void* ptr, size_t size)
{
	CSizeClassMemoryPoolTLS::Free(ptr, size);
}

PostAction DBJob::OnComplete(CGroup* pGroup, CUser* pUser)
{
	return PostAction::None;
}

void ItemUIDRangeAllocateJob::Execute(DBTLS* InDBTLS)
{
	bool success = false;
	ITEM_UID startUID = 0;

	success = InDBTLS->DB_Post_Query(result, "START TRANSACTION");
	if (!success)
		__debugbreak();

	// UID 할당 테이블에서 가져오기
	success = InDBTLS->DB_Post_Query(result, "SELECT startUID FROM worlddb.uid_sequence WHERE uidName = '%s' FOR UPDATE", "ItemUIDAllocator");
	if (!success)
		__debugbreak();

	// 쿼리 날리고 STORE_RESULT로 처리하기 
	MYSQL_RES* mysql_res = InDBTLS->DB_GET_Result(0);
	if (mysql_res == nullptr)
		__debugbreak();

	// 가져온 데이터에 대한 Row 데이터 가리키는 포인터 얻기
	MYSQL_ROW* row = InDBTLS->DB_Fetch_Row(mysql_res);
	if (row == nullptr)
		__debugbreak();

	startUID = (ITEM_UID)atoi((*row)[0]);

	// 데이터 가져온거 밀어버리기
	InDBTLS->DB_Free_Result();

	// StartUID 갱신
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.uid_sequence SET startUID = %llu WHERE uidName = '%s'", startUID+ItemUID::ITEM_UID_RESERVE_COUNT,"ItemUIDAllocator");
	if (!success)
		__debugbreak();

	// 커밋 끝
	success = InDBTLS->DB_Post_Query(result, "COMMIT");
	if (!success)
		__debugbreak();

	// 전달하기
	ItemUIDAllocator::Init(startUID, startUID + ItemUID::ITEM_UID_RESERVE_COUNT);
}

PostAction CharacterSelectJob::OnComplete(CGroup* pGroup, CUser* pUser)
{
	// 매개인자로 받은 pUser에 Job 멤버에 있는 유저 정보를 옮겨서 초기화 하기
	pUser->LoadDataFromDB(characterUID, accountID, level, curEXP, Location, items);

	return PostAction::MoveToField;
}

void CharacterSelectJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	// DBTLS를 통해 유저 객체 및 아이템 정보 얻는 쿼리 날리기
	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "START TRANSACTION");
	if (!success)
		__debugbreak();

	// 유저 테이블 정보 가져오기
	success = InDBTLS->DB_Post_Query(result, "SELECT * FROM worlddb.character WHERE characterUID = %llu", characterUID);
	if (!success)
		__debugbreak();

	// 쿼리 날리고 STORE_RESULT로 처리하기 
	MYSQL_RES* mysql_res = InDBTLS->DB_GET_Result(0);
	if (mysql_res == nullptr)
		__debugbreak();

	// 가져온 데이터에 대한 Row 데이터 가리키는 포인터 얻기
	MYSQL_ROW* row = InDBTLS->DB_Fetch_Row(mysql_res);
	if (row == nullptr)
		__debugbreak();

	level = (uint16)atoi((*row)[2]);
	curEXP = (int32)atoi((*row)[3]);
	Location.xpos = (float)strtof((*row)[4], nullptr);
	Location.ypos = (float)strtof((*row)[5], nullptr);
	Location.zpos = (float)strtof((*row)[6], nullptr);

	// 데이터 가져온거 밀어버리기
	InDBTLS->DB_Free_Result();

	// 아이템 테이블 정보 가져오기
	success = InDBTLS->DB_Post_Query(result, "SELECT * FROM worlddb.item WHERE characterUID = %llu", characterUID);
	if (!success)
		__debugbreak();

	mysql_res = InDBTLS->DB_GET_Result(0);
	if (mysql_res == nullptr)
		__debugbreak();
	size_t cnt = mysql_num_rows(mysql_res); // row 갯수 알려줌
	items.reserve(cnt);

	row = InDBTLS->DB_Fetch_Row(mysql_res);
	while (row != nullptr)
	{
		ItemLoadData item = {};
		item.itemUID = (ITEM_UID)atoi((*row)[0]);
		item.itemID = (ITEM_ID)atoi((*row)[2]);
		item.count = (uint16)atoi((*row)[3]);
		item.slotType = (SLOT_TYPE)atoi((*row)[4]);
		item.slotIndex = (int16)atoi((*row)[5]);

		int16 atk = (int16)atoi((*row)[6]);
		if (atk != 0)
		{
			item.randomStat[item.randomStatCount].randomStatType = RANDOM_STAT_TYPE::ATK;
			item.randomStat[item.randomStatCount].randomStatValue = atk;
			item.randomStatCount++;
		}

		int16 def = (int16)atoi((*row)[7]);
		if (def != 0)
		{
			item.randomStat[item.randomStatCount].randomStatType = RANDOM_STAT_TYPE::DEF;
			item.randomStat[item.randomStatCount].randomStatValue = def;
			item.randomStatCount++;
		}

		int16 maxhp = (int16)atoi((*row)[8]);
		if (maxhp != 0)
		{
			item.randomStat[item.randomStatCount].randomStatType = RANDOM_STAT_TYPE::MAX_HP;
			item.randomStat[item.randomStatCount].randomStatValue = maxhp;
			item.randomStatCount++;
		}

		int16 maxmp = (int16)atoi((*row)[9]);
		if (maxmp != 0)
		{
			item.randomStat[item.randomStatCount].randomStatType = RANDOM_STAT_TYPE::MAX_MP;
			item.randomStat[item.randomStatCount].randomStatValue = maxmp;
			item.randomStatCount++;
		}

		int16 hpregen = (int16)atoi((*row)[10]);
		if (hpregen != 0)
		{
			item.randomStat[item.randomStatCount].randomStatType = RANDOM_STAT_TYPE::HP_REGEN;
			item.randomStat[item.randomStatCount].randomStatValue = hpregen;
			item.randomStatCount++;
		}

		int16 mpregen = (int16)atoi((*row)[11]);
		if (mpregen != 0)
		{
			item.randomStat[item.randomStatCount].randomStatType = RANDOM_STAT_TYPE::MP_REGEN;
			item.randomStat[item.randomStatCount].randomStatValue = mpregen;
			item.randomStatCount++;
		}

		items.push_back(item);
		row = InDBTLS->DB_Fetch_Row(mysql_res);
	}

	// 데이터 가져온거 밀어버리기
	InDBTLS->DB_Free_Result();

	// 커밋 끝
	success = InDBTLS->DB_Post_Query(result, "COMMIT");
	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::CharacterSelect].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());

}

void InsertItemJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "INSERT INTO worlddb.item VALUES (%llu, %llu, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d)", 
		itemUID, characterUID, itemID, count, slotType, slotIndex, itemStat.atk, itemStat.def, itemStat.maxHP, itemStat.maxMP, itemStat.hpRegenPerSec, itemStat.mpRegenPerSec);
	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::InsertItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void DeleteItemJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "DELETE FROM worlddb.item WHERE itemUID = %llu", itemUID);
		
	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::DeleteItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void ItemCountUpdateJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.item SET count = %u WHERE itemUID = %llu", newCount, itemUID);

	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::ItemUpdateCount].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void ItemSlotUpdateJob::Execute(DBTLS* InDBTLS)
{
	// 쿼리 문자열 생성
	std::string lastquery;
	lastquery.reserve(ITEMSLOTUPDATE_SQL_FIXED + updateitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);     // INSERT INTO  ... VALUES(62) + ON DUPLICATE KEY UPDATE ... (78) = 160 + 아이템 1개당 문자열 크기(40) * 갯수 

	lastquery = "UPDATE worlddb.item SET ";

	std::string slottypeQuery;
	std::string slotindexQuery;
	std::string whereQuery;

	slottypeQuery.reserve(updateitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
	slotindexQuery.reserve(updateitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
	whereQuery.reserve(updateitems.size() * 20);


	slottypeQuery = "slottype = CASE itemUID " ;
	slotindexQuery = "slotindex = CASE itemUID ";
	whereQuery = "WHERE itemUID IN (";

	for (size_t i = 0; i < updateitems.size(); i++)
	{
		char type[50] = {};
		char index[50] = {};
		char uid[50] = {};

		if (i != updateitems.size() - 1)
		{
			int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u ", updateitems[i].itemUID, updateitems[i].slotType);
			int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d ", updateitems[i].itemUID, updateitems[i].slotIndex);
			int n3 = snprintf(uid, sizeof(uid), "%llu, ", updateitems[i].itemUID);

		}
		else
		{
			int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u END, ", updateitems[i].itemUID, updateitems[i].slotType);
			int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d END ", updateitems[i].itemUID, updateitems[i].slotIndex);
			int n3 = snprintf(uid, sizeof(uid), "%llu)", updateitems[i].itemUID);
		}

		slottypeQuery.append(type);
		slotindexQuery.append(index);
		whereQuery.append(uid);
	}

	lastquery += slottypeQuery;
	lastquery += slotindexQuery;
	lastquery += whereQuery;


	auto start = std::chrono::steady_clock::now();

	// 아이템 UPDATE 쿼리 한방에 보내기
	bool success = false;
	success = InDBTLS->DB_Post_Query(result, lastquery.c_str());
	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::ItemSlotUpdate].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void CharacterProgressJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.character SET characterlevel = %u, curEXP = %d WHERE characterUID = %llu", level, curEXP, characterUID);

	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::CharacterProgress].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void LogOutJob::Execute(DBTLS* InDBTLS)
{
	// 쿼리 문자열 생성
	std::string lastquery;
	lastquery.reserve(ITEMSLOTUPDATE_SQL_FIXED + updateitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);     // INSERT INTO  ... VALUES(62) + ON DUPLICATE KEY UPDATE ... (78) = 160 + 아이템 1개당 문자열 크기(40) * 갯수 

	lastquery = "UPDATE worlddb.item SET ";

	std::string slottypeQuery;
	std::string slotindexQuery;
	std::string whereQuery;

	slottypeQuery.reserve(updateitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
	slotindexQuery.reserve(updateitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
	whereQuery.reserve(updateitems.size() * 20);


	slottypeQuery = "slottype = CASE itemUID ";
	slotindexQuery = "slotindex = CASE itemUID ";
	whereQuery = "WHERE itemUID IN (";

	for (size_t i = 0; i < updateitems.size(); i++)
	{
		char type[50] = {};
		char index[50] = {};
		char uid[50] = {};

		if (i != updateitems.size() - 1)
		{
			int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u ", updateitems[i].itemUID, updateitems[i].slotType);
			int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d ", updateitems[i].itemUID, updateitems[i].slotIndex);
			int n3 = snprintf(uid, sizeof(uid), "%llu, ", updateitems[i].itemUID);

		}
		else
		{
			int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u END, ", updateitems[i].itemUID, updateitems[i].slotType);
			int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d END ", updateitems[i].itemUID, updateitems[i].slotIndex);
			int n3 = snprintf(uid, sizeof(uid), "%llu)", updateitems[i].itemUID);
		}

		slottypeQuery.append(type);
		slotindexQuery.append(index);
		whereQuery.append(uid);
	}

	lastquery += slottypeQuery;
	lastquery += slotindexQuery;
	lastquery += whereQuery;

	/////////////////////////////////////////////////////////////////////////////////
	// 쿼리 보내기
	/////////////////////////////////////////////////////////////////////////////////
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "START TRANSACTION");
	if (!success)
		__debugbreak();

	// 캐릭터 위치 저장
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.character SET xpos = %f , ypos = %f, zpos = %f WHERE characterUID = %llu", location.xpos, location.ypos, location.zpos, characterUID);
	if (!success)
		__debugbreak();

	// 아이템 저장(바뀐거 있을 때)
	if (updateitems.size() != 0)
	{
		success = InDBTLS->DB_Post_Query(result, lastquery.c_str());
		if (!success)
			__debugbreak();
	}

	// 커밋 끝
	success = InDBTLS->DB_Post_Query(result, "COMMIT");
	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::LogOut].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}