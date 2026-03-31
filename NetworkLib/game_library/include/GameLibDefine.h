#pragma once

namespace GameLibrary
{
	constexpr WORD  GAMELIB_MAX_THREAD_COUNT    = 100;
	constexpr WORD  GAMELIB_MAX_GROUP_COUNT     = 100;
	constexpr WORD  GAMELIB_MAX_SERVICE_COUNT   = 100;
	constexpr WORD  GAMELIB_IP_LEN              = 16;
	constexpr WORD  GAMELIB_INDEX_POS           = 48;
	constexpr WORD  GAMELIB_SESSIONID_POS       = 47;
	constexpr WORD  GAMELIB_SENDQ_MAX_SIZE      = 10000;
	constexpr BYTE  GAMELIB_MAX_FRAMEPROC_COUNT = 3;

}

namespace GameSession
{
	constexpr WORD   GAMESESSION_WSABUFSIZE = 100;
	constexpr UINT64 GAMESESSION_INVALID_ID = -1;
}

namespace Group
{
	constexpr DWORD GROUP_DEFAULT_FRAME = 50;
	constexpr DWORD GROUP_INVALID_GROUPID = -1;
}

namespace Service
{
	constexpr DWORD SERVICE_DEFAULT_FRAME = -1;
}

