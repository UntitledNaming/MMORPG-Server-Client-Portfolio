#pragma once
#include "ContentsType.h"
#include "ContentsStruct.h"

class DBTLS;
class CGroup;
class CUser;

template<typename T>
class LFQueueMul;

enum class PostAction
{
	None,
	MoveToField,
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

struct InsertDropItemJob : public DBJob
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