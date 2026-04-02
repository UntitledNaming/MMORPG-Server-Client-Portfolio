#pragma once
#include <windows.h>

enum class ERouteType : BYTE
{
	GROUP = 0,
	SERVICE,
	NONE,
};



namespace ServiceID
{
	constexpr WORD NONE_SERVICE = 0;
	constexpr WORD CHAT_SERVICE = 1;
}



//////////////////////////////////////////////
// 
// 
//////////////////////////////////////////////
#pragma pack(push,1)
struct st_GAMELIB_LANHEADER
{
	WORD  s_len;
	WORD  s_serviceID;   // 
	BYTE  s_routeType;   // 
}typedef GAMELIB_LANHEADER;
#pragma pack(pop)

