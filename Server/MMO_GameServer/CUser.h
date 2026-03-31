#pragma once
#include <string>
#include "GameDefine.h" 

using namespace UserConst;
using namespace std;

class CUser
{
public:
	enum class USER_ACTION : WORD
	{
		STOP = 0,
		MOVE = 1,
	};


public:
	CUser() = default;
	~CUser() = default;

public:
	UINT64         m_sessionID;          // 세션ID 겸 캐릭터 고유ID
	WORD           m_xpos;               // 캐릭터 X좌표
	WORD           m_ypos;               // 캐릭터 Y좌표
	WORD           m_hp;                 // 캐릭터 HP
	WORD           m_mp;                 // 캐릭터 MP
	WORD           m_sectorXpos;         // 캐릭터 섹터 X좌표
	WORD           m_sectorYpos;         // 캐릭터 섹터 Y좌표
	WORD           m_arrayIdx;           // 특정 섹터에 있는 배열의 몇번째 index에 있는지에 대한 정보
	USER_ACTION    m_action;             // 캐릭터가 현재 하는 행동
	WORD           m_inputMask;          // WSAD 입력 상태(이에 따라 캐릭터 시선 방향 결정)
	FLOAT          m_velocity;           // 캐릭터 속도
	DWORD          m_recvTime;           // 메세지 마지막 수신 시간
	WCHAR          m_nickName[NICK_MAX]; // 캐릭터 닉네임
};

