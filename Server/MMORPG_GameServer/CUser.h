#pragma once
#include "SectorPos.h"
#include "ContentsType.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"

struct SwingInfo
{
	uint8  m_lastSwingIdx;
	uint32 m_lastSwingRecvTime;
};

struct UserStat
{
	int16    m_atk;
	int16    m_def;
	int16    m_maxHP;
	int16    m_maxMP;
	uint16   m_hpRegenPerSec;
	uint16   m_mpRegenPerSec;
};

struct SkillInfo
{
	bool   m_skillActivate;     // skill Activate Flag
	uint32 m_skillLastRecvTime; // skill Coll Time
	uint32 m_skillExpiredTime;  // skill Expired Time
};

class CMonster;

class CUser : public IUser
{
public:
	CUser() = default;
	~CUser() = default;
	
	void   Init(uint64 sessionID);
	void   Destroy();
	void   ResPawn();
	void   ManaRegen(uint32 curTime);
	void   Damage(uint16 damage);
	void   UseSkill(uint32 curTime, uint8 skillIndex);
	void   SetNewSectorPos(const SectorPos& newSec) { m_secPos = newSec; }
	void   CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector);
	void   SwingStop() { m_swingInfo.m_lastSwingIdx = 0; }
	bool   CanUseSkill(uint32 curTime, uint8 skillIndex);
	bool   Move();
	bool   CanSwing(uint32 curTime, uint8 swingidx);
	bool   IsAlive();
	bool   GetItem(FieldDropItem& dropItem);
	bool   DeleteItem(uint16 inventorySlotIndex);
	bool   UseItem(uint16 inventorySlotIndex);
	bool   InventoryItemSlotChange(uint16 fromIndex, uint16 toIndex);
	bool   EquippedItem(uint16 inventorySlotIndex);
	bool   UnEquippedItem(EQUIP_SLOT equipSlot);


	uint64 GetSessionID() const { return m_sessionID; }
	uint32 CalSkillDamage(uint16 skillIndex, CUser* target, uint32 curTime);
	uint32 CalSkillDamage(uint16 skillIndex, CMonster* target, uint32 curTime);
	uint32 CalBaseAttackDamage(CUser* target, uint32 curTime);
	uint32 CalBaseAttackDamage(CMonster* target, uint32 curTime);
	uint16 GetDef(uint32 curTime);
	uint16 GetAtk(uint32 curTime);
	uint16 GetMaxHP(uint32 curTime);
	uint16 GetMaxMP(uint32 curTime);
	uint16 GetHP() const { return m_hp; }
	uint16 GetMP() const { return m_mp; }
	uint16 GetHPRegenSec() const;
	uint16 GetMPRegenSec() const;
	uint16 GetSectorArrayIdx() const { return m_arrayIdx; }
	uint16 GetSectorXpos() const { return m_secPos.GetX(); }
	uint16 GetSectorYpos() const { return m_secPos.GetY(); }
	uint8  GetLastSwingIndex() const { return m_swingInfo.m_lastSwingIdx; }

	float  GetX() const { return m_location.xpos; }
	float  GetY() const { return m_location.ypos; }
	float  GetZ() const { return m_location.zpos; }
	float  GetMoveYaw() const { return m_movementYaw; }
	bool   GetMoveFlag() const { return m_moveFlag; }
	bool   GetDisconnectFlag() const { return m_disconnectFlag; }

	const SectorPos& GetSectorPos() const { return m_secPos; }
	const Location& GetLocation() const { return m_location; }
	void SetLocation(Location& location) { m_location = location; }
	void SetMoveYaw(float moveYaw) { m_movementYaw = moveYaw; }
	void SetSectorArrayIdx(uint16 idx) { m_arrayIdx = idx; }
	void SetMoveFlag(bool flag) { m_moveFlag = flag; }
	void SetDisconnectFlag(bool flag) { m_disconnectFlag = flag; }

	static CUser* Alloc();
	static void Free(CUser* pUser);

public:
	uint64 m_syncCount;
	uint32 m_recvTime;
	uint32 m_lastSyncCheckTime;

private:
	static CMPoolTLS<CUser> m_userPool;

	uint64              m_sessionID;
	uint64              m_currentExp;
	uint64              m_requiredExp;
	SwingInfo           m_swingInfo;                                                       // 좌 클릭 공격 처리 관련 구조체
	SkillInfo           m_skillInfo[UserConst::USER_SKILL_SLOT_COUNT];
	Location            m_location;                                                        // 캐릭터 위치
	SectorPos           m_secPos;           
	uint16              m_level;
	uint16              m_arrayIdx;     
	int16               m_hp;                                                              // 캐릭터 HP
	int16               m_mp;                                                              // 캐릭터 MP
	UserStat            m_baseStat;                                                        // 유저 기본 스탯(클래스, 레벨 기반)
	Inventory           m_inventory;        // Inventory
	Equipment           m_equipment;        // Currently Equipped Items
	CUserItemStorage    m_storage;          // Item Storage

	bool                m_disconnectFlag;
	bool                m_moveFlag;
	float               m_movementYaw;                                                     // 캐릭터 이동 방향, 이동 처리시 사용
	float               m_maxWalkSpeed;                                                    // 캐릭터 최대 이동 속도(이벤트 발생시 변화 값)
	float               m_moveSpeed;
};

