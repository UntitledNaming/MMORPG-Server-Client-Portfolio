#pragma once
#include <windows.h>

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

enum class ERouteType : unsigned char
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