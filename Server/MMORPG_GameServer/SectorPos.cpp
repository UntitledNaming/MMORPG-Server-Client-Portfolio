#include "ContentsDefine.h"
#include "SectorPos.h"

void SectorPos::SectorFind(SectorAround& pAround, const SectorPos& sec)
{
	int cnt = 0;

	int xarray[9] = { -1,0,1,-1,0,1,-1,0,1 };
	int yarray[9] = { -1,-1,-1,0,0,0,1,1,1 };

	for (int i = 0; i < 9; i++)
	{
		int sx = static_cast<int>(sec.m_secX) - xarray[i];
		int sy = static_cast<int>(sec.m_secY) - yarray[i];

		if (!SectorRangeCheck(SectorPos(sx, sy)))
			continue;

		pAround.m_Around[cnt].m_secX = sec.m_secX - xarray[i];
		pAround.m_Around[cnt].m_secY = sec.m_secY - yarray[i];
		cnt++;
	}

	pAround.m_count = cnt;
}

void SectorPos::CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector)
{
	bool curSecOverlapflag[9] = { false };
	SectorAround curSec;
	SectorFind(curSec, oldSecPos);


	bool newSecOverlapflag[9] = { false };
	SectorAround newSec;
	SectorFind(newSec, newSecPos);

	

	// 겹치는 좌표를 찾아서 이를 제외한 좌표값을 아웃 파라미터에 담기
	for (int i = 0; i < curSec.m_count; i++)
	{
		for (int j = 0; j < newSec.m_count; j++)
		{
			if (curSecOverlapflag[i] == true || newSecOverlapflag[j] == true)
				continue;

			if (curSec.m_Around[i].m_secX == newSec.m_Around[j].m_secX && curSec.m_Around[i].m_secY == newSec.m_Around[j].m_secY)
			{
				curSecOverlapflag[i] = true;
				newSecOverlapflag[j] = true;
			}
		}
	}

	// 아웃 파라미터에 담기
	int deletecount = 0;
	int createcount = 0;

	// 겹치지 않는 섹터 좌표가 지워질 영역
	for (int i = 0; i < curSec.m_count; i++)
	{
		if (curSecOverlapflag[i] == true)
			continue;

		outDeleteSector.m_Around[deletecount].m_secX = curSec.m_Around[i].m_secX;
		outDeleteSector.m_Around[deletecount].m_secY = curSec.m_Around[i].m_secY;
		deletecount++;

	}
	outDeleteSector.m_count = deletecount;

	// 겹치지 않는 섹터가 생성해야 할 영역
	for (int i = 0; i < newSec.m_count; i++)
	{
		if (newSecOverlapflag[i] == true)
			continue;

		outCreateSector.m_Around[createcount].m_secX = newSec.m_Around[i].m_secX;
		outCreateSector.m_Around[createcount].m_secY = newSec.m_Around[i].m_secY;
		createcount++;

	}
	outCreateSector.m_count = createcount;
}

bool SectorPos::SectorRangeCheck(const SectorPos& sec)
{
	if (sec.m_secX < 0 || sec.m_secY < 0 || sec.m_secX >= FieldConst::SECTOR_X_MAX || sec.m_secY >= FieldConst::SECTOR_Y_MAX)
		return false;

	return true;
}

bool SectorPos::IsAlreadyPushed(const SectorPos* arr, int count, int16 sx, int16 sy)
{
	for (int i = 0; i < count; ++i)
	{
		if (arr[i].GetX() == sx && arr[i].GetY() == sy)
			return true;
	}
	return false;
}

bool SectorPos::SameSector(const SectorPos& oldSec, const SectorPos& newSec)
{
	if (newSec.m_secX == oldSec.m_secX && newSec.m_secY == oldSec.m_secY)
		return true;

	return false;
}