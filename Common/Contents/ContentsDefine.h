#pragma once
#include "ContentsType.h"

namespace AuthConst
{
	constexpr uint32 NONUSER_TIMEOUT = 3500;
}

namespace FieldConst
{
	constexpr uint32  UPDATE_LOOP_TIME          = 25;                          // 25ms Update Thread loop
	constexpr uint32  UPDATE_FRAME              = 1000 / UPDATE_LOOP_TIME;     // Frame Loop Count per sec
	constexpr uint32  USER_TIMEOUT                    = 40000;
	constexpr uint32  MAP_WORLD_OFFSET_X              = 200000;                      // (0,0) Sector Position X
	constexpr uint32  MAP_WORLD_OFFSET_Y              = 200000;                      // (0,0) Sector Position Y
	constexpr uint16  SECTOR_SIZE                     = 2500;                        // Sector Size : 25m
	constexpr uint16  SECTOR_USER_DEFAULT_COUNT       = 100;
	constexpr uint16  SECTOR_X_MAX                    = 160; 
	constexpr uint16  SECTOR_Y_MAX                    = 160;
	constexpr uint16  SYNC_MAX_COUNT                  = 15;                          // Max Sync Count per SYNC_COUNT_WINDOW_MS
	constexpr uint32  SYNC_COUNT_WINDOW_MS            = 10000;                       // Sync Time Window
	constexpr float   SYNC_X_RANGE                    = 500;                         // Sync Range : 5m
	constexpr float   SYNC_Y_RANGE                    = 500;
	constexpr float   Pi                              = 3.1415926535f;
	constexpr uint16  MAX_SECTOR_USER_COUNT           = 30;
	constexpr uint16  MAX_SECTOR_MONSTER_COUNT        = 30;
	constexpr uint32  MAX_GROSS_FIELD_MONSTER_COUNT   = 2500;
	constexpr uint32  GROSS_FIELD_SECTOR_COUNT        = 2500;
}

namespace UserConst
{
	constexpr uint16  NICK_MAX = 64;
	constexpr float   WALK_SPEED = 600.0; // 600 cm/s
	constexpr float   RUN_SPEED = 1200.0; // 1200 cm/s
	constexpr float   JUMP_ANIMATION_TIME = 0.0f;
	constexpr uint16  WARRIOR_MANA_REGEN = 5;
	constexpr uint16  USER_SKILL_SLOT_COUNT = 4;
	constexpr uint16  USER_BUFF_SKILL_SLOT_COUNT = 1;
	constexpr uint16  USER_ACTIVE_SKILL_SLOT_COUNT = 3;
	constexpr uint16  BASE_MAXHP = 100;
	constexpr uint16  BASE_MAXMP = 100;
	constexpr uint16  BASE_ATK = 5;
	constexpr uint16  BASE_DEF = 1;
}

namespace ClientMovement
{
	constexpr float MOVEMENT_SEND_INTERNAL_SEC      = 0.1f;
	constexpr float MOVEMENT_YAW_SEND_THRESHOLD_DEG = 5.0f;
	constexpr float REMOTE_PLAYER_POS_SNAP_DIST_CM = 200.0f;
	constexpr float SOFT_CORRECTION_STOP_DIST_CM = 5.0f;
}

namespace SnapShotProc
{
	constexpr float INTERP_DELAY_MS = 150.f;
	constexpr float CORRECTION_INTERP_SPEED = 10.f;
}

namespace ClientAttack
{
	constexpr uint16  MaxUserCount = 32;
	constexpr uint16  MaxMonsterCount = 32;

	///////////////////////////////////////////////
	//  평타
	///////////////////////////////////////////////
	constexpr float  LEFATTACK_HALF_ANGLE = 60.0f;
	constexpr float  LEFTATTACK_RANGE = 250.0f;
	constexpr uint32 LEFTATTACK_MAX_HIT_COUNT = 2;
	constexpr uint32 LEFTATTACK_SWING_INTERVAL = 600;

	////////////////////////////////////////////////
	//  1번 스킬
	////////////////////////////////////////////////
	constexpr uint16 BUFF_REQUIRED_MANA = 25;
	constexpr uint32 BUFF_COOLTIME_SEC  = 15000;
	constexpr uint32 BUFF_DURATION      = 7000;
	constexpr uint16 BUFF_DEF_ADD_AMOUNT    = 2;
	constexpr uint16 BUFF_ATK_ADD_AMOUNT    = 5;

	////////////////////////////////////////////////
    //  2번 스킬
    ////////////////////////////////////////////////
	constexpr uint16 SPINSLASH_REQUIRED_MANA = 10;
	constexpr uint32 SPINSLASH_COOLTIME_SEC = 7000;
	constexpr uint32 SPINSLASH_MAX_HIT_COUNT = 4;
	constexpr float  SPINSLASH_RANGE = 400.0f;


	////////////////////////////////////////////////
    //  3번 스킬
    ////////////////////////////////////////////////
	constexpr uint16 GROUNDSMASH_REQUIRED_MANA = 30;
	constexpr uint32 GROUNDSMASH_COOLTIME_SEC = 20000;
	constexpr uint32 GROUNDSMASH_MAX_HIT_COUNT = 8;
	constexpr float  GROUNDSMASH_RANGE = 600.0f;

	////////////////////////////////////////////////
	//  4번 스킬
	////////////////////////////////////////////////
	constexpr uint16 ULTIMATE_REQUIRED_MANA = 80;
	constexpr uint32 ULTIMATE_COOLTIME_SEC = 120000;
	constexpr uint32 ULTIMATE_MAX_HIT_COUNT = 8;
	constexpr float  ULTIMATE_RANGE = 1200.0f;
}

namespace MonsterConst
{
	constexpr uint16  BASE_HP    = 50;
	constexpr uint16  BASE_MAXHP = 50;
	constexpr float   WALK_SPEED = 500.0; // 500 cm/s
}