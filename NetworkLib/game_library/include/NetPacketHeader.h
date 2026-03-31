#pragma once
#include <windows.h>

//////////////////////////////////////////////
// routeType�� Ŭ�� -> �������� �ʿ��ϰ�
// ���� -> Ŭ�󿡼��� �ʿ������ Ŭ��� ����
//////////////////////////////////////////////
#pragma pack(push,1)
struct st_GAMELIB_LANHEADER
{
	WORD  s_len;
	WORD  s_serviceID;   // routeType�� ������ ��� ��ȿ. �Ʒ� ServiceID ���ӽ����̽� ����
	BYTE  s_routeType;   // 0 : �׷� , 1 : ����, 2 : ����
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
	constexpr WORD NONE_SERVICE = 0;
	constexpr WORD CHAT_SERVICE = 1;
}