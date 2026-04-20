#pragma once
#include "ContentsType.h"

enum class ELocomotionType : uint8
{
	Idle = 0,
	Walk,
	Run,
	Jump
};

enum class EM1ActionType : uint8
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