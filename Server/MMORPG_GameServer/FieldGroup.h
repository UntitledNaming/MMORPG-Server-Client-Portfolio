#pragma once
#include "ContentsDefine.h"
#include "ContentsType.h"

class CUser;

class FieldGroup : public CGroup
{
private:

	struct st_Pos
	{
		uint16 m_xpos;
		uint16 m_ypos;
	}typedef Pos;

	struct st_UserArray
	{
		std::vector<CUser*> m_userTable;
		uint16              m_userCount;
	}typedef UserArray;

	struct st_Sector
	{
		UserArray m_userArray;
		// todo : 몬스터
		// todo : 아이템
	}typedef Sector;

	struct st_SectorAround
	{
		uint16 m_count;
		Pos    m_Around[9];
	}typedef SectorAround;


public:
	FieldGroup() = default;
	~FieldGroup() = default;

	size_t UserCount();

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
	bool SectorRangeCheck(uint16 xpos, uint16 ypos);
	void SectorFind(SectorAround& pAround, uint16 xpos, uint16 ypos);
	void SendPacket_SectorOne(CMessage* pMessage, uint16 xpos, uint16 ypos, CUser* pUser);     // 해당 섹터에 있는 유저들에게 메세지 보내기
	void SendPacket_SectorAround(CMessage* pMessage, CUser* pUser);                        // 해당 섹터에 있는 유저들에게 메세지 보내기

	///////////////////////////////////
    // Degree 변환 함수              //
    ///////////////////////////////////
	constexpr float DegreeToRadian(float degree)
	{
		return degree * FieldConst::Pi / 180.0f;
	}

	///////////////////////////////////
    // 컨텐츠 메세지 생성 함수       //
    ///////////////////////////////////
	void mpCreateMyCharacter(CUser* pUser, CMessage* pMessage);
	void mpCreateOtherCharacter(CUser* pUser, CMessage* pMessage);
	void mpDeleteCharacter(CUser* pUser, CMessage* pMessage);
	void mpCharacterMovementUpdate(CUser* pUser, CMessage* pMessage);
	void mpSyncMyCharacterPosition(CUser* pUser, CMessage* pMessage);
	void mpSyncOtherCharacterPosition(CUser* pUser, CMessage* pMessage);
	void mpRTTEchoMessage(CMessage* pMessage, double Time);

	///////////////////////////////////
    // 클라이언트 메세지 처리 핸들러 //
    ///////////////////////////////////
	void HandleCharacterMovementUpdate(uint64 sessionID, CMessage* pMessage);
	void HandleRTTMessage(uint64 sessionID, CMessage* pMessage);

	///////////////////////////////////
    // 프레임 로직 처리 함수         //
    ///////////////////////////////////
	void MovementProc();
	void SectorUpdate(CUser* pUser, uint16 nextXpos, uint16 nextYpos);

private:
	std::unordered_map<uint64, CUser*> m_userLookUpTable;
	Sector                             m_sectors[FieldConst::SECTOR_Y_MAX][FieldConst::SECTOR_X_MAX];

public:
	float xpos = 0;
	float ypos = 0;
	float zpos = 0;
	uint16 secxpos = 0;
	uint16 secypos = 0;
	uint64 fieldframe = 0;
};

