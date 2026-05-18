#pragma once
#include "ContentsStruct.h"
#include "ContentsDefine.h"
#include "ContentsEnum.h"

static SkillData g_skillData[UserConst::USER_SKILL_SLOT_COUNT] =
{
	// Skill 1 : Buff
	{
		0,
		0,
		ClientAttack::BUFF_REQUIRED_MANA,
		ClientAttack::BUFF_COOLTIME_SEC,
		ClientAttack::BUFF_DURATION,
		0,
		0.f,
		0.f,
		0.f,
		false,
		false,
		ESkillDamageType::None,
		EHitShape::None
	},

	// Skill 2 : SpinSlash
	{
		ClientAttack::SPINSLASH_MAX_HIT_COUNT,
		ClientAttack::SPINSLASH_MAX_HIT_COUNT,
		ClientAttack::SPINSLASH_REQUIRED_MANA,
		ClientAttack::SPINSLASH_COOLTIME_SEC,
		0,
		5,
		ClientAttack::SPINSLASH_RANGE,
		1.0f,
		0.f,
		true,
		true,
		ESkillDamageType::Physical,
		EHitShape::Circle
	},

	// Skill 3 : GroundSmash
	{
		ClientAttack::GROUNDSMASH_MAX_HIT_COUNT,
		ClientAttack::GROUNDSMASH_MAX_HIT_COUNT,
		ClientAttack::GROUNDSMASH_REQUIRED_MANA,
		ClientAttack::GROUNDSMASH_COOLTIME_SEC,
		0,
		10,
		ClientAttack::GROUNDSMASH_RANGE,
		1.0f,
		0.f,
		true,
		true,
		ESkillDamageType::Physical,
		EHitShape::Circle
	},

	// Skill 4 : Ultimate
	{
		ClientAttack::ULTIMATE_MAX_HIT_COUNT,
		ClientAttack::ULTIMATE_MAX_HIT_COUNT,
		ClientAttack::ULTIMATE_REQUIRED_MANA,
		ClientAttack::ULTIMATE_COOLTIME_SEC,
		0,
		15,
		ClientAttack::ULTIMATE_RANGE,
		2.0f,
		0.f,
		true,
		true,
		ESkillDamageType::TrueDamage,
		EHitShape::Circle
	}
};