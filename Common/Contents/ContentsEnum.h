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
	LeftAttack = 0,
	Skill1,
	Skill2,
	Skill3,
	Skill4,
};