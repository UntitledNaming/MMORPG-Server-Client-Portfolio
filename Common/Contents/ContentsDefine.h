#pragma once
#include "ContentsType.h"

namespace AuthConst
{
	constexpr uint32 NONUSER_TIMEOUT = 3500;
}

namespace FieldConst
{
	constexpr uint32  UPDATE_FRAME              = 50;     // 50ms frame loop
	constexpr uint32  USER_TIMEOUT              = 40000;
	constexpr uint16  SECTOR_SIZE               = 100;
	constexpr uint16  SECTOR_USER_DEFAULT_COUNT = 100;
	constexpr uint16  SECTOR_Y_MAX              = 50;
	constexpr uint16  SECTOR_X_MAX              = 50;
	constexpr float   SYNC_X_RANGE = 30;
	constexpr float   SYNC_Y_RANGE = 30;
	constexpr float   Pi = 3.1415926535f;
}

namespace InputMask
{
	constexpr uint16 None  = 1 << 0;
	constexpr uint16 North = 1 << 1;
	constexpr uint16 South = 1 << 2;
	constexpr uint16 East  = 1 << 3;
	constexpr uint16 West  = 1 << 4;
}

namespace UserConst
{
	constexpr uint16  NICK_MAX = 64;
	constexpr float WALK_SPEED = 2.0;
	constexpr float RUN_SPEED = 6.0;
	constexpr float JUMP_ANIMATION_TIME = 0.0f;
}

namespace ClientMovement
{
	constexpr float MOVEMENT_SEND_INTERNAL_SEC      = 0.1f;
	constexpr float MOVEMENT_YAW_SEND_THRESHOLD_DEG = 10.0f;
}