#pragma once
#include "ContentsType.h"
#include "ContentsDefine.h"
#include "ContentsStruct.h"


class QuickSlot
{
public:
	QuickSlot() = default;
	~QuickSlot() = default;

	void Init();
	void Destroy();

	bool     SetConsumable(int16 slotIndex, ITEM_UID InItemUID, ITEM_UID& OutItemUID);
	bool     ClearConsumable(int16 slotIndex, ITEM_UID& OutItemUID);
	bool     SwapSlot(int16 fromIndex, int16 toIndex);
	ITEM_UID GetQuickSlotItem(int16 index);

private:
	bool     IndexRangeCheck(int16 slotIndex);

private:
	ITEM_UID  m_quickSlot[UserQuickSlot::QUICK_SLOT_MAX];
};

