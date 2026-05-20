#pragma once
#include "ContentsType.h"

class CUser;
class CMonster;
class SectorPos;
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

class FieldGroup : public CGroup
{
public:
	FieldGroup() = default;
	~FieldGroup() = default;

	size_t UserCount();

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
	virtual void  OnUpdate() override;

	//////////////////////////////////////////////////////////////////////////////////
	// 섹터 관련 함수
	//////////////////////////////////////////////////////////////////////////////////
	void SendPacket_SectorOne(CMessage* pMessage, uint16 xpos, uint16 ypos, CUser* pUser);     // 해당 섹터에 있는 유저들에게 메세지 보내기
	void SendPacket_SectorAround(CMessage* pMessage, CUser* pUser, bool userSend = false);     // 해당 섹터에 있는 유저들에게 메세지 보내기

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
	void HandleLeftAttackStop(uint64 sessionID, CMessage* pMessage);
	void HandleSkillUse(uint64 sessionID, CMessage* pMessage);

	///////////////////////////////////
    // 프레임 로직 처리 함수         //
    ///////////////////////////////////
	void MovementProc();
	void SectorUpdate(CUser* pUser, const SectorPos& newSec);
	void UserManaRegen();


	//////////////////////////////////////////////////////////////////////////////////
    // 몬스터 관련 함수
    //////////////////////////////////////////////////////////////////////////////////
	void MonsterSpawnInit();
	void GrossMonsterSpawnInit();
private:
	std::unordered_map<uint64, CUser*>            m_userLookUpTable;
	FieldSector                                   m_sectors[FieldConst::SECTOR_Y_MAX][FieldConst::SECTOR_X_MAX];
	CMonster                                      m_grossMonsterPoolArray[FieldConst::MAX_GROSS_FIELD_MONSTER_COUNT];
									              
	GrossMonsterSpawnSectorArea                   m_grossFieldSpawnArea[3] = { {81,110,51,100}, {42,75,54,92}, {49,95,86,109} };

	uint32                                        m_ManaRegenOldTime = 0;
	uint64                                        m_monsterAllocID   = 0;

public:
	uint64 fieldframe = 0;
	uint64 syncCount = 0;
};

