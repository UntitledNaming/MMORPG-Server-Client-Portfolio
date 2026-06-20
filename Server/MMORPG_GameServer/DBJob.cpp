#include <mysql.h>
#include <string>
#include <unordered_map>
#include <array>
#include <set>
#include <vector>
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
}

void InsertItemJob::Execute(DBTLS* InDBTLS)
{
	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "INSERT INTO worlddb.item VALUES (%llu, %llu, %u, %d, %d, %d, %d, %d, %d, %d, %d, %d)", 
		itemUID, characterUID, itemID, count, slotType, slotIndex, itemStat.atk, itemStat.def, itemStat.maxHP, itemStat.maxMP, itemStat.hpRegenPerSec, itemStat.mpRegenPerSec);
	if (!success)
		__debugbreak();
}

void DeleteItemJob::Execute(DBTLS* InDBTLS)
{
	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "DELETE FROM worlddb.item WHERE itemUID = %llu", itemUID);
		
	if (!success)
		__debugbreak();
}

void ItemCountUpdateJob::Execute(DBTLS* InDBTLS)
{
	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.item SET count = %u WHERE itemUID = %llu", newCount, itemUID);

	if (!success)
		__debugbreak();
}

void CharacterProgressJob::Execute(DBTLS* InDBTLS)
{
	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "START TRANSACTION");
	if (!success)
		__debugbreak();

	
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.item SET slottype = %u, slotindex = %d WHERE characterUID = %llu", level, curEXP, characterUID);

	if (!success)
		__debugbreak();

	// 커밋 끝
	success = InDBTLS->DB_Post_Query(result, "COMMIT");
	if (!success)
		__debugbreak();
}

void LogOutJob::Execute(DBTLS* InDBTLS)
{
	bool success = false;
	//success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.item SET count = %u WHERE itemUID = %llu", newCount, uid);

	if (!success)
		__debugbreak();
}