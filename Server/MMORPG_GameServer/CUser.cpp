#include <string>
#include <windows.h>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include "ContentsEnum.h"
#include "SkillTable.h"
#include "CMonster.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "ItemTable.h"
#include "IUser.h"
#include "CUser.h"

CMPoolTLS<CUser> CUser::m_userPool;

using namespace UserConst;

void CUser::Init(uint64 sessionID)
{
	m_inventory.Init(&m_storage);
	m_equipment.Init(&m_storage);
	m_storage.Init();

	// todo : 추후 DB에서 데이터 긁어와서 초기화 하기
	m_sessionID = sessionID;
	m_location = Location{ 381250.0f , 443750.0f ,-38775.f };
	m_moveFlag = false;
	m_disconnectFlag = false;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
	m_arrayIdx = 0;
	m_syncCount = 0;
	m_movementYaw = 0.0f;
	m_maxWalkSpeed = WALK_SPEED;
	m_moveSpeed = m_maxWalkSpeed / FieldConst::UPDATE_FRAME;
	m_recvTime = timeGetTime();
	m_lastSyncCheckTime = timeGetTime();

	m_swingInfo.Init();
	SkillInfoInit();

	// 스탯 초기화
	m_level = 1;
	m_currentExp = 0;

	BaseStatInit(m_level);

	uint32 curTime = timeGetTime();
	m_hp = GetMaxHP(curTime);
	m_mp = GetMaxMP(curTime);

	// 리커버리 정보 초기화
	m_recoveryInfo.Init();

	// 소비품 쿨타임 초기화
	m_consumableCooltimeInfo.Init();
}

void CUser::Destroy()
{
	m_equipment.Destroy();
	m_inventory.Destroy();
	m_quickSlot.Destroy();
	m_storage.Destroy();
}

void CUser::ResPawn()
{
	uint32 curTime = timeGetTime();

	m_disconnectFlag = false;
	m_moveFlag = false;
	m_arrayIdx = 0;
	m_syncCount = 0;
	m_movementYaw = 0.0f;
	m_recvTime = curTime;
	m_lastSyncCheckTime = curTime;

	m_swingInfo.m_lastSwingIdx = 0;
	m_swingInfo.m_lastSwingRecvTime = 0;

	for (int i = 0; i < USER_SKILL_SLOT_COUNT; i++)
	{
		m_skillInfo[i].m_skillActivate = false;
		m_skillInfo[i].m_skillExpiredTime = 0;
		m_skillInfo[i].m_skillLastRecvTime = 0;
	}

	m_hp = GetMaxHP(curTime);
	m_mp = GetMaxMP(curTime);
}

void CUser::UpdateRecovery(uint32 curTime)
{
	// 누적 시간 증가
	m_recoveryInfo.HP_accumulatedTimeMs += FieldConst::UPDATE_LOOP_TIME;
	m_recoveryInfo.MP_accumulatedTimeMs += FieldConst::UPDATE_LOOP_TIME;

	if (m_recoveryInfo.HP_accumulatedTimeMs >= USER_HP_REGEN_TIME)
	{
		m_hp += GetHPRegenSec();

		uint16 maxhp = GetMaxHP(curTime);

		if (m_hp > maxhp)
			m_hp = maxhp;

		m_recoveryInfo.HP_accumulatedTimeMs = 0;
	}

	if (m_recoveryInfo.MP_accumulatedTimeMs >= USER_MP_REGEN_TIME)
	{
		m_mp += GetMPRegenSec();

		uint16 maxmp = GetMaxMP(curTime);

		if (m_hp > maxmp)
			m_hp = maxmp;

		m_recoveryInfo.MP_accumulatedTimeMs = 0;
	}

}

void CUser::Damage(uint16 damage)
{
	if (m_hp <= 0)
		return;

	m_hp -= damage;
	
	if (m_hp <= 0)
	{
		m_hp = 0;
	}
}

void CUser::UseSkill(uint32 curTime, uint8 skillIndex)
{
	if (skillIndex >= USER_SKILL_SLOT_COUNT || skillIndex < 0)
		return;

	if (skillIndex < USER_BUFF_SKILL_SLOT_COUNT)
	{
		m_skillInfo[skillIndex].m_skillActivate = true;
		m_skillInfo[skillIndex].m_skillLastRecvTime = curTime;
		m_skillInfo[skillIndex].m_skillExpiredTime = curTime + g_skillData[skillIndex].Duration;
		return;
	}

	m_skillInfo[skillIndex].m_skillLastRecvTime = curTime;
}

void CUser::CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector)
{
	m_secPos.CalSectorTransitionMessageTargets(oldSecPos, newSecPos, outDeleteSector, outCreateSector);
}

