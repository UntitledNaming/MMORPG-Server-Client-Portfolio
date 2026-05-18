#include "ContentsDefine.h"
#include "ContentsStruct.h"
#include "MemoryPoolTLS.h"
#include "SkillTable.h"
#include "SectorPos.h"
#include "CMonster.h"
#include "IUser.h"
#include "CUser.h"
#include "HitSearchBuilder.h"

void HitSearchBuilder::MakeBaseAttack(CUser* attacker, float attackYaw, HitSearchInfo& outInfo)
{
    outInfo.shape = EHitShape::Cone;

    outInfo.x = attacker->GetX();
    outInfo.y = attacker->GetY();
    outInfo.attackYaw = attackYaw;

    outInfo.range = ClientAttack::LEFTATTACK_RANGE;
    outInfo.halfAngleDegree = ClientAttack::LEFATTACK_HALF_ANGLE;

    outInfo.bHitUser = true;
    outInfo.bHitMonster = true;

    outInfo.MaxUserCount = ClientAttack::LEFTATTACK_MAX_HIT_COUNT;
    outInfo.MaxMonsterCount = ClientAttack::LEFTATTACK_MAX_HIT_COUNT;

}

void HitSearchBuilder::MakeSkillAttack(CUser* attacker, uint8 skillIndex, float attackYaw, HitSearchInfo& outInfo)
{
    const SkillData& hitData = g_skillData[skillIndex];

    outInfo = {};

    outInfo.shape = hitData.HitShape;

    outInfo.x = attacker->GetX();
    outInfo.y = attacker->GetY();
    outInfo.attackYaw = attackYaw;

    outInfo.range = hitData.Range;
    outInfo.halfAngleDegree = hitData.HalfAngleDegree;

    outInfo.bHitUser = hitData.bHitUser;
    outInfo.bHitMonster = hitData.bHitMonster;

    outInfo.MaxUserCount = hitData.MaxUserCount;
    outInfo.MaxMonsterCount = hitData.MaxMonsterCount;

}
