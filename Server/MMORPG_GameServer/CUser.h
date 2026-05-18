#pragma once
#include "ContentsType.h"
#include "SectorPos.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"

class CUser : public IUser
{
public:
	CUser() = default;
	~CUser() = default;
	
	void   Init(uint64 sessionID);
	void   ManaRegen(uint32 curTime);
	void   Damage(uint16 damage);
	void   UseSkill(uint32 curTime, uint8 skillIndex);
	void   SectorFind(SectorAround& pAround);
	void   SetNewSectorPos(const SectorPos& newSec) { m_secPos = newSec; }
	void   CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector);
	void   SwingStop() { m_swingInfo.m_lastSwingIdx = 0; }
	bool   CanUseSkill(uint32 curTime, uint8 skillIndex);
	bool   Move();
	bool   CanSwing(uint32 curTime, uint8 swingidx);
	uint64 GetSessionID() const { return m_sessionID; }
	uint32 CalSkillDamage(uint16 skillIndex, CUser* target, uint32 curTime);
	uint32 CalBaseAttackDamage(CUser* target, uint32 curTime);
	uint16 GetDef(uint32 curTime);
	uint16 GetAtk(uint32 curTime);
	uint16 GetMaxHP(uint32 curTime);
	uint16 GetMaxMP(uint32 curTime);
	uint16 GetHP() const { return m_hp; }
	uint16 GetMP() const { return m_mp; }
	uint16 GetMPRegenSec() const{ return m_mpRegenPerSec; }
	uint16 GetSectorArrayIdx() const { return m_arrayIdx; }
	uint16 GetSectorXpos() const { return m_secPos.GetX(); }
	uint16 GetSectorYpos() const { return m_secPos.GetY(); }
	uint8  GetLastSwingIndex() const { return m_swingInfo.m_lastSwingIdx; }
	float  GetX() const { return m_location.xpos; }
	float  GetY() const { return m_location.ypos; }
	float  GetZ() const { return m_location.zpos; }
	float  GetMoveYaw() const { return m_movementYaw; }
	bool   GetMoveFlag() const { return m_moveFlag; }

	const SectorPos& GetSectorPos() const { return m_secPos; }
	const Location& GetLocation() const { return m_location; }
	void SetLocation(Location& location) { m_location = location; }
	void SetMoveYaw(float moveYaw) { m_movementYaw = moveYaw; }
	void SetSectorArrayIdx(uint16 idx) { m_arrayIdx = idx; }
	void SetMoveFlag(bool flag) { m_moveFlag = flag; }

	static CUser* Alloc();
	static void Free(CUser* pUser);

public:
	uint16              m_syncCount;                                                       // 특정 시간동안 싱크 발생한 횟수
	uint32              m_recvTime;                                                        // 메세지 마지막 수신 시간
	uint32              m_lastSyncCheckTime;                                               // 마지막 싱크 패킷 측정 시간
	                                                                                       // 서버가 저장한 lastswing 패킷 저장 시간 + Alpha보다 더 빠르게 도착한거면 비정상으로 판단하여 연결 끊기
private:
	static CMPoolTLS<CUser> m_userPool;

	uint64              m_sessionID;      
	SwingInfo           m_swingInfo;                                                       // 좌 클릭 공격 처리 관련 구조체
	SkillInfo           m_skillInfo[UserConst::USER_SKILL_SLOT_COUNT];
	Location            m_location;                                                        // 캐릭터 위치
	SectorPos           m_secPos;           
	uint16              m_arrayIdx;     
	uint16              m_hp;                                                              // 캐릭터 HP
	uint16              m_mp;                                                              // 캐릭터 MP
	UserStat            m_baseStat;                                                        // 유저 기본 스탯(클래스, 레벨 기반)
	UserStat            m_equipBonusStat;                                                  // 유저 장비 보너스 스탯
	uint16              m_mpRegenPerSec;

	bool                m_moveFlag;
	float               m_movementYaw;                                                     // 캐릭터 이동 방향, 이동 처리시 사용
	float               m_maxWalkSpeed;                                                    // 캐릭터 최대 이동 속도(이벤트 발생시 변화 값)
	float               m_moveSpeed;

};

