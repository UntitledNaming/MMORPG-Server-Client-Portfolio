#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "MemoryPoolTLS.h"
#include "ItemTable.h"
#include "SectorPos.h"
#include "FieldDropItemPool.h"

CMPoolTLS<FieldDropItem>* FieldDropItemPool::m_dropItemPool = nullptr;
uint64                    FieldDropItemPool::m_dropUIDAllocator = 0;

void FieldDropItemPool::Init()
{
    m_dropItemPool = new CMPoolTLS<FieldDropItem>;
}

void FieldDropItemPool::Destroy()
{
    delete m_dropItemPool;
    m_dropItemPool = nullptr;
}

FieldDropItem* FieldDropItemPool::CreateItem(const Location& dropLocation)
{
    ITEM_ID retID = GetItemID();
    if (retID == 0)
        return nullptr;

    // 아이템 테이블에 없으면 nullptr 리턴
    const ItemData* pItemData = ItemTable::GetItemData(retID);
    if (pItemData == nullptr)
        return nullptr;

    return CreateFieldItemByItemData(pItemData, dropLocation);
}

void FieldDropItemPool::FreeItem(FieldDropItem* pDropItem)
{
    if (m_dropItemPool)
    {
        m_dropItemPool->Free(pDropItem);
    }
}

FIELD_DROP_TYPE FieldDropItemPool::RollFieldDropType()
{
    int totalweight = 0;

    for (int i = 0; i < sizeof(g_dropRateTable) / sizeof(DropRate); i++)
    {
        totalweight += g_dropRateTable[i].weight;
    }

    int val = rand() % totalweight;

    int curRate = 0;
    for (int i = 0; i < sizeof(g_dropRateTable) / sizeof(DropRate); i++)
    {
        curRate += g_dropRateTable[i].weight;

        if (val < curRate)
            return g_dropRateTable[i].type;
    }

    return FIELD_DROP_TYPE::NONE;
}

EQUIP_SLOT FieldDropItemPool::RollEquipSlot()
{
    int totalweight = 0;

    for (int i = 0; i < sizeof(g_equipSlotRateTable) / sizeof(EquipSlotRate); i++)
    {
        totalweight += g_equipSlotRateTable[i].weight;
    }

    int val = rand() % totalweight;

    int curRate = 0;
    for (int i = 0; i < sizeof(g_equipSlotRateTable) / sizeof(EquipSlotRate); i++)
    {
        curRate += g_equipSlotRateTable[i].weight;

        if (val < curRate)
            return g_equipSlotRateTable[i].slot;
    }

    return EQUIP_SLOT::NONE;
}

ITEM_ID FieldDropItemPool::GetItemID()
{
    FIELD_DROP_TYPE type = RollFieldDropType();
    switch (type)
    {
    case FIELD_DROP_TYPE::NONE:
        break;

    case FIELD_DROP_TYPE::HP_POTION:
        return ItemIDConst::SMALL_HP_POTION;

    case FIELD_DROP_TYPE::MP_POTION:
        return ItemIDConst::SMALL_MP_POTION;

    case FIELD_DROP_TYPE::NORMAL_EQUIPMENT:
    {
        EQUIP_SLOT slot = RollEquipSlot();
        return GetEquipmentID(ITEM_RARITY::NORMAL, slot);
    }

    case FIELD_DROP_TYPE::MAGIC_EQUIPMENT:
    {
        EQUIP_SLOT slot = RollEquipSlot();
        return GetEquipmentID(ITEM_RARITY::MAGIC, slot);
    }
    }

    return 0;
}

