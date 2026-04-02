#pragma once
#include "ContentsType.h"

enum class ERouteType : uint8
{
	GROUP = 0,
	SERVICE,
	NONE,
};


namespace ServiceID
{
	constexpr uint16 NONE_SERVICE = 0;
	constexpr uint16 CHAT_SERVICE = 1;
}


#pragma pack(push,1)
struct st_GAMELIB_LANHEADER
{
	uint16  s_len;
	uint16  s_serviceID;   // 
	uint8   s_routeType;    // 
}typedef GAMELIB_LANHEADER;
#pragma pack(pop)

