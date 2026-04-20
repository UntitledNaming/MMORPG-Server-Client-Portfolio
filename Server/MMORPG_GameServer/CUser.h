#pragma once

#include "ContentsType.h"

class CUser : public IUser
{
public:

public:
	CUser() = default;
	~CUser() = default;
	
	void Init(uint64 sessionID);

	static CUser* Alloc();
	static void Free(CUser* pUser);

public:
	float          m_xpos;                          // 캐릭터 X좌표
	float          m_ypos;                          // 캐릭터 Y좌표
	bool           m_isFalling;                     // 낙하중인지(true면 점프 중)
	uint16         m_hp;                            // 캐릭터 HP
	uint16         m_mp;                            // 캐릭터 MP
	uint16         m_sectorXpos;                    // 캐릭터 섹터 X좌표
	uint16         m_sectorYpos;                    // 캐릭터 섹터 Y좌표
	uint16         m_arrayIdx;                      // 특정 섹터에 있는 배열의 몇번째 index에 있는지에 대한 정보
	uint8          m_action;                        // 캐릭터가 현재 하는 행동
	uint8          m_inputMask;                     // WSAD 입력 상태
	float          m_cameraYaw;                     // 카메라 시선 방향(0 ~ 359 or -180 ~ 180), 이동 처리시 사용
	float          m_walkSpeed;                     // 캐릭터 걷기 속도(고정 프레임 방식이라 프레임 당 이동량 의미)
	float          m_runSpeed;                      // 캐릭터 달리기 속도(고정 프레임 방식이라 프레임 당 이동량 의미)
	uint32         m_recvTime;                      // 메세지 마지막 수신 시간
	WCHAR          m_nickName[UserConst::NICK_MAX]; // 캐릭터 닉네임
	uint64         m_sessionID;                     // 게임 라이브러리가 전달한 세션 Key

private:
	static CMPoolTLS<CUser> m_userPool;
};