ITEM_ID FieldDropItemPool::GetEquipmentID(ITEM_RARITY rarity, EQUIP_SLOT slot)
{
    if (rarity == ITEM_RARITY::NORMAL)
    {
        switch (slot)
        {
        case EQUIP_SLOT::HELMET:
            return ItemIDConst::NORMAL_HELMET;

        case EQUIP_SLOT::CHEST:
            return ItemIDConst::NORMAL_CHEST;

        case EQUIP_SLOT::PANTS:
            return ItemIDConst::NORMAL_PANTS;

        case EQUIP_SLOT::BOOTS:
            return ItemIDConst::NORMAL_BOOTS;

        case EQUIP_SLOT::WEAPON:
            return ItemIDConst::NORMAL_WEAPON;
        }

        return 0;
    }

    else if (rarity == ITEM_RARITY::MAGIC)
    {
        switch (slot)
        {
        case EQUIP_SLOT::HELMET:
            return ItemIDConst::MAGIC_HELMET;

        case EQUIP_SLOT::CHEST:
            return ItemIDConst::MAGIC_CHEST;

        case EQUIP_SLOT::PANTS:
            return ItemIDConst::MAGIC_PANTS;

        case EQUIP_SLOT::BOOTS:
            return ItemIDConst::MAGIC_BOOTS;

        case EQUIP_SLOT::WEAPON:
            return ItemIDConst::MAGIC_WEAPON;
        }
    }

    return 0;
}

