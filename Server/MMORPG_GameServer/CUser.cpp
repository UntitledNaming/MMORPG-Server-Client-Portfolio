#include <string>
#include <windows.h>
#include "ContentsEnum.h"
#include "SkillTable.h"
#include "IUser.h"
#include "CUser.h"

CMPoolTLS<CUser> CUser::m_userPool;

using namespace UserConst;

void CUser::Init(uint64 sessionID)
{
	// todo : 추후 DB에서 데이터 긁어와서 초기화 하기

	m_sessionID = sessionID;
	m_location = Location{ 381250.0f , 443750.0f ,-38690.f };
	m_moveFlag = false;
	m_hp = 100;
	m_mp = 100;
	m_mpRegenPerSec = WARRIOR_MANA_REGEN;
	m_secPos.SetPos(SectorPos((m_location.xpos - FieldConst::MAP_WORLD_OFFSET_X) / FieldConst::SECTOR_SIZE, (m_location.ypos - FieldConst::MAP_WORLD_OFFSET_Y) / FieldConst::SECTOR_SIZE));
	m_arrayIdx = 0;
	m_mpRegenPerSec = 5;
	m_syncCount = 0;
	m_movementYaw = 0.0f;
	m_maxWalkSpeed = WALK_SPEED;
	m_moveSpeed = m_maxWalkSpeed / FieldConst::UPDATE_FRAME;
	m_recvTime = timeGetTime();
	m_lastSyncCheckTime = timeGetTime();
	m_disconnectFlag = false;

	// SwingInfo 초기화
	m_swingInfo.m_lastSwingIdx = 0;
	m_swingInfo.m_lastSwingRecvTime = 0;

	for (int i = 0; i < USER_SKILL_SLOT_COUNT; i++)
	{
		m_skillInfo[i].m_skillActivate = false;
		m_skillInfo[i].m_skillExpiredTime = 0;
		m_skillInfo[i].m_skillLastRecvTime = 0;
	}

	// 스탯 초기화
	m_baseStat.m_atk = 5;
	m_baseStat.m_def = 1;
	m_baseStat.m_maxHP = 100;
	m_baseStat.m_maxMP = 100;
	m_baseStat.m_atk = BASE_ATK;
	m_baseStat.m_def = BASE_DEF;
	m_baseStat.m_maxHP = BASE_MAXHP;
	m_baseStat.m_maxMP = BASE_MAXMP;

	m_equipBonusStat.m_atk = 0;
	m_equipBonusStat.m_def = 0;
	m_equipBonusStat.m_maxHP = 0;
	m_equipBonusStat.m_maxMP = 0;
}

void CUser::Destroy()
{

}

void CUser::ManaRegen(uint32 curTime)
{
	m_mp += m_mpRegenPerSec;

	uint16 maxmp = GetMaxMP(curTime);

	if (m_mp > maxmp)
		m_mp = maxmp;
}

void CUser::Damage(uint16 damage)
{
	if (m_disconnectFlag == true)
		return;

	m_hp -= damage;
	
	if (m_hp < 0)
		m_hp = 0;
	
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

void CUser::SectorFind(SectorAround& pAround)
{
	m_secPos.SectorFind(pAround, m_secPos);
}

void CUser::CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector)
{
	m_secPos.CalSectorTransitionMessageTargets(oldSecPos, newSecPos, outDeleteSector, outCreateSector);
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

	if (curTime - m_swingInfo.m_lastSwingRecvTime <= ClientAttack::LEFTATTACK_SWING_INTERVAL)
		return false;

	m_swingInfo.m_lastSwingRecvTime = curTime;

	return true;
}

uint32 CUser::CalSkillDamage(uint16 skillIndex, CUser* target, uint32 curTime)
{
	if (skillIndex >= USER_SKILL_SLOT_COUNT || target == nullptr || target->m_disconnectFlag == true)
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

uint32 CUser::CalBaseAttackDamage(CUser* target, uint32 curTime)
{
	if (target == nullptr || target->m_disconnectFlag == true)
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

uint16 CUser::GetDef(uint32 curTime)
{
	uint16 def = m_baseStat.m_def + m_equipBonusStat.m_def;

	// 버프 유효성 체크
	// 버프 아직 켜져있으면서 만료시간이 안되었으면 def 증가

	for (int i = 0; i < USER_BUFF_SKILL_SLOT_COUNT; i++)
	{
		if (m_skillInfo[i].m_skillActivate && m_skillInfo[i].m_skillExpiredTime > curTime)
			def += ClientAttack::BUFF_DEF_ADD_AMOUNT;
	}

	// todo : 타 버프/디버프 스킬 유효성 체크

	return def;
}

uint16 CUser::GetAtk(uint32 curTime)
{
	uint16 atk = m_baseStat.m_atk + m_equipBonusStat.m_atk;

	// 버프 유효성 체크
	// 버프 아직 켜져있으면서 만료시간이 안되었으면 def 증가

	for (int i = 0; i < USER_BUFF_SKILL_SLOT_COUNT; i++)
	{
		if (m_skillInfo[i].m_skillActivate && m_skillInfo[i].m_skillExpiredTime > curTime)
			atk += ClientAttack::BUFF_ATK_ADD_AMOUNT;
	}

	// 디버프 유효성

	return atk;
}

uint16 CUser::GetMaxHP(uint32 curTime)
{
	uint16 maxhp = m_baseStat.m_maxHP + m_equipBonusStat.m_maxHP;

	return maxhp;
}

uint16 CUser::GetMaxMP(uint32 curTime)
{
	uint16 maxmp = m_baseStat.m_maxMP + m_equipBonusStat.m_maxMP;

	// 버프 유효성 체크
	// 버프 아직 켜져있으면서 만료시간이 안되었으면 def 증가


	// 디버프 유효성
	return maxmp;
}

CUser* CUser::Alloc()
{
	return m_userPool.Alloc();
}

void CUser::Free(CUser* pUser)
{
	m_userPool.Free(pUser);
}

