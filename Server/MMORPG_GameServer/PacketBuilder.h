#pragma once
#include <vector>
#include "ContentsType.h"

class CMessage;
class CUser;
class CMonster;

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
	static CMessage* AttackLeftSwing(CUser* pUser, float attackYaw);
	static CMessage* StopLeftSwing(CUser* pUser);
	static CMessage* HitTarget(uint8 hitPlayerCount, uint8 hitMonsterCount, std::vector<CUser*>& hitPlayerArray, std::vector<CMonster*>& hitMonsterArray);
	static CMessage* UseSkillRes(uint8 skillSlot, bool success);
	static CMessage* UseSkillBroadCast(uint64 id, uint8 skillSlot);

	static CMessage* CreateMonster(CMonster* pMonster);
	static CMessage* DeleteMonster(CMonster* pMonster);
	static CMessage* MoveMonster(CMonster* pMonster, Location& DesLocation);
	static CMessage* AttackMonster(CMonster* pMonster, uint64 TargetID, uint16 newHP);
};

