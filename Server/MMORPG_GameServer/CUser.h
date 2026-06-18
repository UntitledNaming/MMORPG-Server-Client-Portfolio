#pragma once
#include "SectorPos.h"
#include "ContentsType.h"
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"

struct FieldDropItem;
class CMonster;
class CDBManager;

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

constexpr UserLevelStat g_userLevelStatTable[10] =
{
	// Lv, RequiredExp, ATK, DEF, MaxHP, MaxMP, HPRegen, MPRegen
	{ 1,     100, 12, 3, 130, 100, 1, 3 },
	{ 2,     150, 14, 3, 150, 110, 1, 4 },
	{ 3,     230, 16, 4, 170, 120, 2, 4 },
	{ 4,     350, 18, 4, 190, 130, 2, 5 },
	{ 5,     520, 20, 5, 210, 140, 2, 5 },
	{ 6,     750, 22, 5, 230, 150, 3, 6 },
	{ 7,    1050, 24, 6, 250, 160, 3, 6 },
	{ 8,    1450, 26, 6, 270, 170, 3, 7 },
	{ 9,    1950, 28, 7, 290, 180, 4, 7 },
	{ 10,   2600, 30, 7, 310, 190, 4, 8 },
};

class CUser : public IUser
{
public:
	CUser() = default;
	~CUser() = default;
	
	void   Init(uint64 sessionID, CDBManager* pDBManager);
	void   Destroy();
	void   ResPawn();
	void   LoadDataFromDB(uint64 characterUID, uint64 accountID, uint16 level, int32 curExp, Location& location, std::vector<ItemLoadData>& items);
	void   UpdateRecovery(uint32 curTime);
	void   Damage(uint16 damage);
	void   UseSkill(uint32 curTime, uint8 skillIndex);
	void   SetNewSectorPos(const SectorPos& newSec) { m_secPos = newSec; }
	void   CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector);

	bool   GainExp(uint32 GetExp, GainEXPResult& result);
	bool   CanUseSkill(uint32 curTime, uint8 skillIndex);
	bool   Move();
	bool   IsAlive();
	bool   GetConsumableItem(FieldDropItem& dropItem, PickUpConsumableResult& OutResult);
	bool   GetEquipmentItem(FieldDropItem& dropItem, PickUpEquipResult& OutResult);
	bool   DeleteItem(int16 slotIndex, SLOT_TYPE slotType);
	bool   UseInventoryItem(int16 slotIndex, UseItemResult& result);
	bool   UseEquipmentItem(int16 slotIndex, UseItemResult& result);
	bool   UseQuickSlotItem(int16 slotIndex, UseItemResult& result);
	bool   ItemSlotChange(SLOT_TYPE fromType, int16 fromIndex, SLOT_TYPE toType, int16 toIndex);

	uint64 GetSessionID() const { return m_sessionID; }
	uint64 GetAccountID() const { return m_accountID; }
	uint64 GetCharacterUID() const { return m_characterUID; }
	int32  GetCurrentEXP() const { return m_currentExp; }
	uint32 CalSkillDamage(uint16 skillIndex, CUser* target, uint32 curTime);
	uint32 CalSkillDamage(uint16 skillIndex, CMonster* target, uint32 curTime);
	uint32 CalBaseAttackDamage(CUser* target, uint32 curTime);
	uint32 CalBaseAttackDamage(CMonster* target, uint32 curTime);
    int16  GetDef(uint32 curTime);
    int16  GetAtk(uint32 curTime);
    int16  GetMaxHP(uint32 curTime);
    int16  GetMaxMP(uint32 curTime);
	int16  GetHP() const { return m_hp; }
	int16  GetMP() const { return m_mp; }
	int16  GetHPRegenSec() const;
	int16  GetMPRegenSec() const;
	uint16 GetSectorArrayIdx() const { return m_arrayIdx; }
	uint16 GetSectorXpos() const { return m_secPos.GetX(); }
	uint16 GetSectorYpos() const { return m_secPos.GetY(); }
	uint16 GetLevel() const { return m_level; }

	float  GetX() const { return m_location.xpos; }
	float  GetY() const { return m_location.ypos; }
	float  GetZ() const { return m_location.zpos; }
	float  GetMoveYaw() const { return m_movementYaw; }
	bool   GetMoveFlag() const { return m_moveFlag; }

	const SectorPos& GetSectorPos() const { return m_secPos; }
	const Location& GetLocation() const { return m_location; }
	const Inventory& GetInventory() const{ return m_inventory; }
	const Equipment& GetEquipment() const { return m_equipment; }
	const QuickSlot& GetQuickslot() const { return m_quickSlot; }
	const CUserItemStorage& GetItemStorage() const { return m_storage; };

	void SetLocation(Location& location) { m_location = location; }
	void SetMoveYaw(float moveYaw) { m_movementYaw = moveYaw; }
	void SetSectorArrayIdx(uint16 idx) { m_arrayIdx = idx; }
	void SetMoveFlag(bool flag) { m_moveFlag = flag; }
	void SetAccountID(uint64 accountID) { m_accountID = accountID; }
	void SetCharacterUID(uint64 characterUID) { m_characterUID = characterUID; }

	static CUser* Alloc();
	static void Free(CUser* pUser);

