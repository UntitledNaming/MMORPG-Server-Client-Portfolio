#pragma once

#include "CoreMinimal.h" // 언리얼 기본 타입 포함

// --- 윈도우 타입 허용 매크로 시작 ---
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"

#include <winsock2.h>
#include <mswsock.h>
#include <string>
#include <thread>
#include "Ring_Buffer.h"
#include "LFQSingleLive.h"
#include "GameLibDefine.h"
#include "NetPacketHeader.h"

#define CLIENT_WSABUFSIZE   50
#define MAX_NUM_THREAD      5
#define CLIENT_WORKER_COUNT 2


class CMessage;


class CLanClient
{
public:
	struct st_SESSION
	{
		SOCKET                 s_Socket;
		OVERLAPPED             s_RecvOverlapped;
		OVERLAPPED             s_SendOverlapped;
		CRingBuffer            s_RecvQ;
		LFQueue<CMessage*>     s_SendQ;
		CMessage*              s_SendArray[CLIENT_WSABUFSIZE];

		int16                  s_SendFlag;
		int32                  s_SendMsgCnt;
		int32                  s_DCFlag;
		int32                  s_IORefCnt;
		int32                  s_RelFlag;
	}typedef SESSION;

public:
	CLanClient();
	virtual ~CLanClient();
	virtual void  Destroy();

	////////////////////////////////////////////////////
	// CLanClient �������̽� �Լ�
	////////////////////////////////////////////////////
	virtual void OnEnterJoinServer() = 0;                                              // ������ ���� ���� ��       
	virtual void OnLeaveServer() = 0;                                                  // ������ ���� ������ ��
	virtual void OnRecv(CMessage* pMessage) = 0;                                       // ��Ŷ ���� �Ϸ� ��
	virtual void OnSend(int sendsize) = 0;                                             // ��Ŷ �۽� �Ϸ� ��


	////////////////////////////////////////////////////
	// �ܺ� ���� �Լ�
	////////////////////////////////////////////////////
	bool  Connect(const WCHAR* SERVERIP, uint32 SERVERPORT);
	bool  Disconnect();
	bool  SendPacket(CMessage* pMessage, uint8 routeType, uint16 serviceID = ServiceID::NONE_SERVICE);
	bool  ReConnect();                                                                 // ConnectAlive �Լ��� ���� ���� ����� Ȯ�ε� �� ȣ���ϴ� �Լ�(Not Thread Safe Func)
	bool  ConnectAlive();

private:
	void  WorkerThread();


private:
	bool  Mem_Init(const WCHAR* ServerIP, uint32 ServerPort);
	bool  CreateAndSetSocket(SOCKET& outParam);                                        // true : ���� ���� �� �ɼ� ���� ����, false : ���� ���� �Ǵ� �ɼ� ���� ����
	bool  ConnectTry(SOCKET inputParam);                                               // true : ������ ���� ���� , false : ���� ����
	bool  RecvPost();              
	bool  SendPost();              
	void  Release(int refCnt);
	void  RecvIOProc(DWORD cbTransferred);
	void  SendIOProc(DWORD cbTransferred);
	void  Thread_Create();
	void  Thread_Destroy();
	void  Session_Init(SOCKET socket);


private:
	HANDLE                    m_iocp;
	uint32                    m_serverport;
	SESSION*                  m_pSession;
	std::thread               m_IOCPWorkerThread[MAX_NUM_THREAD];               
	std::wstring              m_serverIP;
	
};

#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"