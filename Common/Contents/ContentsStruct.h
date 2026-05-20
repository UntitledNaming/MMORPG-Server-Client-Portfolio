#pragma once
#include <vector>
#include "ContentsType.h"
#include "ContentsEnum.h"
#include "ContentsDefine.h"

struct Vec2
{
	float m_xpos;
	float m_ypos;
};

struct SkillData
{
	uint16           MaxUserCount;    // 타격시 최대 피격 유저수
	uint16           MaxMonsterCount; // 타격시 최대 피격 몬스터수
	uint16           RequiredMana;    // 필요 마나
	uint32           CoolTime;        // 쿨 타임
	uint32           Duration;        // 지속 시간
	uint16           BaseDamage;      // 고정 데미지
	float            Range ;          // 공격 거리
	float            AttackRatio;     // 공격력 계수
	float            HalfAngleDegree; // 공격 범위 절반 각도
	bool             bHitUser;        // 유저 피격 할지 말지
	bool             bHitMonster;     // 몬스터 피격 할지 말지
	ESkillDamageType DamageType;      
	EHitShape        HitShape;        // 공격 모양
};

struct Location
{
	float xpos;
	float ypos;
	float zpos;
};