FieldDropItem* FieldDropItemPool::CreateFieldItemByItemData(const ItemData* itemData, const Location& dropLocation)
{
    FieldDropItem* pDropItem = m_dropItemPool->Alloc();
    pDropItem->dropUID = m_dropUIDAllocator++;
    pDropItem->expiredTime = 0;
    pDropItem->itemID = itemData->itemID;
    pDropItem->location.xpos = dropLocation.xpos;
    pDropItem->location.ypos = dropLocation.ypos;
    pDropItem->location.zpos = dropLocation.zpos;
    pDropItem->sectorPos.SetPos(SectorPos(SectorPos((dropLocation.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (dropLocation.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE)));
    for (int i = 0; i < FieldDropItemConst::FIELD_DROP_ITEM_RANDOM_STAT_MAX; i++)
    {
        pDropItem->randomStat[i] = {};
    }

    switch (itemData->itemType)
    {
    case ITEM_TYPE::CONSUMABLE:
        // 포션 갯수 랜덤 생성
        pDropItem->count = CreateConsumableItemCount(pDropItem->itemID);
        break;

    case ITEM_TYPE::EQUIPMENT:
        // 랜덤 스탯 생성
        pDropItem->count = 1;
        CreateRandomStat(itemData, pDropItem->randomStat, pDropItem->randomStatCount);
        break;
    }

    return pDropItem;
}

uint16 FieldDropItemPool::CreateConsumableItemCount(ITEM_ID itemID)
{
    switch (itemID)
    {
    case ItemIDConst::SMALL_HP_POTION:
        return 1 + rand() % FieldDropItemConst::CREATE_SMALL_HP_POTION_MAX;

    case ItemIDConst::SMALL_MP_POTION:
        return 1 + rand() % FieldDropItemConst::CREATE_SMALL_MP_POTION_MAX;
    }

    return 1;
}

void FieldDropItemPool::CreateRandomStat(const ItemData* itemData, RandomStatResult randomStat[], uint8& OutRandomStatCount)
{
    int randomStatCount = 0;
    const RandomStatRule* rule = nullptr;
    uint16 ruleCount = 0;

    switch (itemData->itemRarity)
    {
    case ITEM_RARITY::NORMAL:
        randomStatCount = 1;
        rule = g_normalRandomStatRules;
        ruleCount = sizeof(g_normalRandomStatRules) / sizeof(RandomStatRule);
        break;

    case ITEM_RARITY::MAGIC:
        randomStatCount = 2;
        rule = g_magicRandomStatRules;
        ruleCount = sizeof(g_magicRandomStatRules) / sizeof(RandomStatRule);
        break;
    }

    if (randomStatCount > FieldDropItemConst::FIELD_DROP_ITEM_RANDOM_STAT_MAX)
        randomStatCount = FieldDropItemConst::FIELD_DROP_ITEM_RANDOM_STAT_MAX;


    // 해당 장비 슬롯에 부여할 랜덤 스탯 타입 뽑기
    RANDOM_STAT_TYPE store[(int)RANDOM_STAT_TYPE::MAX - 1] = {};
    uint16 storeCount = 0;

    RANDOM_STAT_TYPE pickUp[(int)RANDOM_STAT_TYPE::MAX - 1] = {};
    uint16 pickUpCount = 0;

    // store에 장비 슬롯 별로 허용 가능한 스탯 담기
    GetAllowedRandomStatType(itemData->equipSlot, store, storeCount);

    // 적용해야할 랜덤 스탯 갯수가 허용 가능한 스탯 개수보다 크면 허용 가능한 스탯 갯수로 맞추기
    if (randomStatCount > storeCount)
        randomStatCount = storeCount;
    
    OutRandomStatCount = randomStatCount;

    // store에서 뽑은 것 PickUp 배열에 담고 맨 뒤 원소로 교체해서 덮어버리기
    for (int i = 0; i < randomStatCount; i++)
    {
        int idx = rand() % storeCount;

        pickUp[pickUpCount++] = store[idx];
        store[idx] = store[storeCount - 1];
        storeCount--;
    }

    // 뽑은 랜덤 스탯 타입에 따라 테이블에서 
    for (int i = 0; i < pickUpCount; i++)
    {
        SetRandomStat(rule, ruleCount, pickUp[i], randomStat[i]);
    }
}

void FieldDropItemPool::GetAllowedRandomStatType(EQUIP_SLOT slot, RANDOM_STAT_TYPE* array, uint16& count)
{
    switch (slot)
    {
    case EQUIP_SLOT::WEAPON:
        array[count++] = RANDOM_STAT_TYPE::ATK;
        array[count++] = RANDOM_STAT_TYPE::MAX_HP;
        array[count++] = RANDOM_STAT_TYPE::MAX_MP;
        array[count++] = RANDOM_STAT_TYPE::MP_REGEN;
        break;

    case EQUIP_SLOT::HELMET:
        array[count++] = RANDOM_STAT_TYPE::MAX_HP;
        array[count++] = RANDOM_STAT_TYPE::MAX_MP;
        array[count++] = RANDOM_STAT_TYPE::DEF;
        array[count++] = RANDOM_STAT_TYPE::MP_REGEN;
        break;

    case EQUIP_SLOT::CHEST:
        array[count++] = RANDOM_STAT_TYPE::MAX_HP;
        array[count++] = RANDOM_STAT_TYPE::DEF;
        array[count++] = RANDOM_STAT_TYPE::HP_REGEN;
        break;

    case EQUIP_SLOT::PANTS:
        array[count++] = RANDOM_STAT_TYPE::MAX_HP;
        array[count++] = RANDOM_STAT_TYPE::DEF;
        array[count++] = RANDOM_STAT_TYPE::HP_REGEN;
        array[count++] = RANDOM_STAT_TYPE::MAX_MP;
        break;

    case EQUIP_SLOT::BOOTS:
        array[count++] = RANDOM_STAT_TYPE::MAX_HP;
        array[count++] = RANDOM_STAT_TYPE::MAX_MP;
        array[count++] = RANDOM_STAT_TYPE::HP_REGEN;
        array[count++] = RANDOM_STAT_TYPE::MP_REGEN;
        break;
    }
}

void FieldDropItemPool::SetRandomStat(const RandomStatRule* rule, uint16 ruleCount, RANDOM_STAT_TYPE type, RandomStatResult& randomStat)
{
    // rule에서 type에 해당하는 min, max 찾기
    uint16 min = 0;
    uint16 max = 0;

    for (int i = 0; i < ruleCount; i++)
    {
        if (rule[i].type != type)
            continue;

        min = rule[i].minVal;
        max = rule[i].maxVal;
        break;
    }

    randomStat.randomStatType = type;
    randomStat.randomStatValue = min + rand() % (max - min + 1);
}


