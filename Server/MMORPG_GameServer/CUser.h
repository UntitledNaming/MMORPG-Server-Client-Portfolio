#pragma once

class CUser : public IUser
{
public:
	enum class USER_ACTION : BYTE
	{
		STOP = 0,
		WALK = 1,
		RUN  = 2,
	};

public:
	CUser() = default;
	~CUser() = default;
	
	void Init(UINT64 sessionID);

	static CUser* Alloc();
	static void Free(CUser* pUser);

public:
	FLOAT          m_xpos;                          // 캐릭터 X좌표
	FLOAT          m_ypos;                          // 캐릭터 Y좌표
	WORD           m_hp;                            // 캐릭터 HP
	WORD           m_mp;                            // 캐릭터 MP
	WORD           m_sectorXpos;                    // 캐릭터 섹터 X좌표
	WORD           m_sectorYpos;                    // 캐릭터 섹터 Y좌표
	WORD           m_arrayIdx;                      // 특정 섹터에 있는 배열의 몇번째 index에 있는지에 대한 정보
	USER_ACTION    m_action;                        // 캐릭터가 현재 하는 행동
	BYTE           m_inputMask;                     // WSAD 입력 상태
	FLOAT          m_cameraYaw;                     // 카메라 시선 방향(0 ~ 359 or -180 ~ 180), 이동 처리시 사용
	FLOAT          m_walkSpeed;                     // 캐릭터 걷기 속도(고정 프레임 방식이라 프레임 당 이동량 의미)
	FLOAT          m_runSpeed;                      // 캐릭터 달리기 속도(고정 프레임 방식이라 프레임 당 이동량 의미)
	DWORD          m_recvTime;                      // 메세지 마지막 수신 시간
	WCHAR          m_nickName[UserConst::NICK_MAX]; // 캐릭터 닉네임

private:
	static CMPoolTLS<CUser> m_userPool;
};

