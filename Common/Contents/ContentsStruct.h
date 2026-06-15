#pragma once
#include <vector>
#include "ContentsType.h"
#include "ContentsEnum.h"
#include "ContentsDefine.h"

struct Vec2
{
	float m_xpos;
	float m_ypos;
};

struct SkillData
{
	uint16           MaxUserCount;    
	uint16           MaxMonsterCount; 
	uint16           RequiredMana;    
	uint32           CoolTime;        
	uint32           Duration;        
	uint16           BaseDamage;      
	float            Range ;          
	float            AttackRatio;     
	float            HalfAngleDegree; 
	bool             bHitUser;        
	bool             bHitMonster;     
	ESkillDamageType DamageType;      
	EHitShape        HitShape;        
};

struct Location
{
	float xpos;
	float ypos;
	float zpos;
};

struct ItemStat
{
	int16 atk = 0;
	int16 def = 0;
	int16 maxHP = 0;
	int16 maxMP = 0;
	uint16 hpRegenPerSec = 0;
	uint16 mpRegenPerSec = 0;
};

struct ItemData
{
	ITEM_ID              itemID = 0;
			             
	ITEM_TYPE            itemType = ITEM_TYPE::NONE;
	EQUIP_SLOT           equipSlot = EQUIP_SLOT::NONE;
				         
	ITEM_RARITY          itemRarity = ITEM_RARITY::NORMAL;
				         
	uint16               maxStack  = 1;
	uint16               recoverHP = 0;
	uint16               recoverMP = 0;
	CONSUMABLE_ITEM_TYPE consumableType;
	ItemStat             baseStat;
};

struct RandomStatResult
{
	RANDOM_STAT_TYPE randomStatType;
	uint16           randomStatValue;
};

struct UpdateSlotResult
{
	int16           slotIndex;
	uint16          newItemCount;
};

struct BaseItemInfo
{
	ITEM_ID             itemID = 0;
	uint16              count = 0;
	uint8               randomStatCount = 0;
	RandomStatResult    randomStat[FieldDropItemConst::FIELD_DROP_ITEM_RANDOM_STAT_MAX];
};

struct UserItem : public BaseItemInfo
{
	ITEM_UID            itemUID = 0;
};

struct DBItem : public BaseItemInfo
{
	ITEM_UID            itemUID = 0;
};

struct UIDRange
{
	ITEM_UID allocID = 0;
	ITEM_UID lastID = 0;
};

struct DropRate
{
	FIELD_DROP_TYPE type;
	uint16          weight;
};

struct EquipSlotRate
{
	EQUIP_SLOT slot;
	uint16     weight;
};

struct RandomStatRule
{
	RANDOM_STAT_TYPE type = RANDOM_STAT_TYPE::NONE;
	uint16           minVal = 0;
	uint16           maxVal = 0;
};


struct PickUpEquipResult
{
	ITEM_ID          itemID;
	int16            slotIndex;
	uint16           count;
	uint8            randomStatCount;
	RandomStatResult randomStatResult[FieldDropItemConst::FIELD_DROP_ITEM_RANDOM_STAT_MAX];
};

struct PickUpConsumableResult
{
	ITEM_ID          itemID;
	uint16           updateSlotCount;

	UpdateSlotResult consumableResult[UserInventory::INVENTORY_SLOT_MAX];
};

struct UPDATE_SLOT
{
	SLOT_TYPE  slotType;
	int16      slotIndex;
	ITEM_ID    itemID;
};

struct EQUIP_ITEM_RESULT
{
	uint8       updateSlotCount;
	UPDATE_SLOT resultSlot[2];    // 0 : Inventory , 1 : Equipment
};

struct UNEQUIP_ITEM_RESULT
{
	int16       inventorySlotIdx;
};

struct USE_CONSUMABLE_ITEM_RESULT
{
	SLOT_TYPE slotType;
	uint16    newItenCount;
	int16     slotIndex;
};

struct UseItemResult
{
	bool                       success          = false;
	USE_ITEM_RESULT            resultType       = USE_ITEM_RESULT::NONE;
	EQUIP_ITEM_RESULT          equipResult      = {};
	UNEQUIP_ITEM_RESULT        unEquipResult    = {};
	USE_CONSUMABLE_ITEM_RESULT consumableResult = {};
};

struct UserLevelStat
{
	uint16 level;
	uint32 requiredExp;

	int16  atk;
	int16  def;
	int16  maxHP;
	int16  maxMP;

	uint16 hpRegenPerSec;
	uint16 mpRegenPerSec;
};

struct RecoveryInfo
{
	uint32 HP_accumulatedTimeMs = 0;
	uint32 MP_accumulatedTimeMs = 0;

	void Init() {
		HP_accumulatedTimeMs = 0;  MP_accumulatedTimeMs
			= 0;
	}
};

struct ConsumableCooltimeInfo
{
	uint32 CooltimeStartTimeMs[(int)CONSUMABLE_ITEM_TYPE::MAX] = {};
	uint32 CoolTimeMs[(int)CONSUMABLE_ITEM_TYPE::MAX] = {};

	void Init() {
		for (int i = 0; i < (int)CONSUMABLE_ITEM_TYPE::MAX; i++)
		{
			CooltimeStartTimeMs[i] = 0;

			switch (static_cast<CONSUMABLE_ITEM_TYPE>(i))
			{
			case CONSUMABLE_ITEM_TYPE::SMALL_HP_POTION:
				CoolTimeMs[i] = ConsumableConst::HP_POTION_USE_COOLTIME_MS;
				break;

			case CONSUMABLE_ITEM_TYPE::SMALL_MP_POTION:
				CoolTimeMs[i] = ConsumableConst::MP_POTION_USE_COOLTIME_MS;
				break;
			}
		}
	}
};

struct GainEXPResult
{
	bool   levelUp = false;
	uint32 curEXP = 0;

	// levelUp == true 일때
	uint16 curLevel;
	int16  curHP;
	int16  curMP;
};