#include <string>
#include <windows.h>
#include <unordered_map>
#include <array>
#include <set>
#include <vector>
#include <mysql.h>
#include <thread>
#include "ContentsEnum.h"
#include "SkillTable.h"
#include "CMonster.h"
#include "Inventory.h"
#include "Equipment.h"
#include "QuickSlot.h"
#include "CUserItemStorage.h"
#include "ItemTable.h"
#include "FieldDropItemPool.h"
#include "DBTLS.h"
#include "DBJob.h"
#include "CDBManager.h"
#include "IUser.h"
#include "CUser.h"

CMPoolTLS<CUser> CUser::m_userPool;

using namespace UserConst;

void CUser::Init(uint64 sessionID, CDBManager* pDBManager)
{
	m_sessionID = sessionID;
	m_pDBManager = pDBManager;
}

void CUser::Destroy()
{
	//// todo : DB에 현재 위치 저장
	//LogOutJob* pJob = new LogOutJob;
	//pJob->characterUID = m_characterUID;
	//pJob->location = m_location;
	//m_pDBManager->EnqueueDBJob(pJob);


	m_equipment.Destroy();
	m_inventory.Destroy();
	m_quickSlot.Destroy();
	m_storage.Destroy();
}

void CUser::ResPawn()
{
	
}

void CUser::LoadDataFromDB(uint64 characterUID, uint64 accountID, uint16 level, int32 curExp, Location& location, std::vector<ItemLoadData>& items)
{
	m_inventory.Init(&m_storage);
	m_equipment.Init(&m_storage);
	m_storage.Init(m_pDBManager);

	m_itemSlotUpdateTime = USER_ITEM_SLOT_UPDATE_MIN_TIME + rand() % (USER_ITEM_SLOT_UPDATE_MAX_TIME - USER_ITEM_SLOT_UPDATE_MIN_TIME);
	m_itemSlotUpdateTimeAccum = 0;

	// 아이템 인벤토리 삽입
	// 아이템 장비 탭 삽입
	// 아이템 퀵슬롯 삽입
	// 저장소에 넣기
	std::vector<ItemLoadData>::iterator it = items.begin();
	for (; it != items.end(); ++it)
	{
		ItemLoadData& item = *it;
		
		// 로드한 데이터 저장소에 넣기. 그래야 장비 넣을 때 저장소에서 찾아서 처리 가능함.
		m_storage.LoadItemFromDB(item);

		// 아이템 타입 판단
		switch (item.slotType)
		{
		case SLOT_TYPE::INVENTORY:
			// 해당 index에 인벤토리 아이템
			InventoryItemLoad(item);
			break;

		case SLOT_TYPE::EQUIPMENT:
			EquipmentItemLoad(item);
			break;

		case SLOT_TYPE::QUICKSLOT:
			QuickSlotItemLoad(item);
			break;
		}

	}


	SkillInfoInit();

	// 리커버리 정보 초기화
	m_recoveryInfo.Init();

	// 소비품 쿨타임 초기화
	m_consumableCooltimeInfo.Init();

	m_characterUID = characterUID;
	m_accountID = accountID;
	m_level = level;
	m_currentExp = curExp;
	m_location = location;
	m_moveFlag = false;
	m_movementYaw = 0.0f;
	m_maxWalkSpeed = WALK_SPEED;
	m_moveSpeed = m_maxWalkSpeed / FieldConst::UPDATE_FRAME;

	// 섹터쪽 변수 초기화
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
	m_arrayIdx = 0;

	// 레벨에 따른 초기화
	BaseStatInit(m_level);

	// 레벨 및 장비 세팅 다 하고 나서 hp, mp 설정
	uint32 curTime = timeGetTime();
	m_hp = GetMaxHP(curTime);
	m_mp = GetMaxMP(curTime);

	m_recvTime = timeGetTime();
	m_lastSyncCheckTime = timeGetTime();
	m_syncCount = 0;
}

