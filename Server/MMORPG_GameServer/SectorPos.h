#pragma once
#include "ContentsType.h"

struct SectorAround;

class SectorPos
{
public:
	SectorPos() : m_secX(0), m_secY(0) {}
	SectorPos(uint16 xpos, uint16 ypos) : m_secX(xpos), m_secY(ypos) {}

public:
	void SetPos(const SectorPos& sec) { m_secX = sec.m_secX; m_secY = sec.m_secY; }
	uint16 GetX() const { return m_secX; }
	uint16 GetY() const { return m_secY; }

	static void SectorFind(SectorAround& pAround, const SectorPos& sec);
	static void CalSectorTransitionMessageTargets(const SectorPos& oldSecPos, const SectorPos& newSecPos, SectorAround& outDeleteSector, SectorAround& outCreateSector);
	static bool SectorRangeCheck(const SectorPos& sec);
	static bool IsAlreadyPushed(const SectorPos* arr, int count, uint16 sx, uint16 sy);
	static bool SameSector(const SectorPos& oldSec, const SectorPos& newSec);

private:
	uint16 m_secX;
	uint16 m_secY;
};

struct SectorAround
{
	SectorAround() = default;

	uint16 m_count;
	SectorPos m_Around[9];
};