#pragma once
#include <AuthGroup.cpp>

class FieldGroup : CGroup
{
private:

	struct st_Pos
	{
		WORD m_xpos;
		WORD m_ypos;
	}typedef Pos;

	struct st_UserArray
	{
		std::vector<CUser*> m_userTable;
		WORD                m_userCount;
	}typedef UserArray;

	struct st_Sector
	{
		UserArray m_userArray;
		// todo : 몬스터
		// todo : 아이템
	}typedef Sector;

	struct st_SectorAround
	{
		WORD m_count;
		Pos  m_Around[9];
	}typedef SectorAround;


public:
	FieldGroup() = default;
	~FieldGroup() = default;


	//////////////////////////////////////////////////////////////////////////////////
	// 그룹 콜백 함수
	//////////////////////////////////////////////////////////////////////////////////
	virtual void  Init(CGameLibrary* p) override;
	virtual void  Destroy() override;
	virtual void  OnClientJoin(UINT64 sessionID) override;
	virtual void  OnClientLeave(UINT64 sessionID) override;
	virtual void  OnRecv(UINT64 sessionID, CMessage* pMessage) override;
	virtual void  OnIUserMove(UINT64 sessionID, IUser* pUser) override;
	virtual void  OnUpdate() override;

	//////////////////////////////////////////////////////////////////////////////////
	// 섹터 관련 함수
	//////////////////////////////////////////////////////////////////////////////////
	bool SectorRangeCheck(WORD xpos, WORD ypos);
	void SectorFind(SectorAround& pAround, WORD xpos, WORD ypos);
	void SendPacket_SectorOne(CMessage* pMessage, WORD xpos, WORD ypos, CUser* pUser);     // 해당 섹터에 있는 유저들에게 메세지 보내기
	void SendPacket_SectorAround(CMessage* pMessage, CUser* pUser);                        // 해당 섹터에 있는 유저들에게 메세지 보내기

	///////////////////////////////////
    // Input Offset 처리 함수        //
    ///////////////////////////////////
	bool GetInputOffset(BYTE inputMask, FLOAT& outOffset);

	///////////////////////////////////
    // Degree 변환 함수              //
    ///////////////////////////////////
	constexpr FLOAT DegreeToRadian(FLOAT degree)
	{
		return degree * PI / 180.0f;
	}

	///////////////////////////////////
    // 컨텐츠 메세지 생성 함수       //
    ///////////////////////////////////
	void mpCreateMyCharacter(CUser* pUser, CMessage* pMessage);
	void mpCreateOtherCharacter(CUser* pUser, CMessage* pMessage);
	void mpDeleteCharacter(CUser* pUser, CMessage* pMessage);
	void mpCharacterInputUpdate(CUser* pUser, CMessage* pMessage);
	void mpSyncMyCharacterPosition(CUser* pUser, CMessage* pMessage);
	void mpSyncOtherCharacterPosition(CUser* pUser, CMessage* pMessage);

	///////////////////////////////////
    // 클라이언트 메세지 처리 핸들러 //
    ///////////////////////////////////
	void HandleCharacterInputUpdate(UINT64 sessionID, CMessage* pMessage);

	///////////////////////////////////
    // 프레임 로직 처리 함수         //
    ///////////////////////////////////
	void MovementProc();


private:
	std::unordered_map<UINT64, CUser*> m_userLookUpTable;
	Sector                             m_sectors[FieldConst::SECTOR_Y_MAX][FieldConst::SECTOR_X_MAX];
};

