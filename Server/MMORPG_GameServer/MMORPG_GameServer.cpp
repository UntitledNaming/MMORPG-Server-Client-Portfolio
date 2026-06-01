#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <unordered_map>
#include "ContentsDefine.h"
#include "ContentsEnum.h"
#include "ContentsStruct.h"
#include "GameLibDefine.h"
#include "CGroup.h"
#include "CService.h"
#include "CGameLibrary.h"
#include "AuthGroup.h"
#include "FieldSector.h"
#include "SectorPos.h"
#include "CMonster.h"
#include "FieldGroup.h"
#include "CDBManager.h"
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

