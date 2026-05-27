#pragma once
#include "ContentsType.h"

struct SectorAround;

class SectorPos
{
public:
	SectorPos() : m_secX(0), m_secY(0) {}
	SectorPos(int16 xpos, int16 ypos) : m_secX(xpos), m_secY(ypos) {}

public:
	void   SetPos(const SectorPos& sec) { m_secX = sec.m_secX; m_secY = sec.m_secY; }
	int16 GetX() const { return m_secX; }
	int16 GetY() const { return m_secY; }

	static void SectorFind(SectorAround& pAround, const SectorPos& sec);
	static void CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector);
	static bool SectorRangeCheck(const SectorPos& sec);
	static bool IsAlreadyPushed(const SectorPos* arr, int count, int16 sx, int16 sy);
	static bool SameSector(const SectorPos& oldSec, const SectorPos& newSec);

private:
	int16 m_secX = 0;
	int16 m_secY = 0;
};

struct SectorAround
{
	SectorAround() = default;

	uint16    m_count = 0;
	SectorPos m_Around[9] = {};
};