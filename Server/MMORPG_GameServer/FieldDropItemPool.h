#pragma once
#include "ContentsType.h"
#include "ContentsStruct.h"

struct FieldDropItem;
struct ItemCreateInfo;
struct Location;
template <typename T>
class CMPoolTLS;

static constexpr DropRate g_dropRateTable[] =
{
    { FIELD_DROP_TYPE::NONE,             40 },
    { FIELD_DROP_TYPE::HP_POTION,        20 },
    { FIELD_DROP_TYPE::MP_POTION,        10 },
    { FIELD_DROP_TYPE::NORMAL_EQUIPMENT, 20 },
    { FIELD_DROP_TYPE::MAGIC_EQUIPMENT,  10 },
};

static constexpr EquipSlotRate g_equipSlotRateTable[] =
{
    { EQUIP_SLOT::WEAPON, 25 },
    { EQUIP_SLOT::CHEST,  25 },
    { EQUIP_SLOT::PANTS,  20 },
    { EQUIP_SLOT::HELMET, 15 },
    { EQUIP_SLOT::BOOTS,  15 },
};

static constexpr RandomStatRule g_normalRandomStatRules[] =
{
    { RANDOM_STAT_TYPE::ATK,      1, 3},
    { RANDOM_STAT_TYPE::DEF,      1, 1},
    { RANDOM_STAT_TYPE::MAX_HP,   10, 20},
    { RANDOM_STAT_TYPE::MAX_MP,   10, 20},
    { RANDOM_STAT_TYPE::HP_REGEN, 1, 1},
    { RANDOM_STAT_TYPE::MP_REGEN, 1, 1},
};

static constexpr RandomStatRule g_magicRandomStatRules[] =
{
    { RANDOM_STAT_TYPE::ATK,      2, 4},
    { RANDOM_STAT_TYPE::DEF,      1, 1},
    { RANDOM_STAT_TYPE::MAX_HP,   20, 30},
    { RANDOM_STAT_TYPE::MAX_MP,   20, 30},
    { RANDOM_STAT_TYPE::HP_REGEN, 1, 2},
    { RANDOM_STAT_TYPE::MP_REGEN, 1, 2},
};

struct FieldDropItem : public BaseItemInfo
{
    uint64              dropUID = 0;
    uint32              expiredTime = 0;
    uint16              sectorIdx = 0;
    Location            location;
    SectorPos           sectorPos;
};

class FieldDropItemPool
{
public:
    static void Init();
    static void Destroy();

    static FieldDropItem* CreateItem(const Location& dropLocation);
    static void FreeItem(FieldDropItem* pDropItem);
    static uint64 GetDropItemUseCount();

private:
    static FIELD_DROP_TYPE RollFieldDropType();
    static EQUIP_SLOT      RollEquipSlot();
    static ITEM_ID         GetItemID();
    static ITEM_ID         GetEquipmentID(ITEM_RARITY rarity, EQUIP_SLOT slot);
    static FieldDropItem*  CreateFieldItemByItemData(const ItemData* itemData, const Location& dropLocation);
    static uint16          CreateConsumableItemCount(ITEM_ID itemID);
    static void            CreateRandomStat(const ItemData* itemData, RandomStatResult randomStat[], uint8& OutRandomStatCount);
    static void            GetAllowedRandomStatType(EQUIP_SLOT slot, RANDOM_STAT_TYPE* array, uint16& count);
    static void            SetRandomStat(const RandomStatRule* rule, uint16 ruleCount, RANDOM_STAT_TYPE type, RandomStatResult& randomStat);
private:
    static CMPoolTLS<FieldDropItem>* m_dropItemPool;
    static uint64                    m_dropUIDAllocator;                                 // 필드에 떨어진 드랍 아이템 구분용 UID(아이템 UID와 다름)
};

