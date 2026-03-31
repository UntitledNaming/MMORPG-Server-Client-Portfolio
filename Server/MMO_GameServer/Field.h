#pragma once
#include "GameDefine.h"

using namespace FieldProtocol;
using namespace FieldConst;
using namespace std;

class Field : IModule
{
private:

	struct st_Pos
	{
		WORD m_xpos;
		WORD m_ypos;
	}typedef Pos;

	struct st_UserArray
	{
		vector<CUser*>         m_userTable;
		WORD                   m_count;
	}typedef UserArray;

	struct st_Sector
	{
		UserArray              m_userArray;
		SRWLOCK                m_sectorLock;
	}typedef Sector;

	struct st_SectorAround
	{
		WORD        m_count;
		Pos         m_Around[9];
	}typedef SectorAround;


public:
	Field() = default;
	~Field() = default;

	virtual void Init(ServerContext* ctx) override;
	virtual void Destroy() override;
	virtual void OnUserCreate(CUser* pUser) override;
	virtual void OnUserDelete(CUser* pUser) override;
	virtual void OnRecv(UINT64 sessionID, CMessage* pMessage) override;
	virtual void OnUpdate() override;

	///////////////////////////////////
    // 섹터 관련 함수                //
    ///////////////////////////////////
	bool SectorRangeCheck(WORD xpos, WORD ypos);
	void SectorFind(SectorAround& pAround, WORD xpos, WORD ypos);
	void SendPacket_SectorOne(CMessage* pMessage, WORD xpos, WORD ypos, CUser* pUser);     // 해당 섹터에 있는 유저들에게 메세지 보내기
	void SendPacket_SectorAround(CMessage* pMessage, CUser* pUser);
	void LSendPacket_SectorOne(CMessage* pMessage, WORD xpos, WORD ypos, CUser* pUser);    // Lock 걸고 해당 섹터에 있는 유저들에게 메세지 보내기
	void LSendPacket_SectorAround(CMessage* pMessage, CUser* pUser);

	///////////////////////////////////
    // 컨텐츠 메세지 생성 함수       //
    ///////////////////////////////////
	void mpCreateMyCharacter(CUser* pUser, CMessage* pMessage);
	void mpCreateOtherCharacter(CUser* pUser, CMessage* pMessage);
	void mpDeleteCharacter(CUser* pUser, CMessage* pMessage);
	void mpUpdateInputMask(CUser* pUser, CMessage* pMessage);

	///////////////////////////////////
    // 이동 처리 함수                //
    ///////////////////////////////////
	void HandleInputMask(UINT64 sessionID, CMessage* pMessage);

private:
	Sector      m_sectors[SECTOR_Y_MAX][SECTOR_X_MAX];

};

