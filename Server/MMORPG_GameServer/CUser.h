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
	float               m_xpos;                          // 캐릭터 X좌표
	float               m_ypos;                          // 캐릭터 Y좌표
	float               m_zpos;                          // 캐릭터 Z좌표
	bool                m_isFalling;                     // 낙하중인지(true면 점프 중, 이때 공격 불가)
	bool                m_moveFlag;                      // 정지, 이동 플래그
	uint16              m_hp;                            // 캐릭터 HP
	uint16              m_maxHP;                         // 캐릭터 MaxHP
	uint16              m_mp;                            // 캐릭터 MP
	uint16              m_maxMP;                         // 캐릭터 MaxMP
	uint16              m_sectorXpos;                    // 캐릭터 섹터 X좌표
	uint16              m_sectorYpos;                    // 캐릭터 섹터 Y좌표
	uint16              m_arrayIdx;                      // 특정 섹터에 있는 배열의 몇번째 index에 있는지에 대한 정보
	uint16              m_syncCount;                     // 특정 시간동안 싱크 발생한 횟수
	EM1ActionStateType  m_action;                        // 캐릭터가 현재 하는 행동
	EM1MoveMode         m_moveMode;                      // 캐릭터 이동 타입(Walk, Run), Shift 누르면 Run
	float               m_movementYaw;                   // 캐릭터 이동 방향, 이동 처리시 사용
	float               m_maxWalkSpeed;                  // 캐릭터 최대 이동 속도(이벤트 발생시 변화 값)
	float               m_moveSpeed;                     // 캐릭터 이동 속도(고정 프레임 방식이라 프레임 당 이동량)
	uint32              m_recvTime;                      // 메세지 마지막 수신 시간
	uint32              m_jumpStartTime;                 // 점프 시작시간
	uint32              m_lastSyncCheckTime;             // 마지막 싱크 패킷 측정 시간
	WCHAR               m_nickName[UserConst::NICK_MAX]; // 캐릭터 닉네임
	uint64              m_sessionID;                     // 게임 라이브러리가 전달한 세션 Key

private:
	static CMPoolTLS<CUser> m_userPool;
};

