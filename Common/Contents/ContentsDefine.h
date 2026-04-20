#pragma once

namespace AuthConst
{
	constexpr DWORD NONUSER_TIMEOUT = 3500;
}

namespace FieldConst
{
	constexpr DWORD UPDATE_FRAME              = 50; // 50ms 마다 프레임 로직 실행
	constexpr DWORD USER_TIMEOUT              = 40000;
	constexpr WORD  SECTOR_SIZE               = 100;
	constexpr WORD  SECTOR_USER_DEFAULT_COUNT = 100;
	constexpr WORD  SECTOR_Y_MAX              = 50;
	constexpr WORD  SECTOR_X_MAX              = 50;
	constexpr FLOAT SYNC_X_RANGE = 30;
	constexpr FLOAT SYNC_Y_RANGE = 30;
	constexpr FLOAT PI = 3.1415926535f;
}

namespace InputMask
{
	constexpr WORD None  = 1 << 0;
	constexpr WORD North = 1 << 1;
	constexpr WORD South = 1 << 2;
	constexpr WORD East  = 1 << 3;
	constexpr WORD West  = 1 << 4;
}

namespace UserConst
{
	constexpr WORD  NICK_MAX = 64;
	constexpr FLOAT WALK_SPEED = 2.0;
	constexpr FLOAT RUN_SPEED = 6.0;
}

