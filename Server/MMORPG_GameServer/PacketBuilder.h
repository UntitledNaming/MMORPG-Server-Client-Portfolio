#pragma once
#include <vector>
#include "ContentsType.h"

class CMessage;
class CUser;
class CMonster;
struct FieldDropItem;

class PacketBuilder
{
public:
	static CMessage* CreateMyCharacter(CUser* pUser);
	static CMessage* CreateOtherCharacter(CUser* pUser);
	static CMessage* DeleteCharacter(CUser* pUser);
	static CMessage* UpdateCharacterMovement(CUser* pUser);
	static CMessage* SyncMyCharacter(CUser* pUser);
	static CMessage* SyncOtherCharacter(CUser* pUser);
	static CMessage* CreateRTTEchoMessage();
	static CMessage* AttackLeftSwing(uint64 sessionID, float attackYaw, uint8 swingIndex);
	static CMessage* HitTarget(uint8 hitPlayerCount, uint8 hitMonsterCount, std::vector<CUser*>& hitPlayerArray, std::vector<CMonster*>& hitMonsterArray);
	static CMessage* UseSkillRes(uint8 skillSlot, bool success);
	static CMessage* UseSkillBroadCast(uint64 id, uint8 skillSlot);

	static CMessage* CreateMonster(CMonster* pMonster);
	static CMessage* DeleteMonster(CMonster* pMonster);
	static CMessage* MoveMonster(CMonster* pMonster, const Location& DesLocation);
	static CMessage* StopMonster(CMonster* pMonster, const Location& StopLocation);
	static CMessage* AttackMonsterToMe(CMonster* pMonster, uint64 TargetID, int16 newHP);
	static CMessage* AttackMonsterToOther(CMonster* pMonster, CUser* pTarget);


	static CMessage* CreateFieldDropItem(FieldDropItem* pItem);
	static CMessage* DeleteFieldDropItem(FieldDropItem* pItems);
	static CMessage* PickUpEquipFieldDropItem(PickUpEquipResult* result);
	static CMessage* PickUpConsumableFieldDropItem(PickUpConsumableResult* result);
	static CMessage* DeleteItem(bool Success);
	static CMessage* UseItem(UseItemResult& result);
	static CMessage* SwapSlot(bool Success);

	static CMessage* GainExp(GainEXPResult& result);
};

