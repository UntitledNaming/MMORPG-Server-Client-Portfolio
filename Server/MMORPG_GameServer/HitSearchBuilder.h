#pragma once
#include "ContentsType.h"

class CUser;
class CMonster;

struct HitSearchInfo
{
	EHitShape shape;

	float x;
	float y;
	float attackYaw;

	float range;
	float halfAngleDegree;

	bool bHitUser;           // 플레이어 타격 가능
	bool bHitMonster;        // 몬스터   타격 가능
	uint16 MaxUserCount;     // 타격 최대 갯수
	uint16 MaxMonsterCount;  // 타격 최대 갯수

};

struct HitResult
{
	HitResult()
	{
		HitUserArray.resize(ClientAttack::MaxUserCount);
		HitMonsterArray.resize(ClientAttack::MaxMonsterCount);
	}

	uint8 HitUserCount = 0;
	uint8 HitMonsterCount = 0;
	std::vector<CUser*> HitUserArray;
	std::vector<CMonster*> HitMonsterArray;
};

// HitSerachInfo를 평타와 스킬 구분하여 만드는 클래스 제공 
class HitSearchBuilder
{
public:
    static void MakeBaseAttack(CUser* attacker, float attackYaw, HitSearchInfo& outInfo);
           
    static void MakeSkillAttack(CUser* attacker, uint8 skillIndex, float attackYaw, HitSearchInfo& outInfo);
};

