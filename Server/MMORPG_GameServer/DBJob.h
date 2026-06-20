#pragma once
#include "ContentsType.h"
#include "ContentsStruct.h"

class DBTLS;
class CGroup;
class CUser;

template<typename T>
class LFQueueMul;

constexpr size_t ITEMSLOTUPDATE_SQL_FIXED = 150;
constexpr size_t ITEMSLOTUPDATE_SQL_PER_ITEM = 100;

enum class PostAction
{
	None,
	MoveToField,
};

struct ItemSwapInfo
{
	ITEM_UID  itemUID = ItemUID::ITEM_UID_INVALID_ID;
	SLOT_TYPE newSlotType;
	int16     newSlotIndex;
};

struct DBJob
{
	virtual PostAction OnComplete(CGroup* pGroup, CUser* pUser);
	virtual void Execute(DBTLS* InDBTLS) = 0;
	virtual ~DBJob() = default;

	void* operator new(size_t size);

	void  operator delete(void* ptr, size_t size);

	DB_QUERY_RESULT     result = DB_QUERY_RESULT::None;
	LFQueueMul<DBJob*>* replyTo = nullptr;                    // Job을 던진 쪽에서 소유한 DBJob 큐 포인터, Read 작업이면 이 큐로 다시 DBJob 포인터 넣음.
	uint64              sessionID = 0;                        // 이 Job을 던진 세션의 유효성 검증용 
};

struct ItemUIDRangeAllocateJob : public DBJob
{
	virtual void Execute(DBTLS* InDBTLS) override;
};

struct CharacterSelectJob : public DBJob
{
	virtual PostAction OnComplete(CGroup* pGroup, CUser* pUser) override;
	virtual void Execute(DBTLS* DBTLS) override;

	uint64   characterUID = 0;

	////////////////////////////////
	//  유저 테이블 정보
	////////////////////////////////
	uint64   accountID = 0;
	uint16   level = 0;
	int32    curEXP = 0;
	Location Location;

	////////////////////////////////
	// 아이템 테이블 정보
	////////////////////////////////
	std::vector<ItemLoadData> items;
};

struct InsertItemJob  : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;

	// 데이터 삽입 시 필요한 정보 
	ITEM_UID  itemUID;
	ITEM_ID   itemID;
	uint64    characterUID;           // 소유 캐릭터 UID
	uint16    count;
	SLOT_TYPE slotType;
	int16     slotIndex;
	ItemStat  itemStat;
};

struct DeleteItemJob : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;

	ITEM_UID itemUID;                 // 삭제할 아이템 UID
};

struct ItemCountUpdateJob : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;

	ITEM_UID itemUID;
	uint16   newCount;
};

struct ItemSlotUpdateJob : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;

	std::vector<ItemSlotUpdateData> updateitems;
};

struct CharacterProgressJob : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;

	uint64 characterUID;
	uint16 level;
	int32  curEXP;
};

struct LogOutJob : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;

	uint64                          characterUID;
	Location                        location;
	std::vector<ItemSlotUpdateData> updateitems;
};
