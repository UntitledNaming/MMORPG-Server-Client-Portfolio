#pragma once
#include "ContentsType.h"

class AuthGroup : public CGroup
{
public:
	AuthGroup() = default;
	~AuthGroup() = default;

	size_t  GetNonUserCount() { return m_nonuserTable.size(); }
	virtual void  Init(CGameLibrary* p) override;
	virtual void  Destroy() override;
	virtual void  OnClientJoin(uint64 sessionID) override;
	virtual void  OnClientLeave(uint64 sessionID) override;
	virtual void  OnRecv(uint64 sessionID, CMessage* pMessage) override;
	virtual void  OnIUserMove(uint64 sessionID, IUser* pUser) override;
	virtual void  OnUpdate() override;

	void LoginRequestProc(uint64 sessionID, CMessage* pMessage);
	void CharacterSelectProc(uint64 sessionID, CMessage* pMessage);

private:
	std::unordered_map<uint64, DWORD>  m_nonuserTable;
	std::unordered_map<uint64, CUser*> m_userTable;
};