void CUser::UpdateRecovery(uint32 curTime)
{
	// 누적 시간 증가
	m_recoveryInfo.HP_accumulatedTimeMs += FieldConst::UPDATE_LOOP_TIME;
	m_recoveryInfo.MP_accumulatedTimeMs += FieldConst::UPDATE_LOOP_TIME;

	if (m_recoveryInfo.HP_accumulatedTimeMs >= USER_HP_REGEN_TIME * 1000)
	{
		m_hp += GetHPRegenSec();

		int16 maxhp = GetMaxHP(curTime);

		if (m_hp > maxhp)
			m_hp = maxhp;

		m_recoveryInfo.HP_accumulatedTimeMs = 0;
	}

	if (m_recoveryInfo.MP_accumulatedTimeMs >= USER_MP_REGEN_TIME * 1000)
	{
		m_mp += GetMPRegenSec();

		uint16 maxmp = GetMaxMP(curTime);

		if (m_mp > maxmp)
			m_mp = maxmp;

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
		m_mp -= g_skillData[skillIndex].RequiredMana;
		if (m_mp < 0)
			__debugbreak();

		return;
	}

	m_skillInfo[skillIndex].m_skillLastRecvTime = curTime;
	m_mp -= g_skillData[skillIndex].RequiredMana;
	if (m_mp < 0)
		__debugbreak();
}

void CUser::CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector)
{
	m_secPos.CalSectorTransitionMessageTargets(oldSecPos, newSecPos, outDeleteSector, outCreateSector);
}

bool CUser::UserOnUpdate(uint32 curTime)
{
	if (!IsAlive())
		return false;

	UpdateRecovery(curTime);

	// 아이템 슬롯 업데이트 체크
	ItemSlotUpdate();

	// 섹터 변경은 FieldGroup이 Move가 성공하면 그때 할 것임.
	return Move();
}