private:
	void InventoryItemLoad(ItemLoadData& Item);
	void EquipmentItemLoad(ItemLoadData& Item);
	void QuickSlotItemLoad(ItemLoadData& Item);
	void SkillInfoInit();
	void BaseStatInit(uint16 level);
	bool UseConsumableItem(const UserItem* pUserItem, const ItemData* pItemData, uint16& newItemCount);
	bool CanUseConsumalbIetem(uint32 curTime, CONSUMABLE_ITEM_TYPE itemType);
	bool EquippedItem(int16 inventorySlotIndex, UseItemResult& result);
	bool UnEquippedItem(EQUIP_SLOT equipSlot);
	bool SlotTypeRangeCheck(SLOT_TYPE type);
	bool SwapInventoryEquipment(int16 inventoryIndex, EQUIP_SLOT equipSlot);
	bool SwapInventoryQuickSlot(int16 inventoryIndex, int16 quickSlotIndex);

public:
	uint64 m_syncCount;
	uint32 m_recvTime;
	uint32 m_lastSyncCheckTime;


private:
	static CMPoolTLS<CUser> m_userPool;
	CDBManager*             m_pDBManager = nullptr;

	uint64                 m_sessionID;
	uint64                 m_accountID;
	uint64                 m_characterUID;
	int32                  m_currentExp;
	int32                  m_requiredExp;
	SkillInfo              m_skillInfo[UserConst::USER_SKILL_SLOT_COUNT];
	Location               m_location;                                                        // 캐릭터 위치
	SectorPos              m_secPos;           
	uint16                 m_level;
	uint16                 m_arrayIdx;     
	int16                  m_hp;                                                              // 캐릭터 HP
	int16                  m_mp;                                                              // 캐릭터 MP
	UserStat               m_baseStat;                                                        // 유저 기본 스탯(클래스, 레벨 기반)
	Inventory              m_inventory;                                                       // Inventory
	Equipment              m_equipment;                                                       // Currently Equipped Items Slots
	QuickSlot              m_quickSlot;                                                       // Consumable Item Slots
	CUserItemStorage       m_storage;                                                         // Item Storage
	RecoveryInfo           m_recoveryInfo;    
	ConsumableCooltimeInfo m_consumableCooltimeInfo;
	bool                   m_moveFlag;
	float                  m_movementYaw;                                                     // 캐릭터 이동 방향, 이동 처리시 사용
	float                  m_maxWalkSpeed;                                                    // 캐릭터 최대 이동 속도(이벤트 발생시 변화 값)
	float                  m_moveSpeed;
};