bool CUser::GetExp(uint64 GetExp, UserLevelStat& result)
{
	// 레벨 Max 찼으면 그냥 리턴
	if (m_level >= UserConst::USER_MAX_LEVEL)
		return false;

	// 경험치 올리기
	m_currentExp += GetExp;

	// 필요 경험치 덜 찼으면 그냥 리턴
	if (m_currentExp < m_requiredExp)
		return true;

	// 레벨업 했으면 해당 레벨에 해당하는 스탯으로 스탯 초기화
	m_level++;

	BaseStatInit(m_level);

	// 최종 스탯 result에 담기
	uint32 curTime = timeGetTime();
	int16 maxHP = GetMaxHP(curTime);
	int16 maxMP = GetMaxMP(curTime);

	result.atk = GetAtk(curTime);
	result.def = GetDef(curTime);
	result.maxHP = maxHP;
	result.maxMP = maxMP;
	result.hpRegenPerSec = GetHPRegenSec();
	result.mpRegenPerSec = GetMPRegenSec();
	result.level = m_level;

	m_currentExp = 0;
	m_hp = maxHP;
	m_mp = maxMP;
	return true;
}

bool CUser::CanUseSkill(uint32 curTime, uint8 skillIndex)
{
	if (skillIndex >= USER_SKILL_SLOT_COUNT )
		return false;

	// mp 및 쿨타임 체크
	if (m_mp < g_skillData[skillIndex].RequiredMana
		|| (g_skillData[skillIndex].CoolTime > curTime - m_skillInfo[skillIndex].m_skillLastRecvTime))
		return false;

	return true;
}

bool CUser::Move()
{
	if (m_moveFlag == false)
		return false;


	float rad = m_movementYaw * FieldConst::Pi / 180.0f;
	float dirX = cosf(rad);
	float dirY = sinf(rad);

	m_location.xpos += dirX * m_moveSpeed;
	m_location.ypos += dirY * m_moveSpeed;

	return true;
}

bool CUser::CanSwing(uint32 curTime, uint8 swingidx)
{
	if (m_swingInfo.m_lastSwingIdx == 0 || m_swingInfo.m_lastSwingIdx == 4)
	{
		m_swingInfo.m_lastSwingIdx = 1;
	}
	else
	{
		m_swingInfo.m_lastSwingIdx++;
	}

	// 유저 swingindex랑 패킷으로 받은 swingindex가 다르면 연결 끊기
	if (m_swingInfo.m_lastSwingIdx != swingidx)
		return false;

	m_swingInfo.m_lastSwingRecvTime = curTime;

	return true;
}

bool CUser::IsAlive()
{
	if (m_hp <= 0 || m_disconnectFlag == true)
		return false;

	return true;
}

// 필드에 있는 드랍 아이템 먹으려고 할 때 작동
bool CUser::GetConsumableItem(FieldDropItem& dropItem, PickUpConsumableResult& OutResult)
{
	// 넣을 갯수를 기준으로 우선 FullStack에 있는 UID 있으면 거기에 먼저 넣고 남은것 remainCount로 다시 받기
	uint16 remainCount = dropItem.count;

	if (!AddToExistingConsumableStack(dropItem.itemID, remainCount, OutResult))
		return false;

	// 다 넣었으면 true 리턴
	if (remainCount == 0)
		return true;

	return CreateNewConsumableStacks(dropItem, remainCount, OutResult);
}

bool CUser::GetEquipmentItem(FieldDropItem& dropItem, PickUpEquipResult& OutResult)
{
	// 여유 공간 없으면 false 리턴
	int16 ret = m_inventory.GetEmptySlotIndex();
	if (ret == -1)
		return false;

	// 여유분 있으면 Storage에 CreateItem
	ITEM_UID ID;
	if (!m_storage.CreateItem(dropItem, ID))
		return false;

	if (!m_inventory.InsertItemToSlot(ID, ret))
		return false;

	OutResult.itemID = dropItem.itemID;
	OutResult.slotIndex = ret;
	OutResult.count = dropItem.count;
	OutResult.randomStatCount = dropItem.randomStatCount;

	for (int i = 0; i < dropItem.randomStatCount; i++)
	{
		OutResult.randomStatResult[i].randomStatType = dropItem.randomStat[i].randomStatType;
		OutResult.randomStatResult[i].randomStatValue = dropItem.randomStat[i].randomStatValue;
	}

	return true;
}

