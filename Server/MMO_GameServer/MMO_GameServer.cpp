#include <windows.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <Pdh.h>
#include <mysql.h>
#include "CUser.h"
#include "CPUUsage.h"
#include "ProcessMonitor.h"
#include "DumpClass.h"
#include "LogClass.h"
#include "TextParser.h"
#include "Ring_Buffer.h"
#include "DBTLS.h"
#include "myList.h"
#include "LockFreeMemoryPoolLive.h"
#include "MemoryPoolTLS.h"
#include "LFQSingleLive.h"
#include "LFQMultiLive.h"
#include "LFStack.h"
#include "CMessage.h"
#include "CSession.h"
#include "ServerContext.h"
#include "IModule.h"
#include "CLanServer.h"
#include "GameServer.h"

int main()
{
	GameServer server;
   
	// todo : 모듈 등록

	server.RunServer();
	while (1)
	{
		Sleep(1000);
	}
	server.StopServer();
}

