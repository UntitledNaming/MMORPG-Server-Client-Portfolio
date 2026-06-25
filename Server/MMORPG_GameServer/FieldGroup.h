#pragma once
#include "ContentsType.h"
#include "ContentsStruct.h"

class CUser;
class CMonster;
class SectorPos;
class CDBManager;
struct HitSearchInfo;
struct HitResult;

struct SyncInfo
{
	uint16 m_syncCount;
	uint32 m_lastSyncCheckTime;
};

struct GrossMonsterSpawnSectorArea
{
	uint16 minSectorX;
	uint16 maxSectorX;
	uint16 minSectorY;
	uint16 maxSectorY;
};

constexpr GrossMonsterSpawnSectorArea GROSS_FIELD_MONSTER_SPAWN_AREAS[FieldConst::GROSS_FIELD_AREA_ARRAY_COUNT] =
{
    { 83, 89, 52, 52 },
    { 83, 92, 53, 53 },
    { 83, 94, 54, 54 },
    { 72, 74, 55, 55 },
    { 83, 98, 55, 55 },
    { 69, 75, 56, 56 },
    { 83, 100, 56, 56 },
    { 67, 75, 57, 57 },
    { 83, 101, 57, 57 },
    { 65, 75, 58, 58 },
    { 83, 102, 58, 58 },
    { 63, 75, 59, 59 },
    { 83, 103, 59, 59 },
    { 61, 75, 60, 60 },
    { 83, 104, 60, 60 },
    { 59, 75, 61, 61 },
    { 83, 104, 61, 61 },
    { 57, 76, 62, 62 },
    { 83, 105, 62, 62 },
    { 56, 76, 63, 63 },
    { 83, 105, 63, 63 },
    { 54, 76, 64, 64 },
    { 83, 106, 64, 64 },
    { 53, 76, 65, 65 },
    { 84, 106, 65, 65 },
    { 52, 76, 66, 66 },
    { 84, 107, 66, 66 },
    { 51, 76, 67, 67 },
    { 84, 108, 67, 67 },
    { 50, 76, 68, 68 },
    { 84, 108, 68, 68 },
    { 49, 75, 69, 69 },
    { 85, 108, 69, 69 },
    { 48, 75, 70, 70 },
    { 85, 108, 70, 70 },
    { 47, 75, 71, 71 },
    { 87, 109, 71, 71 },
    { 46, 75, 72, 72 },
    { 88, 109, 72, 72 },
    { 45, 75, 73, 73 },
    { 89, 110, 73, 73 },
    { 45, 75, 74, 74 },
    { 89, 110, 74, 74 },
    { 44, 75, 75, 75 },
    { 90, 110, 75, 75 },
    { 44, 74, 76, 76 },
    { 90, 111, 76, 76 },
    { 43, 74, 77, 77 },
    { 90, 111, 77, 77 },
    { 43, 72, 78, 78 },
    { 91, 111, 78, 78 },
    { 43, 72, 79, 79 },
    { 91, 111, 79, 79 },
    { 43, 71, 80, 80 },
    { 92, 111, 80, 80 },
    { 42, 70, 81, 81 },
    { 92, 112, 81, 81 },
    { 42, 70, 82, 82 },
    { 92, 112, 82, 82 },
    { 42, 68, 83, 83 },
    { 92, 112, 83, 83 },
    { 43, 67, 84, 84 },
    { 93, 112, 84, 84 },
    { 43, 64, 85, 85 },
    { 93, 111, 85, 85 },
    { 43, 62, 86, 86 },
    { 93, 111, 86, 86 },
    { 43, 60, 87, 87 },
    { 93, 111, 87, 87 },
    { 43, 57, 88, 88 },
    { 78, 81, 88, 88 },
    { 93, 111, 88, 88 },
    { 43, 55, 89, 89 },
    { 77, 82, 89, 89 },
    { 94, 111, 89, 89 },
    { 44, 53, 90, 90 },
    { 76, 84, 90, 90 },
    { 94, 111, 90, 90 },
    { 44, 52, 91, 91 },
    { 74, 84, 91, 91 },
    { 94, 111, 91, 91 },
    { 44, 51, 92, 92 },
    { 71, 85, 92, 92 },
    { 95, 111, 92, 92 },
    { 45, 48, 93, 93 },
    { 69, 86, 93, 93 },
    { 95, 111, 93, 93 },
    { 66, 88, 94, 94 },
    { 96, 111, 94, 94 },
    { 64, 88, 95, 95 },
    { 97, 110, 95, 95 },
    { 61, 89, 96, 96 },
    { 97, 110, 96, 96 },
    { 60, 90, 97, 97 },
    { 98, 110, 97, 97 },
    { 58, 91, 98, 98 },
    { 98, 111, 98, 98 },
    { 56, 91, 99, 99 },
    { 99, 111, 99, 99 },
    { 55, 92, 100, 100 },
    { 99, 111, 100, 100 },
    { 53, 92, 101, 101 },
    { 101, 108, 101, 101 },
    { 110, 110, 101, 101 },
    { 52, 93, 102, 102 },
    { 104, 107, 102, 102 },
    { 51, 93, 103, 103 },
    { 50, 94, 104, 104 },
    { 50, 95, 105, 105 },
    { 50, 95, 106, 106 },
    { 51, 95, 107, 107 },
    { 52, 95, 108, 108 },
    { 54, 96, 109, 109 },
    { 57, 96, 110, 110 },
    { 61, 95, 111, 111 },
};


class FieldGroup : public CGroup
{
public:
	FieldGroup() = default;
	~FieldGroup() = default;