// 아이템을 인벤토리 바깥으로 버릴 때 작동
bool CUser::DeleteItem(int16 slotIndex, SLOT_TYPE slotType)
{
	ITEM_UID retID;

	// 슬롯 타입에 따라 해당 위치에서 UID 제거
	switch (slotType)
	{
	case SLOT_TYPE::INVENTORY:
	{
		if (!m_inventory.DeleteInventorySlot(slotIndex, retID))
			return false;

		const UserItem* pItem = m_storage.FindItem(retID);
		if (pItem == nullptr)
			return false;

		// 장비 아이템 UID면 false
		m_inventory.DeleteNotFullStackItemUID(pItem->itemID, retID);

		break;
	}

	case SLOT_TYPE::EQUIPMENT:
	{
		if (!m_equipment.UnEquippedItem(static_cast<EQUIP_SLOT>(slotIndex), retID))
			return false;
		break;
	}

	case SLOT_TYPE::QUICKSLOT:
	{
		if (!m_quickSlot.ClearConsumable(slotIndex, retID))
			return false;

		break;
	}

	default:
		return false;
	}

	// Storage에서 제거(INVALID_ID면 fale 리턴됨)
	return m_storage.DeleteItem(retID);
}

// 해당 아이템 슬롯에 대해서 우클릭 할때 혹은 해당 버튼 누를 때 작동
bool CUser::UseInventoryItem(int16 slotIndex, UseItemResult& result)
{
	ITEM_UID retID;
	if (!m_inventory.GetItemUID(slotIndex, retID))
		return false;

	const UserItem* pUserData = m_storage.FindItem(retID);
	if (pUserData == nullptr)
		return false;

	const ItemData* pItemData = ItemTable::GetItemData(pUserData->itemID);
	if (pItemData == nullptr)
		return false;

	// 해당 아이템 타입이 Consumable이면 아이템 사용해서 적용 후 Count 0 이면 아이템 삭제
	if (pItemData->itemType == ITEM_TYPE::CONSUMABLE)
	{
		// 소모품 아이템 사용
		uint16 newItemCount = 0;
		if (!UseConsumableItem(pUserData, pItemData, newItemCount))
			return false;

		result.resultType = USE_ITEM_RESULT::CONSUME;
		result.consumableResult.newItenCount = newItemCount;
		result.consumableResult.slotIndex = slotIndex;

		// 사용 후 카운트가 0이 아니면 지울 필요없으니 리턴
		if (newItemCount != 0)
			return true;


		// 다 사용했으면 아이템 삭제
		ITEM_UID retUID;
		m_inventory.DeleteInventorySlot(slotIndex, retUID);
		m_inventory.DeleteNotFullStackItemUID(pUserData->itemID, retUID);

		return m_storage.DeleteItem(retUID);
	}

	// 장비 아이템이면 해당 슬롯에 장착 하기
	else if (pItemData->itemType == ITEM_TYPE::EQUIPMENT)
		return EquippedItem(slotIndex, result);


	return false;
}

