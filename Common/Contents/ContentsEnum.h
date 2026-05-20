#pragma once
#include "ContentsType.h"

/////////////////////////////////////////
//           <MoveMode>
// 0 : Stop  / 1 : Walk / 2 : Run
/////////////////////////////////////////
enum class EM1MoveMode : uint8
{
	Walk = 0,   // Shift 안눌렀을 때
	Run         // Shift 눌렀을 때
};

// 상태관련 타입
enum class EM1ActionStateType : uint8
{
	None = 0,
	Attack,
	Skill,
	Hit,
	Dead
};

enum class EAIState : uint8
{
	None = 0,
	Idle,
	Patrol,
	Chase,
	Combat,
	Return,
	Dead
};

enum class EServerAbilitySlot : uint8
{
	Skill1,
	Skill2,
	Skill3,
	Skill4,
};

enum class ESkillDamageType : uint8
{
	None,
	Physical,
	Magic,
	TrueDamage,
};

enum class EHitShape : uint8
{
	None,
	Circle,
	Cone,
	Box,
};

enum class EMonsterState : uint8
{
	Idle,           // 대기
	Patrol,         // 주변 순찰
	Chase,          // 추격
	Attack,         // 공격
	Return,         // 스폰 위치로 복귀
	Dead            // 사망, 리젠 대기
};
