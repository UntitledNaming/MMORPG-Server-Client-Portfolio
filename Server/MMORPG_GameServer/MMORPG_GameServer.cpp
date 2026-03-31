#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include "GameLibDefine.h"

#include "CGroup.h"
#include "CService.h"
#include "AuthGroup.h"
#include "FieldGroup.h"
#include "ChatService.h"
#include "CDBManager.h"
#include "CUserDirectory.h"
#include "CGameLibrary.h"
#include "GameServer.h"

int main()
{
	BOOL endflag = false;
	GameServer server;

	while (!endflag)
	{
		Sleep(1000);

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8001)
		{
			endflag = true;
		}

	}

}