// 해당 장비 슬롯에 대해서 우클릭 하면 장비 해제
bool CUser::UseEquipmentItem(int16 slotIndex, UseItemResult& result)
{
	// 해당 슬롯에 대한 UID 얻는데 없으면 그냥 false 리턴
	ITEM_UID retID = m_equipment.GetEquippedItem(static_cast<EQUIP_SLOT>(slotIndex));
	if (retID == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	// 만약 장비가 있다면 장착 해제

	// 장비 장착 해제 시 인벤토리 슬롯 여유분 확인
	int16 emptyIdx = m_inventory.GetEmptySlotIndex();
	if (emptyIdx == -1)
		return false;

	// 인벤토리 여유 있으면 기존 장비 탭에서 빼고 인벤토리에 삽입
	ITEM_UID UID;
	if (!m_equipment.UnEquippedItem(static_cast<EQUIP_SLOT>(slotIndex), UID))
		return false;

	if (!m_inventory.InsertItemToSlot(UID, emptyIdx))
		return false;

	result.resultType = USE_ITEM_RESULT::UNEQUIP;
	result.unEquipResult.inventorySlotIdx = emptyIdx;
	return true;
}

// 퀵 슬롯에서 해당 버튼 누르면 해당 소모품 사용
bool CUser::UseQuickSlotItem(int16 slotIndex, UseItemResult& result)
{
	ITEM_UID retUID = m_quickSlot.GetQuickSlotItem(slotIndex);
	if(retUID == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	// storage에서 해당 아이템 찾기
	const UserItem* pUserItem = m_storage.FindItem(retUID);
	if (pUserItem == nullptr)
		return false;

	const ItemData* pItem = ItemTable::GetItemData(pUserItem->itemID);
	if (pItem == nullptr)
		return false;

	// 소모 아이템 사용
	uint16 newItemCount = 0;
	if (!UseConsumableItem(pUserItem, pItem, newItemCount))
		return false;

	result.resultType = USE_ITEM_RESULT::CONSUME;
	result.consumableResult.slotType = SLOT_TYPE::QUICKSLOT;
	result.consumableResult.slotIndex = slotIndex;
	result.consumableResult.newItenCount = newItemCount;

	// 만약 퀵슬롯의 아이템 다 썼으면 퀵슬롯 및 storage에서 제거
	if (newItemCount != 0)
		return true;

	ITEM_UID eraseUID;
	if (!m_quickSlot.ClearConsumable(slotIndex, eraseUID))
		return false;

	if (!m_storage.DeleteItem(eraseUID))
		return false;

	return true;
}

// 내 아이템 슬롯 위치 바꿀 때 작동
bool CUser::ItemSlotChange(SLOT_TYPE fromType, int16 fromIndex, SLOT_TYPE toType, int16 toIndex)
{
	// 슬롯 타입 범위 체크
	if ((!SlotTypeRangeCheck(fromType)) || (!SlotTypeRangeCheck(toType)))
		return false;

	// from 타입과 to 타입이 같으면 from 타입만 체크해서 인벤토리면 인벤토리 이동, 퀵 슬롯이면 퀵슬롯 이동
	if (fromType == toType)
	{
		switch (fromType)
		{
		case SLOT_TYPE::INVENTORY:
			return m_inventory.ItemSlotChange(fromIndex, toIndex);

		case SLOT_TYPE::QUICKSLOT:
			return m_quickSlot.SwapSlot(fromIndex, toIndex);

		case SLOT_TYPE::EQUIPMENT:
			return false; // 장비 슬롯끼리 이동 금지

		default:
			return false;
		}
	}

	// 인벤토리에서 장비 슬롯으로 이동
	else if (fromType == SLOT_TYPE::INVENTORY && toType == SLOT_TYPE::EQUIPMENT)
		return SwapInventoryEquipment(fromIndex, static_cast<EQUIP_SLOT>(toIndex));

	// 장비 슬롯에서 인벤토리 이동
	else if (fromType == SLOT_TYPE::EQUIPMENT && toType == SLOT_TYPE::INVENTORY)
		return SwapInventoryEquipment(toIndex, static_cast<EQUIP_SLOT>(fromIndex));

	// 인벤토리에서 퀵 슬롯 이동
	else if (fromType == SLOT_TYPE::INVENTORY && toType == SLOT_TYPE::QUICKSLOT)
		return SwapInventoryQuickSlot(fromIndex, toIndex);

	// 퀵 슬롯에서 인벤토리 이동
	else if (fromType == SLOT_TYPE::QUICKSLOT && toType == SLOT_TYPE::INVENTORY)
		return SwapInventoryQuickSlot(toIndex, fromIndex);


	// 그 외의 이동은 불가
	return false;
}

uint32 CUser::CalSkillDamage(uint16 skillIndex, CUser* target, uint32 curTime)
{
	if (skillIndex >= USER_SKILL_SLOT_COUNT || target == nullptr || !target->IsAlive())
		return 0;

	const SkillData& skillData = g_skillData[skillIndex];

	uint16 atk = GetAtk(curTime);
	uint32 damage = skillData.BaseDamage + static_cast<uint32>(atk * skillData.AttackRatio);
	
	switch (skillData.DamageType)
	{
	case ESkillDamageType::Physical:
	{
		uint16 targetDef = target->GetDef(curTime);

		// 데미지 낮아도 1딜 들어감.
		if (damage <= targetDef)
			damage = 1;
		else
			damage -= targetDef;

		break;
	}

	case ESkillDamageType::Magic:
	{
		uint16 targetDef = target->GetDef(curTime);

		if (damage <= targetDef)
			damage = 1;
		else
			damage -= targetDef;

		break;
	}

	case ESkillDamageType::TrueDamage:
		// 방어력 무시
		break;
	}

	return damage;
}

uint32 CUser::CalSkillDamage(uint16 skillIndex, CMonster* target, uint32 curTime)
{
	if (skillIndex >= USER_SKILL_SLOT_COUNT || target == nullptr || target->GetMonsterState() == EMonsterState::Dead)
		return 0;

	const SkillData& skillData = g_skillData[skillIndex];

	uint16 atk = GetAtk(curTime);
	uint32 damage = skillData.BaseDamage + static_cast<uint32>(atk * skillData.AttackRatio);

	switch (skillData.DamageType)
	{
	case ESkillDamageType::Physical:
	{
		uint16 targetDef = target->GetDef();

		// 데미지 낮아도 1딜 들어감.
		if (damage <= targetDef)
			damage = 1;
		else
			damage -= targetDef;

		break;
	}

	case ESkillDamageType::Magic:
	{
		uint16 targetDef = target->GetDef();

		if (damage <= targetDef)
			damage = 1;
		else
			damage -= targetDef;

		break;
	}

	case ESkillDamageType::TrueDamage:
		// 방어력 무시
		break;
	}

	return damage;
}

uint32 CUser::CalBaseAttackDamage(CUser* target, uint32 curTime)
{
	if (target == nullptr || !target->IsAlive())
		return 0;

	uint16 atk = GetAtk(curTime);

	// if swing index마다 데미지 배율 다르게 하고 싶으면 ratio 수정

	float ratio = 1.0f;

	uint32 damage = static_cast<uint32>(atk * ratio);
	uint16 targetDef = target->GetDef(curTime);

	if (damage <= targetDef)
		return 1;

	return damage - targetDef;
}

uint32 CUser::CalBaseAttackDamage(CMonster* target, uint32 curTime)
{
	if (target == nullptr || target->GetMonsterState() == EMonsterState::Dead)
		return 0;

	uint16 atk = GetAtk(curTime);

	// if swing index마다 데미지 배율 다르게 하고 싶으면 ratio 수정

	float ratio = 1.0f;

	uint32 damage = static_cast<uint32>(atk * ratio);
	uint16 targetDef = target->GetDef();

	if (damage <= targetDef)
		return 1;

	return damage - targetDef;
}

uint16 CUser::GetDef(uint32 curTime)
{
	uint16 def = m_baseStat.m_def + m_equipment.GetDEF();

	// 버프 유효성 체크
	// 버프 아직 켜져있으면서 만료시간이 안되었으면 def 증가

	for (int i = 0; i < USER_BUFF_SKILL_SLOT_COUNT; i++)
	{
		if (m_skillInfo[i].m_skillActivate && m_skillInfo[i].m_skillExpiredTime > curTime)
			def += ClientAttack::BUFF_DEF_ADD_AMOUNT;
	}

	// 타 버프/디버프 스킬 유효성 체크

	return def;
}

uint16 CUser::GetAtk(uint32 curTime)
{
	uint16 atk = m_baseStat.m_atk + m_equipment.GetATK();

	// 버프 유효성 체크
	// 버프 아직 켜져있으면서 만료시간이 안되었으면 def 증가

	for (int i = 0; i < USER_BUFF_SKILL_SLOT_COUNT; i++)
	{
		if (m_skillInfo[i].m_skillActivate && m_skillInfo[i].m_skillExpiredTime > curTime)
			atk += ClientAttack::BUFF_ATK_ADD_AMOUNT;
	}

	// 타 버프/디버프 스킬 유효성 체크

	return atk;
}

uint16 CUser::GetMaxHP(uint32 curTime)
{
	uint16 maxhp = m_baseStat.m_maxHP + m_equipment.GetMaxHP();

	return maxhp;
}

uint16 CUser::GetMaxMP(uint32 curTime)
{
	uint16 maxmp = m_baseStat.m_maxMP + m_equipment.GetMaxMP();

	return maxmp;
}

uint16 CUser::GetHPRegenSec() const
{
	uint16 HPRegenSec = m_baseStat.m_hpRegenPerSec + m_equipment.GetHPRegen();

	return HPRegenSec;
}

uint16 CUser::GetMPRegenSec() const
{
	uint16 MPRegenSec = m_baseStat.m_mpRegenPerSec + m_equipment.GetMPRegen();

	return MPRegenSec;
}

CUser* CUser::Alloc()
{
	return m_userPool.Alloc();
}

void CUser::Free(CUser* pUser)
{
	m_userPool.Free(pUser);
}

void CUser::SkillInfoInit()
{
	for (int i = 0; i < USER_SKILL_SLOT_COUNT; i++)
	{
		m_skillInfo[i].m_skillActivate = false;
		m_skillInfo[i].m_skillExpiredTime = 0;
		m_skillInfo[i].m_skillLastRecvTime = 0;
	}
}

void CUser::BaseStatInit(uint16 level)
{
	m_baseStat.m_atk = g_userLevelStatTable[level - 1].atk;
	m_baseStat.m_def = g_userLevelStatTable[level - 1].def;
	m_baseStat.m_maxHP = g_userLevelStatTable[level - 1].maxHP;
	m_baseStat.m_maxMP = g_userLevelStatTable[level - 1].maxMP;
	m_baseStat.m_hpRegenPerSec = g_userLevelStatTable[level - 1].hpRegenPerSec;
	m_baseStat.m_mpRegenPerSec = g_userLevelStatTable[level - 1].mpRegenPerSec;
	m_requiredExp = g_userLevelStatTable[level - 1].requiredExp;
}

bool CUser::UseConsumableItem(const UserItem* pUserItem, const ItemData* pItemData, uint16& newItemCount)
{
	// 쿨타임 다 된건지 체크
	uint32 curTime = timeGetTime();
	CONSUMABLE_ITEM_TYPE consumeType = pItemData->consumableType;
	if (!CanUseConsumalbIetem(curTime, consumeType))
		return false;

	// 쿨타임 지났으면 사용처리

	// 소모품 사용 통한 효과 처리
	switch (consumeType)
	{
	case CONSUMABLE_ITEM_TYPE::SMALL_HP_POTION:
		uint16 maxHP = GetMaxHP(curTime);
		m_hp += pItemData->recoverHP;
		if (m_hp > maxHP)
			m_hp = maxHP;
		m_consumableCooltimeInfo.CooltimeStartTimeMs[(int)consumeType] = curTime;
		break;

	case CONSUMABLE_ITEM_TYPE::SMALL_MP_POTION:
		uint16 maxMP = GetMaxMP(curTime);
		m_mp += pItemData->recoverMP;
		if (m_mp > maxMP)
			m_mp = maxMP;
		m_consumableCooltimeInfo.CooltimeStartTimeMs[(int)consumeType] = curTime;
		break;
	}

	// 아이템 카운트 감소
	uint16 newCount = pUserItem->count;
	if (!m_storage.ChangeItemCount(pUserItem->itemUID, --newCount))
		return false;

	newItemCount = newCount;
	return true;
}

bool CUser::CanUseConsumalbIetem(uint32 curTime, CONSUMABLE_ITEM_TYPE itemType)
{
	if (curTime - m_consumableCooltimeInfo.CooltimeStartTimeMs[(int)itemType]
		>= m_consumableCooltimeInfo.CoolTimeMs[(int)itemType])
		return true;

	return false;
}

// 해당 인벤토리 슬롯에 있는 장비 장착하려고 할 때 작동(인벤토리에 장비 우클릭시 작동)
bool CUser::EquippedItem(int16 inventorySlotIndex, UseItemResult& result)
{
	// UID 획득 실패 했으면 false 리턴
	ITEM_UID retID;
	if (!m_inventory.GetItemUID(inventorySlotIndex, retID))
		return false;

	// 해당 슬롯의 아이템 타입 체크 Consumable을 false 리턴
	const UserItem* pUserItem = m_storage.FindItem(retID);
	if (pUserItem == nullptr)
		return false;

	// 아이템 테이블에 애초에 없는 ID면 서버 문제
	const ItemData* pItemData = ItemTable::GetItemData(pUserItem->itemID);
	if (pItemData == nullptr)
		return false;

	// 타입이 장비 타입 아니면 false 리턴
    if (pItemData->itemType != ITEM_TYPE::EQUIPMENT)
		return false;

	// 먼저 인벤토리 슬롯에서 제거
	ITEM_UID removedUID;
	if (!m_inventory.DeleteInventorySlot(inventorySlotIndex, removedUID))
		return false;


	// 해당 아이템의 EQUIP_SLOT 체크 해서 해당 장비 슬롯에 장비 장착
	ITEM_UID OutEquipItem;
	if (!m_equipment.EquippedItem(pItemData->equipSlot, removedUID, OutEquipItem))
		return false;

	uint8 slotUpdateCount = result.equipResult.updateSlotCount;
	result.resultType = USE_ITEM_RESULT::EQUIP;
	result.equipResult.resultSlot[slotUpdateCount].slotType = SLOT_TYPE::EQUIPMENT;
	result.equipResult.resultSlot[slotUpdateCount].slotIndex = static_cast<int16>(pItemData->equipSlot);
	result.equipResult.resultSlot[slotUpdateCount].slotState = SLOT_STATE::EXIST;
	result.equipResult.resultSlot[slotUpdateCount].itemID = pItemData->itemID;
	slotUpdateCount++;

	result.equipResult.resultSlot[slotUpdateCount].slotType = SLOT_TYPE::INVENTORY;
	result.equipResult.resultSlot[slotUpdateCount].slotIndex = inventorySlotIndex;
	result.equipResult.resultSlot[slotUpdateCount].slotState = SLOT_STATE::EMPTY;

	// 해당 장비 슬롯에 장착된 장비가 없으면 그냥 리턴
	if (OutEquipItem == ItemUID::ITEM_UID_INVALID_ID)
	{
		result.equipResult.updateSlotCount = slotUpdateCount;
		return true;
	}

	// 있으면 inventoryindex로 기존에 장착하던 장비 넣기(뺐는데 못넣는거는 서버 문제)
	if (!m_inventory.InsertItemToSlot(OutEquipItem, inventorySlotIndex))
		__debugbreak();

	const UserItem* pOutEquipItem = m_storage.FindItem(OutEquipItem);
	if (pOutEquipItem == nullptr)
		__debugbreak();

	result.equipResult.resultSlot[slotUpdateCount].slotState = SLOT_STATE::EXIST;
	result.equipResult.resultSlot[slotUpdateCount].itemID = pOutEquipItem->itemID;
	result.equipResult.updateSlotCount = slotUpdateCount;
	return true;
}

// 해당 장비 슬롯에 있는 장비 해제하려고 할 때 작동
bool CUser::UnEquippedItem(EQUIP_SLOT equipSlot)
{
	// 인벤토리에 여유 공간 있는지 확인 없으면 false
	int16 emptyIdx = m_inventory.GetEmptySlotIndex();
	if (emptyIdx == -1)
		return false;

	// 장비 해제 실패하면 false 리턴
	ITEM_UID retUID;
	if (!m_equipment.UnEquippedItem(equipSlot, retUID))
		return false;

	// 해제 성공했는데 UID가 INVALID면 false 리턴
	if (retUID == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	// 있으면 해당 슬롯에 옮기기
	return m_inventory.InsertItemToSlot(retUID, emptyIdx);
}

bool CUser::AddToExistingConsumableStack(ITEM_ID ItemID, uint16& RemainCount,PickUpConsumableResult& OutResult)
{
	while (true)
	{
		ITEM_UID retID;
		m_inventory.FindNotFullStackItemUID(ItemID, retID);

		// 애초에 인벤토리에 해당 소비품에 대한 UID가 없으면 리턴해서 새롭게 인벤토리에 생성하기
		if (retID == ItemUID::ITEM_UID_INVALID_ID)
			return true;

		const UserItem* pUserItem = m_storage.FindItem(retID);
		uint32 retCount = pUserItem->count;
		if (retCount == 0)
			return false;

		// 있으면 retCount에 수량 더하기 이게 stack 넘어가면 새로운 아이템 생성
		retCount += RemainCount;

		const ItemData* pItemData = ItemTable::GetItemData(pUserItem->itemID);
		if (pItemData == nullptr)
			return false;

		OutResult.itemID = pItemData->itemID;

		// 현재 증가시킨 갯수가 Max에 도달 안했으면 그냥 반영하고 리턴
		uint16 maxStack = pItemData->maxStack;

		if (retCount < maxStack)
		{
			RemainCount = 0;

			if (!m_storage.ChangeItemCount(retID, retCount))
				return false;

			OutResult.consumableResult[OutResult.updateSlotCount].slotIndex = m_inventory.GetUIDToSlotIndex(retID);
			OutResult.consumableResult[OutResult.updateSlotCount].newItemCount = retCount;
			OutResult.updateSlotCount++;
			return true;
		}

		// 만약 도달 했으면 Storage에 반영하고 FullStack 자료구조에서 제거하고 결과에 반영해주고 리턴
		else if (retCount == maxStack)
		{
			RemainCount = 0;
			if (!m_storage.ChangeItemCount(retID, retCount))
				return false;

			if (!m_inventory.DeleteNotFullStackItemUID(pUserItem->itemID, retID))
				return false;

			OutResult.consumableResult[OutResult.updateSlotCount].slotIndex = m_inventory.GetUIDToSlotIndex(retID);
			OutResult.consumableResult[OutResult.updateSlotCount].newItemCount = retCount;
			OutResult.updateSlotCount++;
			return true;
		}

		// Stack에 다 채웠고 남았으면 Remain 갱신 후 다시 FullStack 확인해서 소모품 UID 찾기
		OutResult.consumableResult[OutResult.updateSlotCount].slotIndex = m_inventory.GetUIDToSlotIndex(retID);
		OutResult.consumableResult[OutResult.updateSlotCount].newItemCount = maxStack;
		OutResult.updateSlotCount++;

		m_storage.ChangeItemCount(retID, maxStack);
		m_inventory.DeleteNotFullStackItemUID(pUserItem->itemID, retID);
		RemainCount = retCount - maxStack;
	}

	return false;
}

bool CUser::CreateNewConsumableStacks(FieldDropItem& dropItem, uint16& RemainCount, PickUpConsumableResult& OutResult)
{
	// RemainCount를 인벤토리 및 Storage에 생성
	const ItemData* pItemData = ItemTable::GetItemData(dropItem.itemID);
	if (pItemData == nullptr)
		return false;

	// 기본으로 생성할 1개 + 남은 갯수가 해당 아이템의 stack 갯수보다 엄청 많은 경우도 고려해야 해서 몫을 더해줌.
	uint16 needSlotCount = (RemainCount + pItemData->maxStack - 1) / pItemData->maxStack;

	for (int i = 0; i < needSlotCount; i++)
	{
		int16 inventoryIdx = m_inventory.GetEmptySlotIndex();
		if (inventoryIdx == -1)
			return false;

		// 여유 슬롯이 있으면 아이템 생성하기
		uint16 stackCount = min(RemainCount, pItemData->maxStack);
		dropItem.count = stackCount;

		ITEM_UID retID;
		if (!m_storage.CreateItem(dropItem, retID))
			return false;

		// 성공하면 인벤토리에 배치
		if (!m_inventory.InsertItemToSlot(retID, inventoryIdx))
			return false;

		OutResult.itemID = pItemData->itemID;
		OutResult.consumableResult[OutResult.updateSlotCount].slotIndex = inventoryIdx;
		OutResult.consumableResult[OutResult.updateSlotCount].newItemCount = stackCount;
		OutResult.updateSlotCount++;

		RemainCount -= stackCount;

		if (stackCount < pItemData->maxStack)
		{
			m_inventory.InsertNotFullStackItemUID(dropItem.itemID, retID);
		}
	}

	return true;
}

bool CUser::SlotTypeRangeCheck(SLOT_TYPE type)
{
	if ((int)type < (int)SLOT_TYPE::INVENTORY || (int)type > (int)SLOT_TYPE::QUICKSLOT )
		return false;

	return true;
}

bool CUser::SwapInventoryEquipment(int16 inventoryIndex, EQUIP_SLOT equipSlot)
{
	// 해당 인벤토리 및 장비 슬롯 위치에 있는 아이템 UID가 하나라도 Invalid면 false 리턴
	ITEM_UID inventoryUID;
	ITEM_UID equipmentUID;
	if (!m_inventory.GetItemUID(inventoryIndex, inventoryUID))
		return false;

	// 장착 중이던 장비가 없으면 false 리턴
	equipmentUID = m_equipment.GetEquippedItem(equipSlot);
	if (equipmentUID == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	// 인벤토리에 있는 아이템 UID 체크 장비가 아니면 false 리턴
	const UserItem* pInventoryItem = m_storage.FindItem(inventoryUID);
	if (pInventoryItem == nullptr)
		return false;

	const ItemData* pInventoryItemData = ItemTable::GetItemData(pInventoryItem->itemID);
	if (pInventoryItemData == nullptr)
		return false;

	// 인벤토리에 있는 아이템 UID 체크 장비가 아니면 false 리턴
	if (pInventoryItemData->itemType != ITEM_TYPE::EQUIPMENT)
		return false;

	// 인벤토리 아이템이 장비의 슬롯과 이동 시킬 장비 탭의 슬롯이 일치 하지 않으면 false리턴
	if (pInventoryItemData->equipSlot != equipSlot)
		return false;

	// 장착 중이던 장비를 기존 인벤토리 위치로 이동 시키고 기존 인벤토리에 있는 장비를 장비 탭으로 이동
	m_inventory.DeleteInventorySlot(inventoryIndex, inventoryUID);
	if (!m_inventory.InsertItemToSlot(equipmentUID, inventoryIndex))
		return false;

	if (!m_equipment.EquippedItem(equipSlot, inventoryUID, equipmentUID))
		return false;

	return true;
}

bool CUser::SwapInventoryQuickSlot(int16 inventoryIndex, int16 quickSlotIndex)
{
	// 인벤토리나 퀵슬롯에 있는 아이템 UID가 Invalid면 false 리턴
	ITEM_UID inventoryUID;
	ITEM_UID quickSlotUID;

	if (!m_inventory.GetItemUID(inventoryIndex, inventoryUID))
		return false;

	quickSlotUID = m_quickSlot.GetQuickSlotItem(quickSlotIndex);
	if (quickSlotUID == ItemUID::ITEM_UID_INVALID_ID)
		return false;

	const UserItem* pInventoryItem = m_storage.FindItem(inventoryUID);
	if (pInventoryItem == nullptr)
		return false;

	const ItemData* pInventoryItemData = ItemTable::GetItemData(pInventoryItem->itemID);
	if (pInventoryItemData == nullptr)
		return false;

	// 인벤토리 위치에 있는 아이템이 소모품이 아니면 false 리턴
	if (pInventoryItemData->itemType != ITEM_TYPE::CONSUMABLE)
		return false;

	// 기존 인벤토리 및 퀵슬롯에서 아이템 제거
	m_inventory.DeleteInventorySlot(inventoryIndex, inventoryUID);
	m_quickSlot.ClearConsumable(quickSlotIndex, quickSlotUID);

	// 기존 인벤토리 UID를 퀵슬롯으로 이동
	m_quickSlot.SetConsumable(quickSlotIndex, inventoryUID, quickSlotUID);

	// 퀵슬롯에 있던 UID를 인벤토리로 이동
	m_inventory.InsertItemToSlot(quickSlotUID, inventoryIndex);

	const UserItem* pQuickSlotItem = m_storage.FindItem(quickSlotUID);
	if (pQuickSlotItem == nullptr)
		return false;

	const ItemData* pQuickSlotItemData = ItemTable::GetItemData(pQuickSlotItem->itemID);
	if (pQuickSlotItemData == nullptr)
		return false;

	// 퀵 슬롯 아이템의 Count가 MaxStack 아래면 FullStack 자료구조에 넣기
	if (pQuickSlotItem->count < pQuickSlotItemData->maxStack)
	{
		m_inventory.InsertNotFullStackItemUID(pQuickSlotItem->itemID, quickSlotUID);
	}

	// 기존에 있던 인벤토리 소모품은 FullStack에서 제거
	m_inventory.DeleteNotFullStackItemUID(pInventoryItem->itemID, inventoryUID);

	return true;
}

