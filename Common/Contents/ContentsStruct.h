#pragma once
#include "ContentsType.h"
#include "ContentsEnum.h"

struct st_Position
{
	float m_xpos;
	float m_ypos;
	float m_zpos;
}typedef Position;

struct st_Stats
{
	uint16 m_hp;
	uint16 m_maxHP;
	uint16 m_mp;
	uint16 m_maxMP;
};

struct st_Movement
{
	bool m_moveFlag;
	bool m_isFalling;
	EM1MoveMode m_moveMode;
	float m_movementYaw;
	float m_maxWalkSpeed;
	float m_moveSpeed;
};

struct st_Sector
{
	uint16 m_sectorXpos;
	uint16 m_sectorYpos;
	uint16 m_arrayIdx;
}typedef SectorInfo;

struct st_SyncInfo
{
	uint16 m_syncCount;
	uint32 m_lastSyncCheckTime;
}typedef SyncInfo;

struct st_SwingInfo
{
	uint8  m_lastSwingIdx;
	uint32 m_lastSwingRecvTime;
}typedef SwingInfo;


struct st_Vector2
{
	float m_xpos;
	float m_ypos;
}typedef Vec2;

struct st_SecPos
{
	uint16 m_secXpos;
	uint16 m_secYpos;
}typedef SecPos;