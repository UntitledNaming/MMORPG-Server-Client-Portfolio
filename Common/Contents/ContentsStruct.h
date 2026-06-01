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
	uint16           MaxUserCount;    // 타격시 최대 피격 유저수
	uint16           MaxMonsterCount; // 타격시 최대 피격 몬스터수
	uint16           RequiredMana;    // 필요 마나
	uint32           CoolTime;        // 쿨 타임
	uint32           Duration;        // 지속 시간
	uint16           BaseDamage;      // 고정 데미지
	float            Range ;          // 공격 거리
	float            AttackRatio;     // 공격력 계수
	float            HalfAngleDegree; // 공격 범위 절반 각도
	bool             bHitUser;        // 유저 피격 할지 말지
	bool             bHitMonster;     // 몬스터 피격 할지 말지
	ESkillDamageType DamageType;      
	EHitShape        HitShape;        // 공격 모양
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
	uint32           itemID = 0;
			         
	ITEM_TYPE        itemType = ITEM_TYPE::NONE;
	EQUIP_SLOT       euipSlot = EQUIP_SLOT::NONE;
				     
	ITEM_RARITY      itemRarity = ITEM_RARITY::NORMAL;
				     
	uint16           maxStack = 1;
	uint16           recoverHP = 0;
	uint16           recoverMP = 0;
				     
	ItemStat         baseStat;
};


struct UserItem
{
	uint64           m_itemUID = 0;
	uint32           m_itemID = 0;
		             
	uint16           m_count = 0;
				     
	ITEM_RARITY      m_rarity = ITEM_RARITY::NORMAL;
				     
	ItemStat         m_randomStat;

};

struct ItemCreateInfo
{
	uint32      itemID = 0;
	uint16      count = 0;
	ITEM_RARITY rarity = ITEM_RARITY::NORMAL;
	ItemStat    randomStat;
};

struct DBItemInfo
{
	uint64      itemUID = 0;
	uint32      itemID = 0;
		        
	uint16      count = 0;

	ITEM_RARITY rarity = ITEM_RARITY::NORMAL;

	ItemStat    randomStat;
};