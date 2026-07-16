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

	// 아이템 저장할 공간 확보
	items.reserve(UserQuickSlot::QUICK_SLOT_MAX + UserInventory::INVENTORY_SLOT_MAX + (int)EQUIP_SLOT::MAX);

	// 스택 아이템 테이블 정보 가져오기
	success = InDBTLS->DB_Post_Query(result, "SELECT * FROM worlddb.stackitem WHERE characterUID = %llu", characterUID);
	if (!success)
		__debugbreak();

	mysql_res = InDBTLS->DB_GET_Result(0);
	if (mysql_res == nullptr)
		__debugbreak();

	row = InDBTLS->DB_Fetch_Row(mysql_res);
	while (row != nullptr)
	{
		ItemLoadData item = {};
		item.itemUID = (ITEM_UID)strtoull((*row)[0], nullptr, 10);
		item.itemID = (ITEM_ID)atoi((*row)[2]);
		item.slotType = (SLOT_TYPE)atoi((*row)[3]);
		item.slotIndex = (int16)atoi((*row)[4]);
		item.count = (uint16)atoi((*row)[5]);

		items.push_back(item);
		row = InDBTLS->DB_Fetch_Row(mysql_res);
	}

	// 데이터 가져온거 밀어버리기
	InDBTLS->DB_Free_Result();


	// 인스턴스 아이템 테이블 정보 가져오기(itemUID로 오름차순 정렬하여 100, 101, 100 UID일 때 뒤 100 UID의 장비 스탯이 저장 제대로 안되는 상황 막기)
	success = InDBTLS->DB_Post_Query(result, "SELECT instanceitem.itemUID, instanceitem.itemID, instanceitem.slottype, instanceitem.slotindex, instanceitem_stat.randomStatType, instanceitem_stat.statValue FROM worlddb.instanceitem LEFT JOIN worlddb.instanceitem_stat ON instanceitem_stat.itemUID = instanceitem.itemUID WHERE characterUID = %llu ORDER BY instanceitem.itemUID ", characterUID);
	if (!success)
		__debugbreak();

	mysql_res = InDBTLS->DB_GET_Result(0);
	if (mysql_res == nullptr)
		__debugbreak();

	ITEM_UID lastUID = ItemUID::ITEM_UID_INVALID_ID;

	row = InDBTLS->DB_Fetch_Row(mysql_res);
	while (row != nullptr)
	{
		ItemLoadData item = {};
		item.itemUID = (ITEM_UID)strtoull((*row)[0], nullptr, 10); //10 진법으로 8바이트 uid 추출
		item.itemID = (ITEM_ID)atoi((*row)[1]);
		item.slotType = (SLOT_TYPE)atoi((*row)[2]);
		item.slotIndex = (int16)atoi((*row)[3]);
		item.count = 1;

		if ((*row)[4] != nullptr)
		{
			if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::ATK)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::DEF)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::MAX_HP)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::MAX_MP)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::HP_REGEN)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::MP_REGEN)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}
		}

		lastUID = item.itemUID;

		// 다음 row가 없거나 다음 row의 UID가 다르거나 stat 테이블에 스탯 없는 장비 나오면 (*row)[4]가 nullptr일때 루프 탈출 후 벡터에 넣기
		while (1)
		{
			row = InDBTLS->DB_Fetch_Row(mysql_res);

			if (row == nullptr || lastUID != (ITEM_UID)strtoull((*row)[0], nullptr, 10) || (*row)[4] == nullptr)
				break;

			// 아이템 UID가 이전과 같으면 랜덤 스탯 타입쪽만 확인해서 넣기
			if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::ATK)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::DEF)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::MAX_HP)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::MAX_MP)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::HP_REGEN)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

			else if ((RANDOM_STAT_TYPE)atoi((*row)[4]) == RANDOM_STAT_TYPE::MP_REGEN)
			{
				item.randomStat[item.randomStatCount].randomStatType = (RANDOM_STAT_TYPE)atoi((*row)[4]);
				item.randomStat[item.randomStatCount].randomStatValue = (int16)atoi((*row)[5]);
				item.randomStatCount++;
			}

		}

		items.push_back(item);
	}

	// 데이터 가져온거 밀어버리기
	InDBTLS->DB_Free_Result();


	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::CharacterSelect].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());

}

void InsertItemJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;

	if (itemType == ITEM_TYPE::CONSUMABLE)
	{
		success = InDBTLS->DB_Post_Query(result, "INSERT INTO worlddb.stackitem VALUES (%llu, %llu, %u, %d, %d, %d)",
			itemUID, characterUID, itemID, slotType, slotIndex, count);
		if (!success)
			__debugbreak();
	}
	else if (itemType == ITEM_TYPE::EQUIPMENT)
	{
		success = InDBTLS->DB_Post_Query(result, "INSERT INTO worlddb.instanceitem VALUES (%llu, %llu, %u, %d, %d)",
			itemUID, characterUID, itemID, slotType, slotIndex);
		if (!success)
			__debugbreak();

		if (randomStatCount != 0)
		{
			std::string query;
			query.reserve(60 + (int)RANDOM_STAT_TYPE::MAX * (sizeof(int16) + sizeof(RANDOM_STAT_TYPE)));

			query = "INSERT INTO worlddb.instanceitem_stat VALUES ";

			// 랜덤 스탯 갯수에 따라 stat 테이블에 넣을 쿼리 문자열 담을 버퍼 생성
			for(int i = 0; i < randomStatCount; i++)
			{
				char buffer[250] = {};

				if (i < randomStatCount - 1)
				{
					int n1 = snprintf(buffer, sizeof(buffer), "(%llu, %d, %d), ", itemUID, randomStat[i].randomStatType, randomStat[i].randomStatValue);
				}
				else
				{
					int n1 = snprintf(buffer, sizeof(buffer), "(%llu, %d, %d)", itemUID, randomStat[i].randomStatType, randomStat[i].randomStatValue);
				}

				query.append(buffer);
			}

			success = InDBTLS->DB_Post_Query(result, "%s", query.c_str());
			if (!success)
				__debugbreak();
		}
	}

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::InsertItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void DeleteItemJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	if (itemType == ITEM_TYPE::CONSUMABLE)
	{
		success = InDBTLS->DB_Post_Query(result, "DELETE FROM worlddb.stackitem WHERE itemUID = %llu", itemUID);
		if (!success)
			__debugbreak();
	}
	else if (itemType == ITEM_TYPE::EQUIPMENT)
	{
		success = InDBTLS->DB_Post_Query(result, "DELETE FROM worlddb.instanceitem WHERE itemUID = %llu", itemUID);
		if (!success)
			__debugbreak();

		success = InDBTLS->DB_Post_Query(result, "DELETE FROM worlddb.instanceitem_stat WHERE itemUID = %llu", itemUID);
		if (!success)
			__debugbreak();
	}


	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::DeleteItem].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void ItemCountUpdateJob::Execute(DBTLS* InDBTLS)
{
	auto start = std::chrono::steady_clock::now();

	bool success = false;
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.stackitem SET count = %u WHERE itemUID = %llu", newCount, itemUID);

	if (!success)
		__debugbreak();

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::ItemUpdateCount].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}

