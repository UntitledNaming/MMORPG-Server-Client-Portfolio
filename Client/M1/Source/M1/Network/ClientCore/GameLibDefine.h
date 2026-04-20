#pragma once

namespace GameLibrary
{
	constexpr uint16  GAMELIB_MAX_THREAD_COUNT    = 100;
	constexpr uint16  GAMELIB_MAX_GROUP_COUNT     = 100;
	constexpr uint16  GAMELIB_MAX_SERVICE_COUNT   = 100;
	constexpr uint16  GAMELIB_IP_LEN              = 16;
	constexpr uint16  GAMELIB_INDEX_POS           = 48;
	constexpr uint16  GAMELIB_SESSIONID_POS       = 47;
	constexpr uint16  GAMELIB_SENDQ_MAX_SIZE      = 10000;
	constexpr uint8   GAMELIB_MAX_FRAMEPROC_COUNT = 3;

}

namespace GameSession
{
	constexpr uint16 GAMESESSION_WSABUFSIZE = 100;
	constexpr uint64 GAMESESSION_INVALID_ID = -1;
}

namespace Group
{
	constexpr uint32 GROUP_DEFAULT_FRAME = 50;
	constexpr uint32 GROUP_INVALID_GROUPID = -1;
}

namespace Service
{
	constexpr uint32 SERVICE_DEFAULT_FRAME = -1;
}