bool CUser::GainExp(uint32 GetExp, GainEXPResult& result)
{
	// 레벨 Max 찼으면 그냥 리턴
	if (m_level >= UserConst::USER_MAX_LEVEL)
		return false;

	// 경험치 올리기
	m_currentExp += GetExp;

	CharacterProgressJob* pJob = new CharacterProgressJob;
	pJob->characterUID = m_characterUID;

	// 필요 경험치 덜 찼으면 그냥 리턴
	if (m_currentExp < m_requiredExp)
	{
		result.levelUp = false;
		result.curEXP = m_currentExp;
		pJob->curEXP = m_currentExp;
		pJob->level = m_level;

		m_pDBManager->EnqueueDBJob(pJob);
		return true;
	}

	result.levelUp = true;

	// 레벨업 했을 때 초과분에 대한 처리
	while (true)
	{
		m_currentExp -= m_requiredExp;
		m_level++;

		BaseStatInit(m_level);

		// 레벨업에 대한 스탯 초기화 후 해당 레벨이 Max면 hp, mp 초기화 후 curExp = 0 후 탈출
		// 잔여 경험치가 필요 경험치보다 작으면 반영 후 탈출
		if (m_level == UserConst::USER_MAX_LEVEL || m_requiredExp > m_currentExp)
		{
			uint32 curTime = timeGetTime();
			int16 maxHP = GetMaxHP(curTime);
			int16 maxMP = GetMaxMP(curTime);

			m_hp = maxHP;
			m_mp = maxMP;

			result.curLevel = m_level;
			result.curHP = m_hp;
			result.curMP = m_mp;

			if (m_level == UserConst::USER_MAX_LEVEL)
				result.curEXP = 0;
			else
				result.curEXP = m_currentExp;

			break;
		}
	}

	pJob->level = m_level;
	pJob->curEXP = m_currentExp;
	m_pDBManager->EnqueueDBJob(pJob);
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

bool CUser::IsAlive()
{
	if (m_hp <= 0)
		return false;

	return true;
}

// 필드에 있는 드랍 아이템 먹으려고 할 때 작동
bool CUser::GetConsumableItem(FieldDropItem& dropItem, PickUpConsumableResult& OutResult)
{
	// 그냥 빈 슬롯 찾아서 거기에 아이템 넣기
	int16 emptyIndex = m_inventory.GainEmptySlotIndex();
	if (emptyIndex == -1)
		return false;

	// 슬롯 있으면 거기에 아이템 넣기
	ITEM_UID retUID;
	if (!m_storage.CreateItem(dropItem, retUID))
	{
		// 저장소 가득 찼으면 false 리턴 
		m_inventory.ReturnSlotIndex(emptyIndex);
		return false;
	}

	// 해당 UID를 인벤토리에 배치
	m_inventory.InsertItemToSlot(retUID, emptyIndex);

	OutResult.itemID = dropItem.itemID;
	OutResult.consumableResult.slotIndex = emptyIndex;
	OutResult.consumableResult.newItemCount = dropItem.count;

	InsertItemJob* pJob = new InsertItemJob;
	pJob->characterUID = m_characterUID;
	pJob->itemUID = retUID;
	pJob->itemID = dropItem.itemID;
	pJob->count = dropItem.count;
	pJob->slotType = SLOT_TYPE::INVENTORY;
	pJob->slotIndex = emptyIndex;
	pJob->itemStat = {};

	for (int i = 0; i < dropItem.randomStatCount; i++)
	{
		RandomStatResult& stat = dropItem.randomStat[i];

		switch (stat.randomStatType)
		{
		case RANDOM_STAT_TYPE::ATK: pJob->itemStat.atk = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::DEF: pJob->itemStat.def = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::MAX_HP: pJob->itemStat.atk = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::MAX_MP: pJob->itemStat.def = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::HP_REGEN: pJob->itemStat.hpRegenPerSec = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::MP_REGEN: pJob->itemStat.mpRegenPerSec = stat.randomStatValue; break;
		default:
			__debugbreak();
		}
	}

	m_pDBManager->EnqueueDBJob(pJob);


	return true;
}

bool CUser::GetEquipmentItem(FieldDropItem& dropItem, PickUpEquipResult& OutResult)
{
	// 여유 공간 없으면 false 리턴
	int16 ret = m_inventory.GainEmptySlotIndex();
	if (ret == -1)
		return false;

	// 여유분 있으면 Storage에 CreateItem
	ITEM_UID ID;
	if (!m_storage.CreateItem(dropItem, ID))
	{
		// 저장소 가득 찼으면 리턴
		m_inventory.ReturnSlotIndex(ret);
		return false;
	}

	if (!m_inventory.InsertItemToSlot(ID, ret))
	{
		m_inventory.ReturnSlotIndex(ret);
		return false;
	}
	OutResult.itemID = dropItem.itemID;
	OutResult.slotIndex = ret;
	OutResult.count = dropItem.count;
	OutResult.randomStatCount = dropItem.randomStatCount;

	for (int i = 0; i < dropItem.randomStatCount; i++)
	{
		OutResult.randomStatResult[i].randomStatType = dropItem.randomStat[i].randomStatType;
		OutResult.randomStatResult[i].randomStatValue = dropItem.randomStat[i].randomStatValue;
	}


	InsertItemJob* pJob = new InsertItemJob;
	pJob->characterUID = m_characterUID;
	pJob->itemUID = ID;
	pJob->itemID = dropItem.itemID;
	pJob->count = dropItem.count;
	pJob->slotType = SLOT_TYPE::INVENTORY;
	pJob->slotIndex = ret;
	pJob->itemStat = {};

	for (int i = 0; i < dropItem.randomStatCount; i++)
	{
		RandomStatResult& stat = dropItem.randomStat[i];

		switch (stat.randomStatType)
		{
		case RANDOM_STAT_TYPE::ATK: pJob->itemStat.atk = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::DEF: pJob->itemStat.def = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::MAX_HP: pJob->itemStat.atk = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::MAX_MP: pJob->itemStat.def = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::HP_REGEN: pJob->itemStat.hpRegenPerSec = stat.randomStatValue; break;
		case RANDOM_STAT_TYPE::MP_REGEN: pJob->itemStat.mpRegenPerSec = stat.randomStatValue; break;
		default:
			__debugbreak();
		}
	}

	m_pDBManager->EnqueueDBJob(pJob);

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
		// 이미 비워져 있거나 index 이상하면 false 리턴
		if (!m_inventory.DeleteInventorySlot(slotIndex, retID))
			return false;

		const UserItem* pItem = m_storage.FindItem(retID);
		if (pItem == nullptr)
			__debugbreak();

		// 제거 성공하면 인벤토리에 해당 index 할당자에 반환
		m_inventory.ReturnSlotIndex(slotIndex);
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
	if (!m_storage.DeleteItem(retID))
		return false;

	DeleteItemJob* pJob = new DeleteItemJob;
	pJob->itemUID = retID;
	m_pDBManager->EnqueueDBJob(pJob);

	return true;
}

// 해당 아이템 슬롯에 대해서 우클릭 할때 혹은 해당 버튼 누를 때 작동
bool CUser::UseInventoryItem(int16 slotIndex, UseItemResult& result)
{
	ITEM_UID retID = m_inventory.GetItemUID(slotIndex);
	if (retID == ItemUID::ITEM_UID_INVALID_ID)
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
		result.consumableResult.slotType = SLOT_TYPE::INVENTORY;
		result.consumableResult.newItenCount = newItemCount;
		result.consumableResult.slotIndex = slotIndex;

		// 사용 후 카운트가 0이 아니면 지울 필요없으니 리턴
		if (newItemCount != 0)
		{
			ItemCountUpdateJob* pJob = new ItemCountUpdateJob;
			pJob->itemUID = retID;
			pJob->newCount = newItemCount;
			m_pDBManager->EnqueueDBJob(pJob);
			return true;
		}

		// 다 사용했으면 아이템 삭제
		ITEM_UID retUID;
		if (!m_inventory.DeleteInventorySlot(slotIndex, retUID))
			__debugbreak();

		m_inventory.ReturnSlotIndex(slotIndex);

		if (!m_storage.DeleteItem(retID))
			return false;

		DeleteItemJob* pJob = new DeleteItemJob;
		pJob->itemUID = retID;
		m_pDBManager->EnqueueDBJob(pJob);
		return true;
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
	int16 emptyIdx = m_inventory.GainEmptySlotIndex();
	if (emptyIdx == -1)
		return false;

	// 인벤토리 여유 있으면 기존 장비 탭에서 빼고 인벤토리에 삽입
	ITEM_UID unEquippedItemUID;
	if (!m_equipment.UnEquippedItem(static_cast<EQUIP_SLOT>(slotIndex), unEquippedItemUID))
	{
		m_inventory.ReturnSlotIndex(emptyIdx);
		return false;
	}

	if (!m_inventory.InsertItemToSlot(unEquippedItemUID, emptyIdx))
	{
		m_inventory.ReturnSlotIndex(emptyIdx);
		return false;
	}
	result.resultType = USE_ITEM_RESULT::UNEQUIP;
	result.unEquipResult.inventorySlotIdx = emptyIdx;

	//ItemSwapJob* pJob = new ItemSwapJob;
	//pJob->itemA.itemUID = unEquippedItemUID;
	//pJob->itemA.newSlotIndex = emptyIdx;
	//pJob->itemA.newSlotType = SLOT_TYPE::INVENTORY;
	//m_pDBManager->EnqueueDBJob(pJob);

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
	{
		ItemCountUpdateJob* pJob = new ItemCountUpdateJob;
		pJob->itemUID = retUID;
		pJob->newCount = newItemCount;
		m_pDBManager->EnqueueDBJob(pJob);

		return true;
	}

	ITEM_UID eraseUID;
	if (!m_quickSlot.ClearConsumable(slotIndex, eraseUID))
		return false;

	if (!m_storage.DeleteItem(eraseUID))
		return false;

	DeleteItemJob* pJob = new DeleteItemJob;
	pJob->itemUID = retUID;
	m_pDBManager->EnqueueDBJob(pJob);

	return true;
}

// 내 아이템 슬롯 위치 바꿀 때 작동
bool CUser::ItemSlotChange(SLOT_TYPE fromType, int16 fromIndex, SLOT_TYPE toType, int16 toIndex)
{
	// 슬롯 타입 범위 체크
	if ((!SlotTypeRangeCheck(fromType)) || (!SlotTypeRangeCheck(toType)))
		return false;

	// todo : from이 InvalidID면 비정상 유저로 판단해서 끊기
	switch (fromType)
	{
	case SLOT_TYPE::INVENTORY:
	{
		ITEM_UID retID = m_inventory.GetItemUID(fromIndex);
		if (retID == ItemUID::ITEM_UID_INVALID_ID)
			return false;
	}
	break;

	case SLOT_TYPE::EQUIPMENT:
	{
		ITEM_UID retUID = m_equipment.GetEquippedItem(static_cast<EQUIP_SLOT>(fromIndex));
		if (retUID == ItemUID::ITEM_UID_INVALID_ID)
			return false;
	}
	break;

	case SLOT_TYPE::QUICKSLOT:
	{
		ITEM_UID retUID = m_quickSlot.GetQuickSlotItem(fromIndex);
		if (retUID == ItemUID::ITEM_UID_INVALID_ID)
			return false;
	}
	break;

	default:
		return false;
	}

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

	int16 atk = GetAtk(curTime);
	int16 damage = skillData.BaseDamage + static_cast<int16>(atk * skillData.AttackRatio);
	
	switch (skillData.DamageType)
	{
	case ESkillDamageType::Physical:
	{
		int16 targetDef = target->GetDef(curTime);

		// 데미지 낮아도 1딜 들어감.
		if (damage <= targetDef)
			damage = 1;
		else
			damage -= targetDef;

		break;
	}

	case ESkillDamageType::Magic:
	{
		int16 targetDef = target->GetDef(curTime);

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

	int16 atk = GetAtk(curTime);
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

	int16 atk = GetAtk(curTime);

	// if swing index마다 데미지 배율 다르게 하고 싶으면 ratio 수정

	float ratio = 1.0f;

	uint32 damage = static_cast<uint32>(atk * ratio);
	int16 targetDef = target->GetDef(curTime);

	if (damage <= targetDef)
		return 1;

	return damage - targetDef;
}

uint32 CUser::CalBaseAttackDamage(CMonster* target, uint32 curTime)
{
	if (target == nullptr || target->GetMonsterState() == EMonsterState::Dead)
		return 0;

	int16 atk = GetAtk(curTime);

	// if swing index마다 데미지 배율 다르게 하고 싶으면 ratio 수정

	float ratio = 1.0f;

	uint32 damage = static_cast<uint32>(atk * ratio);
	uint16 targetDef = target->GetDef();

	if (damage <= targetDef)
		return 1;

	return damage - targetDef;
}

int16 CUser::GetDef(uint32 curTime)
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

int16 CUser::GetAtk(uint32 curTime)
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

int16 CUser::GetMaxHP(uint32 curTime)
{
	uint16 maxhp = m_baseStat.m_maxHP + m_equipment.GetMaxHP();

	return maxhp;
}

int16 CUser::GetMaxMP(uint32 curTime)
{
	uint16 maxmp = m_baseStat.m_maxMP + m_equipment.GetMaxMP();

	return maxmp;
}

int16 CUser::GetHPRegenSec() const
{
	uint16 HPRegenSec = m_baseStat.m_hpRegenPerSec + m_equipment.GetHPRegen();

	return HPRegenSec;
}

int16 CUser::GetMPRegenSec() const
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

void CUser::ItemSlotUpdate()
{
	m_itemSlotUpdateTimeAccum += FieldConst::UPDATE_LOOP_TIME;

	if (m_itemSlotUpdateTimeAccum < m_itemSlotUpdateTime)
		return;

	// Storage 클래스 호출해주기
	m_storage.ItemSlotUpdate();
}

void CUser::InventoryItemLoad(ItemLoadData& Item)
{
	// 삽입할 index는 indexAllocator에서 제거
	m_inventory.EraseEmptyIndex(Item.slotIndex);

	// 해당 index에 아이템 삽입
	m_inventory.InsertItemToSlot(Item.itemUID, Item.slotIndex);

}

void CUser::EquipmentItemLoad(ItemLoadData& Item)
{
	// 해당 index에 장비 탭 삽입
	ITEM_UID retID;
	m_equipment.EquippedItem(static_cast<EQUIP_SLOT>(Item.slotIndex), Item.itemUID, retID);
}

void CUser::QuickSlotItemLoad(ItemLoadData& Item)
{
	ITEM_UID retID;
	m_quickSlot.SetConsumable(Item.slotIndex, Item.itemUID, retID);
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
	{
		int16 maxHP = GetMaxHP(curTime);
		m_hp += pItemData->recoverHP;
		if (m_hp > maxHP)
			m_hp = maxHP;
		m_consumableCooltimeInfo.CooltimeStartTimeMs[(int)consumeType] = curTime;
	}
		break;

	case CONSUMABLE_ITEM_TYPE::SMALL_MP_POTION:
	{
		uint16 maxMP = GetMaxMP(curTime);
		m_mp += pItemData->recoverMP;
		if (m_mp > maxMP)
			m_mp = maxMP;
		m_consumableCooltimeInfo.CooltimeStartTimeMs[(int)consumeType] = curTime;
	}
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
	ITEM_UID retID = m_inventory.GetItemUID(inventorySlotIndex);
	if (retID == ItemUID::ITEM_UID_INVALID_ID)
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

	uint8 slotUpdateCount = 0;
	result.resultType = USE_ITEM_RESULT::EQUIP;
	result.equipResult.resultSlot[slotUpdateCount].slotType = SLOT_TYPE::EQUIPMENT;
	result.equipResult.resultSlot[slotUpdateCount].slotIndex = static_cast<int16>(pItemData->equipSlot);
	result.equipResult.resultSlot[slotUpdateCount].itemID = pItemData->itemID;
	slotUpdateCount++;

	result.equipResult.resultSlot[slotUpdateCount].slotType = SLOT_TYPE::INVENTORY;
	result.equipResult.resultSlot[slotUpdateCount].slotIndex = inventorySlotIndex;

	//ItemSwapJob* pJob = new ItemSwapJob;
	//pJob->itemA.itemUID = removedUID;
	//pJob->itemA.newSlotType = SLOT_TYPE::EQUIPMENT;
	//pJob->itemA.newSlotIndex = static_cast<int16>(pItemData->equipSlot);

	// 해당 장비 슬롯에 장착된 장비가 없으면 그냥 리턴
	if (OutEquipItem == ItemUID::ITEM_UID_INVALID_ID)
	{
		result.equipResult.resultSlot[slotUpdateCount].itemID = ItemUID::ITEM_UID_INVALID_ID;
		m_inventory.ReturnSlotIndex(inventorySlotIndex);
		return true;
	}

	// 있으면 inventoryindex로 기존에 장착하던 장비 넣기(뺐는데 못넣는거는 서버 문제)
	if (!m_inventory.InsertItemToSlot(OutEquipItem, inventorySlotIndex))
		__debugbreak();

	const UserItem* pOutEquipItem = m_storage.FindItem(OutEquipItem);
	if (pOutEquipItem == nullptr)
		__debugbreak();

	result.equipResult.resultSlot[slotUpdateCount].itemID = pOutEquipItem->itemID;

	return true;
}

// 해당 장비 슬롯에 있는 장비 해제하려고 할 때 작동
bool CUser::UnEquippedItem(EQUIP_SLOT equipSlot)
{
	// 장비 해제 실패하면 false 리턴
	ITEM_UID retUID;
	if (!m_equipment.UnEquippedItem(equipSlot, retUID))
		return false;

	// 인벤토리에 여유 공간 있는지 확인 없으면 false
	int16 emptyIdx = m_inventory.GainEmptySlotIndex();
	if (emptyIdx == -1)
		return false;

	// 있으면 해당 슬롯에 옮기기
	return m_inventory.InsertItemToSlot(retUID, emptyIdx);
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
	ITEM_UID equipmentUID;
	ITEM_UID inventoryUID = m_inventory.GetItemUID(inventoryIndex);
	if (inventoryUID == ItemUID::ITEM_UID_INVALID_ID)
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
	ITEM_UID inventoryUID = m_inventory.GetItemUID(inventoryIndex);
	ITEM_UID quickSlotUID = m_quickSlot.GetQuickSlotItem(quickSlotIndex);

	// 매개인자로 받은 index에 있는 UID의 ITEM_ID 얻기 위해 Find
	const UserItem* pInvenUserItem = m_storage.FindItem(inventoryUID);
	const UserItem* pQuickSlotUserItem = m_storage.FindItem(quickSlotUID);

	const ItemData* pInvenItem = nullptr;
	const ItemData* pQuickItem = nullptr;

	// 매개인자로 받은 index에 있는 UID에 있는 아이템의 타입 체크하기 위해 GetItemData 호출
	if (pInvenUserItem != nullptr)
	{
		pInvenItem = ItemTable::GetItemData(pInvenUserItem->itemID);
	}

	if (pQuickSlotUserItem != nullptr)
	{
		pQuickItem = ItemTable::GetItemData(quickSlotUID);
	}


	// todo : 둘 중 하나라도 아이템이 소모품이 아니면 false 리턴 후 해당 유저 끊기
	if (pInvenItem)
	{
		if (pInvenItem->itemType != ITEM_TYPE::CONSUMABLE)
			return false;
	}

	if (pQuickItem)
	{
		if (pQuickItem->itemType != ITEM_TYPE::CONSUMABLE)
			return false;
	}
	
	// 인벤토리와 퀵슬롯에서 UID 제거
	m_inventory.DeleteInventorySlot(inventoryIndex, inventoryUID);
	m_quickSlot.ClearConsumable(quickSlotIndex, quickSlotUID);

	// from Invalid는 상위 함수에서 걸러졌으니 여기서 Invalid면 To가 Invalid인 상황
	if (inventoryUID == ItemUID::ITEM_UID_INVALID_ID)
	{
		// to가 Inventory인데 빈 슬롯인 상황
	
		// 퀵슬롯 UID가 인벤토리에 오니 해당 index는 할당 불가
		m_inventory.EraseEmptyIndex(inventoryIndex);
		m_inventory.InsertItemToSlot(quickSlotUID, inventoryIndex);
		return true;
	}
	
	else if (quickSlotUID == ItemUID::ITEM_UID_INVALID_ID)
	{
		// to가 QuickSlot인데 빈 슬롯인 상황

		// 기존 인벤토리 아이템이 퀵슬롯으로 이동하니 해당 인벤토리 index 반환
		m_inventory.ReturnSlotIndex(inventoryIndex);
		m_quickSlot.SetConsumable(quickSlotIndex, inventoryUID, quickSlotUID);
		return true;
	}

	// 둘 다 아이템이 있는 상황
	ITEM_UID retQuickUID = ItemUID::ITEM_UID_INVALID_ID;
	m_quickSlot.SetConsumable(quickSlotIndex, inventoryUID, retQuickUID);
	m_inventory.InsertItemToSlot(quickSlotUID, inventoryIndex);
	return true;
}