void ItemSlotUpdateJob::Execute(DBTLS* InDBTLS)
{
	// 쿼리 문자열 생성 스택따로 인스턴스 따로 생성

	// 스택 쿼리 문자열 생성
	std::string stackQuery;

	if (!updatestackitems.empty())
	{
		stackQuery.reserve(ITEMSLOTUPDATE_SQL_FIXED + updatestackitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);

		stackQuery = "UPDATE worlddb.stackitem SET ";

		std::string slottypeStackQuery;
		std::string slotindexStackQuery;
		std::string whereStackQuery;

		slottypeStackQuery.reserve(updatestackitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		slotindexStackQuery.reserve(updatestackitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		whereStackQuery.reserve(updatestackitems.size() * 20);


		slottypeStackQuery = "slottype = CASE itemUID ";
		slotindexStackQuery = "slotindex = CASE itemUID ";
		whereStackQuery = "WHERE itemUID IN (";

		for (size_t i = 0; i < updatestackitems.size(); i++)
		{
			char type[50] = {};
			char index[50] = {};
			char uid[50] = {};

			if (i != updatestackitems.size() - 1)
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u ", updatestackitems[i].itemUID, updatestackitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d ", updatestackitems[i].itemUID, updatestackitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu, ", updatestackitems[i].itemUID);

			}
			else
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u END, ", updatestackitems[i].itemUID, updatestackitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d END ", updatestackitems[i].itemUID, updatestackitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu)", updatestackitems[i].itemUID);
			}

			slottypeStackQuery.append(type);
			slotindexStackQuery.append(index);
			whereStackQuery.append(uid);
		}

		// 스택 쿼리 완성
		stackQuery += slottypeStackQuery;
		stackQuery += slotindexStackQuery;
		stackQuery += whereStackQuery;

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 인스턴스 아이템 쿼리 생성
	std::string instanceQuery;

	if (!updateinstanceitems.empty())
	{
		instanceQuery.reserve(ITEMSLOTUPDATE_SQL_FIXED + updateinstanceitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);

		instanceQuery = "UPDATE worlddb.instanceitem SET ";

		std::string slottypeInstanceQuery;
		std::string slotindexInstanceQuery;
		std::string whereInstanceQuery;

		slottypeInstanceQuery.reserve(updateinstanceitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		slotindexInstanceQuery.reserve(updateinstanceitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		whereInstanceQuery.reserve(updateinstanceitems.size() * 20);


		slottypeInstanceQuery = "slottype = CASE itemUID ";
		slotindexInstanceQuery = "slotindex = CASE itemUID ";
		whereInstanceQuery = "WHERE itemUID IN (";

		for (size_t i = 0; i < updateinstanceitems.size(); i++)
		{
			char type[50] = {};
			char index[50] = {};
			char uid[50] = {};

			if (i != updateinstanceitems.size() - 1)
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu, ", updateinstanceitems[i].itemUID);

			}
			else
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u END, ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d END ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu)", updateinstanceitems[i].itemUID);
			}

			slottypeInstanceQuery.append(type);
			slotindexInstanceQuery.append(index);
			whereInstanceQuery.append(uid);
		}

		// 인스턴스 쿼리 완성
		instanceQuery += slottypeInstanceQuery;
		instanceQuery += slotindexInstanceQuery;
		instanceQuery += whereInstanceQuery;
	}

	auto start = std::chrono::steady_clock::now();

	// 아이템 UPDATE 쿼리 한방에 보내기
	bool success = false;

	if (!updatestackitems.empty())
	{
		success = InDBTLS->DB_Post_Query(result, "%s", stackQuery.c_str());
		if (!success)
			__debugbreak();
	}
	
	if (!updateinstanceitems.empty())
	{
		success = InDBTLS->DB_Post_Query(result, "%s", instanceQuery.c_str());
		if (!success)
			__debugbreak();
	}

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
	// 쿼리 문자열 생성 스택따로 인스턴스 따로 생성

    // 스택 쿼리 문자열 생성
	std::string stackQuery;

	if (!updatestackitems.empty())
	{
		stackQuery.reserve(ITEMSLOTUPDATE_SQL_FIXED + updatestackitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);

		stackQuery = "UPDATE worlddb.stackitem SET ";

		std::string slottypeStackQuery;
		std::string slotindexStackQuery;
		std::string whereStackQuery;

		slottypeStackQuery.reserve(updatestackitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		slotindexStackQuery.reserve(updatestackitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		whereStackQuery.reserve(updatestackitems.size() * 20);


		slottypeStackQuery = "slottype = CASE itemUID ";
		slotindexStackQuery = "slotindex = CASE itemUID ";
		whereStackQuery = "WHERE itemUID IN (";

		for (size_t i = 0; i < updatestackitems.size(); i++)
		{
			char type[50] = {};
			char index[50] = {};
			char uid[50] = {};

			if (i != updatestackitems.size() - 1)
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u ", updatestackitems[i].itemUID, updatestackitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d ", updatestackitems[i].itemUID, updatestackitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu, ", updatestackitems[i].itemUID);

			}
			else
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u END, ", updatestackitems[i].itemUID, updatestackitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d END ", updatestackitems[i].itemUID, updatestackitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu)", updatestackitems[i].itemUID);
			}

			slottypeStackQuery.append(type);
			slotindexStackQuery.append(index);
			whereStackQuery.append(uid);
		}

		// 스택 쿼리 완성
		stackQuery += slottypeStackQuery;
		stackQuery += slotindexStackQuery;
		stackQuery += whereStackQuery;

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 인스턴스 아이템 쿼리 생성
	std::string instanceQuery;

	if (!updateinstanceitems.empty())
	{
		instanceQuery.reserve(ITEMSLOTUPDATE_SQL_FIXED + updateinstanceitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);

		instanceQuery = "UPDATE worlddb.instanceitem SET ";

		std::string slottypeInstanceQuery;
		std::string slotindexInstanceQuery;
		std::string whereInstanceQuery;

		slottypeInstanceQuery.reserve(updateinstanceitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		slotindexInstanceQuery.reserve(updateinstanceitems.size() * ITEMSLOTUPDATE_SQL_PER_ITEM);
		whereInstanceQuery.reserve(updateinstanceitems.size() * 20);


		slottypeInstanceQuery = "slottype = CASE itemUID ";
		slotindexInstanceQuery = "slotindex = CASE itemUID ";
		whereInstanceQuery = "WHERE itemUID IN (";

		for (size_t i = 0; i < updateinstanceitems.size(); i++)
		{
			char type[50] = {};
			char index[50] = {};
			char uid[50] = {};

			if (i != updateinstanceitems.size() - 1)
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu, ", updateinstanceitems[i].itemUID);

			}
			else
			{
				int n1 = snprintf(type, sizeof(type), "WHEN %llu THEN %u END, ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotType);
				int n2 = snprintf(index, sizeof(index), "WHEN %llu THEN %d END ", updateinstanceitems[i].itemUID, updateinstanceitems[i].slotIndex);
				int n3 = snprintf(uid, sizeof(uid), "%llu)", updateinstanceitems[i].itemUID);
			}

			slottypeInstanceQuery.append(type);
			slotindexInstanceQuery.append(index);
			whereInstanceQuery.append(uid);
		}

		// 인스턴스 쿼리 완성
		instanceQuery += slottypeInstanceQuery;
		instanceQuery += slotindexInstanceQuery;
		instanceQuery += whereInstanceQuery;
	}

	/////////////////////////////////////////////////////////////////////////////////
	// 쿼리 보내기
	/////////////////////////////////////////////////////////////////////////////////
	auto start = std::chrono::steady_clock::now();

	bool success = false;

	// 캐릭터 위치 저장
	success = InDBTLS->DB_Post_Query(result, "UPDATE worlddb.character SET xpos = %f , ypos = %f, zpos = %f, characterlevel = %u, curEXP = %d WHERE characterUID = %llu", location.xpos, location.ypos, location.zpos, level, curEXP, characterUID);
	if (!success)
		__debugbreak();

	// 아이템 저장(바뀐거 있을 때)
	if (!updatestackitems.empty())
	{
		success = InDBTLS->DB_Post_Query(result, "%s", stackQuery.c_str());
		if (!success)
			__debugbreak();
	}

	if (!updateinstanceitems.empty())
	{
		success = InDBTLS->DB_Post_Query(result, "%s", instanceQuery.c_str());
		if (!success)
			__debugbreak();
	}

	auto end = std::chrono::steady_clock::now();

	g_QueryProcTime[(int)DBJobCount::LogOut].Record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}