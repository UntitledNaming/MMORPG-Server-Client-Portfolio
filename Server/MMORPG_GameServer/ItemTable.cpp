#include <unordered_map>
#include "MemoryPoolTLS.h"
#include "ItemTable.h"

std::unordered_map<ITEM_ID, ItemData*> ItemTable::m_itemTable;
CMPoolTLS<ItemData>*                   ItemTable::m_pItemDataPool = nullptr;


void ItemTable::Init()
{
	m_pItemDataPool = new CMPoolTLS<ItemData>;

	// itemTable에 데이터 채우기
	// 
	////////////////////////////////////////////////
	// Consumable
	////////////////////////////////////////////////

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::SMALL_HP_POTION;
		data->itemType = ITEM_TYPE::CONSUMABLE;
		data->equipSlot = EQUIP_SLOT::NONE;
		data->itemRarity = ITEM_RARITY::NORMAL;
		data->consumableType = CONSUMABLE_ITEM_TYPE::SMALL_HP_POTION;

		data->maxStack = 10;
		data->recoverHP = 40;
		data->recoverMP = 0;
		data->baseStat = {};

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::SMALL_HP_POTION, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::SMALL_MP_POTION;
		data->itemType = ITEM_TYPE::CONSUMABLE;
		data->equipSlot = EQUIP_SLOT::NONE;
		data->itemRarity = ITEM_RARITY::NORMAL;
		data->consumableType = CONSUMABLE_ITEM_TYPE::SMALL_MP_POTION;

		data->maxStack = 10;
		data->recoverHP = 0;
		data->recoverMP = 35;

		data->baseStat = {};

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::SMALL_MP_POTION, data));
	}

	////////////////////////////////////////////////
	// Normal Equipment
	////////////////////////////////////////////////

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::NORMAL_HELMET;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::HELMET;
		data->itemRarity = ITEM_RARITY::NORMAL;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 0;
		data->baseStat.maxHP = 15;
		data->baseStat.maxMP = 10;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::NORMAL_HELMET, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::NORMAL_CHEST;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::CHEST;
		data->itemRarity = ITEM_RARITY::NORMAL;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 1;
		data->baseStat.maxHP = 25;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::NORMAL_CHEST, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::NORMAL_PANTS;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::PANTS;
		data->itemRarity = ITEM_RARITY::NORMAL;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 1;
		data->baseStat.maxHP = 20;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::NORMAL_PANTS, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::NORMAL_BOOTS;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::BOOTS;
		data->itemRarity = ITEM_RARITY::NORMAL;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 0;
		data->baseStat.maxHP = 10;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 1;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::NORMAL_BOOTS, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::NORMAL_WEAPON;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::WEAPON;
		data->itemRarity = ITEM_RARITY::NORMAL;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 5;
		data->baseStat.def = 0;
		data->baseStat.maxHP = 0;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::NORMAL_WEAPON, data));
	}

	////////////////////////////////////////////////
	// Magic Equipment
	////////////////////////////////////////////////

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::MAGIC_HELMET;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::HELMET;
		data->itemRarity = ITEM_RARITY::MAGIC;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 0;
		data->baseStat.maxHP = 25;
		data->baseStat.maxMP = 20;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::MAGIC_HELMET, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::MAGIC_CHEST;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::CHEST;
		data->itemRarity = ITEM_RARITY::MAGIC;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 2;
		data->baseStat.maxHP = 35;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::MAGIC_CHEST, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::MAGIC_PANTS;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::PANTS;
		data->itemRarity = ITEM_RARITY::MAGIC;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 1;
		data->baseStat.maxHP = 30;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::MAGIC_PANTS, data));;
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::MAGIC_BOOTS;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::BOOTS;
		data->itemRarity = ITEM_RARITY::MAGIC;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 0;
		data->baseStat.def = 0;
		data->baseStat.maxHP = 20;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 2;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::MAGIC_BOOTS, data));
	}

	{
		ItemData* data = m_pItemDataPool->Alloc();;

		data->itemID = ItemIDConst::MAGIC_WEAPON;
		data->itemType = ITEM_TYPE::EQUIPMENT;
		data->equipSlot = EQUIP_SLOT::WEAPON;
		data->itemRarity = ITEM_RARITY::MAGIC;

		data->maxStack = 1;
		data->recoverHP = 0;
		data->recoverMP = 0;

		data->baseStat.atk = 7;
		data->baseStat.def = 0;
		data->baseStat.maxHP = 0;
		data->baseStat.maxMP = 0;
		data->baseStat.hpRegenPerSec = 0;
		data->baseStat.mpRegenPerSec = 0;

		m_itemTable.insert(std::pair<ITEM_ID, ItemData*>(ItemIDConst::MAGIC_WEAPON, data));
	}
}

void ItemTable::Destroy()
{
	// itemTable에 ItemData* 반납 및 자료구조 비우기
	std::unordered_map<ITEM_ID, ItemData*>::iterator it = m_itemTable.begin();
	for (; it != m_itemTable.end(); ++it)
	{
		m_pItemDataPool->Free(it->second);
	}

	delete m_pItemDataPool;
	m_pItemDataPool = nullptr;
}

const ItemData* ItemTable::GetItemData(ITEM_ID ItemID)
{
	std::unordered_map<ITEM_ID, ItemData*>::iterator it = m_itemTable.find(ItemID);
	if (it == m_itemTable.end())
		return nullptr;

	return it->second;
}
