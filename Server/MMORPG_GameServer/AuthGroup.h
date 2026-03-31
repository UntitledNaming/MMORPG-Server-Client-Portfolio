#pragma once

class AuthGroup :CGroup
{
public:
	AuthGroup() = default;
	~AuthGroup() = default;

	virtual void  Init(CGameLibrary* p) override;
	virtual void  Destroy() override;
	virtual void  OnClientJoin(UINT64 sessionID) override;
	virtual void  OnClientLeave(UINT64 sessionID) override;
	virtual void  OnRecv(UINT64 sessionID, CMessage* pMessage) override;
	virtual void  OnIUserMove(UINT64 sessionID, IUser* pUser) override;
	virtual void  OnUpdate() override;

	void LoginRequestProc(UINT64 sessionID, CMessage* pMessage);


private:
	std::unordered_map<UINT64, DWORD> m_nonuserTable;
};

