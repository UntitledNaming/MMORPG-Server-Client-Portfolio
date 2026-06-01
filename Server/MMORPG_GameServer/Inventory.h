#pragma once
#include "ContentsDefine.h"
#include "ContentsStruct.h"

class Inventory
{
public:
	Inventory() = default;
	~Inventory() = default;

	void Init();
	void Destroy();

private:
	uint64  m_inventory[UserInventory::INVENTORY_SLOT_MAX];
};