	size_t UserCount();
    void InitDBManager(CDBManager* pDBManager);

    void SendMonsterCreateToSector(CMonster* pMonster, uint16 secX, uint16 secY);
    void SendMonsterDeleteToSector(CMonster* pMonster, uint16 secX, uint16 secY);
    void SendMonsterTargetUpdate(CMonster* pMonster);
    void SendMonsterAttackTarget(CMonster* pMonster, CUser* pTarget, int16 newHP);
    void SendMonsterStop(CMonster* pMonster);
    void SendCreateFieldDropItem(FieldDropItem* pItem);
    void SendDeleteFieldDropItem(FieldDropItem* pItem);
    void AddMonsterToSector(CMonster* pMonster, uint16 secX, uint16 secY);
    void RemoveMonsterToSector(CMonster* pMonster, uint16 secX, uint16 secY);
    FieldSector& GetFieldSector(uint16 secX, uint16 secY) { return m_sectors[secY][secX]; }
    CUser* GetUser(uint64 sessionID);

private:

	//////////////////////////////////////////////////////////////////////////////////
	// 그룹 콜백 함수
	//////////////////////////////////////////////////////////////////////////////////
	virtual void  Init(CGameLibrary* p) override;
	virtual void  Destroy() override;
	virtual void  OnClientJoin(UINT64 sessionID) override;
	virtual void  OnClientLeave(UINT64 sessionID) override;
	virtual void  OnRecv(UINT64 sessionID, CMessage* pMessage) override;
	virtual void  OnIUserMove(UINT64 sessionID, IUser* pUser) override;
	virtual void  OnUpdate(long long LockEnterTime) override;

public:
	//////////////////////////////////////////////////////////////////////////////////
	// 메세지 송신 관련 함수
	//////////////////////////////////////////////////////////////////////////////////
	void SendPacket_SectorOne(BroadCastType type, CMessage* pMessage, uint16 xpos, uint16 ypos, CUser* pUser);     // 해당 섹터에 있는 유저들에게 메세지 보내기
	void SendPacket_SectorAround(BroadCastType type, CMessage* pMessage, CUser* pUser, bool userSend = false);     // 해당 섹터에 있는 유저들에게 메세지 보내기
    void SendPacket_HitSectors(HitResult& result);

private:
	///////////////////////////////////
    // Degree 변환 함수              //
    ///////////////////////////////////
	constexpr float DegreeToRadian(float degree)
	{
		return degree * FieldConst::Pi / 180.0f;
	}


	///////////////////////////////////
    // 공격 관련 함수                //
    ///////////////////////////////////
	void   CollectHitTarget(CUser* attacker, HitSearchInfo& hitInfo, HitResult& hitResult);

	///////////////////////////////////
    // 클라이언트 메세지 처리 핸들러 //
    ///////////////////////////////////
	void HandleCharacterMovementUpdate(uint64 sessionID, CMessage* pMessage);
	void HandleRTTMessage(uint64 sessionID, CMessage* pMessage);
	void HandleLeftAttackSwing(uint64 sessionID, CMessage* pMessage);
	void HandleSkillUse(uint64 sessionID, CMessage* pMessage);
	void HandlePickUpItems(uint64 sessionID, CMessage* pMessage);
	void HandleUseItem(uint64 sessionID, CMessage* pMessage);
	void HandleDeleteItem(uint64 sessionID, CMessage* pMessage);
	void HandleSwapSlot(uint64 sessionID, CMessage* pMessage);
	void HandleRespawn(uint64 sessionID, CMessage* pMessage);

	///////////////////////////////////
    // 프레임 로직 처리 함수         //
    ///////////////////////////////////
	void UserUpdate();
	void SectorUpdate(CUser* pUser, const SectorPos& newSec);
    void MonsterUpdate();
    void FieldDropItemExpired();

	//////////////////////////////////////////////////////////////////////////////////
    // 몬스터 관련 함수
    //////////////////////////////////////////////////////////////////////////////////
	void MonsterSpawnInit();
	void GrossMonsterSpawnInit();

    //////////////////////////////////////////////////////////////////////////////////
    // 아이템 관련 함수
    //////////////////////////////////////////////////////////////////////////////////
    void CreateFieldDropItem(CMonster& monster);

private:
	std::unordered_map<uint64, CUser*>            m_userLookUpTable;
	std::unordered_map<uint64, FieldDropItem*>    m_dropItemLookUpTable;
	FieldSector                                   m_sectors[FieldConst::SECTOR_Y_MAX][FieldConst::SECTOR_X_MAX];
	CMonster                                      m_grossMonsterPoolArray[FieldConst::MAX_GROSS_FIELD_MONSTER_COUNT];
    CDBManager*                                   m_DBManagerPtr = nullptr;
	uint64                                        m_monsterAllocID   = 0;

public:
	uint64 syncCount = 0;

    // 수신 메세지 쪽 처리 시간 측정 저장 변수
    LatencyHistogram m_OnRecvLockEnterTime;
    LatencyHistogram m_OnRecvMsgProcTime[(int)FieldRecvType::Max];     // 수신 메세지 프로토콜 별 처리 시간 담는 배열


    // 프레임쪽 처리 시간 측정 저장 변수
    LatencyHistogram m_OnUpdateLockEnterTime;
    LatencyHistogram m_OnUpdateProcTime[(int)FrameProcType::Max];

    // 브로드 캐스팅 하는 쪽 처리 시간 측정 변수
    LatencyHistogram m_BroadCastProcTime[(int)BroadCastType::Max];
};

