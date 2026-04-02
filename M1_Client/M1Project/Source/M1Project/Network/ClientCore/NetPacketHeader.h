#pragma once
#include "Windows/WindowsHWrapper.h"

//////////////////////////////////////////////
// routeType�� Ŭ�� -> �������� �ʿ��ϰ�
// ���� -> Ŭ�󿡼��� �ʿ������ Ŭ��� ����
//////////////////////////////////////////////
#pragma pack(push,1)
struct st_GAMELIB_LANHEADER
{
	uint16  s_len;
	uint16  s_serviceID;   // routeType�� ������ ��� ��ȿ. �Ʒ� ServiceID ���ӽ����̽� ����
	uint8   s_routeType;   // 0 : �׷� , 1 : ����, 2 : ����
}typedef GAMELIB_LANHEADER;
#pragma pack(pop)

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