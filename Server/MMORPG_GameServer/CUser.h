#pragma once

#include "ContentsType.h"
#include "ContentsStruct.h"
#include "ContentsStruct.h"

class CUser : public IUser
{
public:
	CUser() = default;
	~CUser() = default;
	
	void   Init(uint64 sessionID);
	void   SkillInfoUpdate(uint16 skillIndex, uint32 curTime, bool bActivate);
	void   AddDef(uint16 amount) { m_def += amount; }
	uint16 GetDef();

	static CUser* Alloc();
	static void Free(CUser* pUser);

private:
	void DefUpdate();

public:
	SwingInfo           m_swingInfo;                                                       // 좌 클릭 공격 처리 관련 구조체
	SkillInfo           m_skillInfo[UserConst::USER_SKILL_SLOT_COUNT];             // 유저 스킬 쿨타임 처리 구조체
	float               m_xpos;                                                            // 캐릭터 X좌표
	float               m_ypos;                                                            // 캐릭터 Y좌표
	float               m_zpos;                                                            // 캐릭터 Z좌표
	bool                m_isFalling;                                                       // 낙하중인지(true면 점프 중, 이때 공격 불가)
	bool                m_moveFlag;                                                        // 정지, 이동 플래그
	uint16              m_atk;                                                             // 캐릭터 공격력
	uint16              m_def;                                                             // 캐릭터 방어력
	uint16              m_hp;                                                              // 캐릭터 HP
	uint16              m_maxHP;                                                           // 캐릭터 MaxHP
	uint16              m_mp;                                                              // 캐릭터 MP
	uint16              m_maxMP;                                                           // 캐릭터 MaxMP
	uint16              m_mpRegenPerSec;                                                   // 캐릭터 초당 마나 재생
	uint16              m_sectorXpos;                                                      // 캐릭터 섹터 X좌표
	uint16              m_sectorYpos;                                                      // 캐릭터 섹터 Y좌표
	uint16              m_arrayIdx;                                                        // 특정 섹터에 있는 배열의 몇번째 index에 있는지에 대한 정보
	uint16              m_syncCount;                                                       // 특정 시간동안 싱크 발생한 횟수
	float               m_movementYaw;                                                     // 캐릭터 이동 방향, 이동 처리시 사용
	float               m_maxWalkSpeed;                                                    // 캐릭터 최대 이동 속도(이벤트 발생시 변화 값)
	float               m_moveSpeed;                                                       // 캐릭터 이동 속도(고정 프레임 방식이라 프레임 당 이동량)
	uint32              m_recvTime;                                                        // 메세지 마지막 수신 시간
	uint32              m_jumpStartTime;                                                   // 점프 시작시간
	uint32              m_lastSyncCheckTime;                                               // 마지막 싱크 패킷 측정 시간
	                                                                                       // 서버가 저장한 lastswing 패킷 저장 시간 + Alpha보다 더 빠르게 도착한거면 비정상으로 판단하여 연결 끊기
	WCHAR               m_nickName[UserConst::NICK_MAX];                                   // 캐릭터 닉네임
	uint64              m_sessionID;                                                       // 게임 라이브러리가 전달한 세션 Key

private:
	static CMPoolTLS<CUser> m_userPool;
};

