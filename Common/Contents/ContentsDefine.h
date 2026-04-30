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
	constexpr uint32  USER_TIMEOUT              = 40000;
	constexpr uint32  MAP_WORLD_OFFSET_X        = 201600;                      // (0,0) Sector Position X
	constexpr uint32  MAP_WORLD_OFFSET_Y        = 201600;                      // (0,0) Sector Position Y
	constexpr uint16  SECTOR_SIZE               = 10000;                       // Sector Size : 100m
	constexpr uint16  SECTOR_USER_DEFAULT_COUNT = 100;
	constexpr uint16  SECTOR_X_MAX              = 40; 
	constexpr uint16  SECTOR_Y_MAX              = 40;
	constexpr uint16  SYNC_MAX_COUNT            = 15;                          // Max Sync Count per SYNC_COUNT_WINDOW_MS
	constexpr uint32  SYNC_COUNT_WINDOW_MS      = 10000;                       // Sync Time Window
	constexpr float   SYNC_X_RANGE              = 500;                         // Sync Range : 10m
	constexpr float   SYNC_Y_RANGE              = 500;
	constexpr float   Pi                        = 3.1415926535f;
}

namespace UserConst
{
	constexpr uint16  NICK_MAX = 64;
	constexpr float   WALK_SPEED = 600.0; // 600 cm/s
	constexpr float   RUN_SPEED = 1200.0; // 1200 cm/s
	constexpr float   JUMP_ANIMATION_TIME = 0.0f;
}

namespace ClientMovement
{
	constexpr float MOVEMENT_SEND_INTERNAL_SEC      = 0.1f;
	constexpr float MOVEMENT_YAW_SEND_THRESHOLD_DEG = 5.0f;
	constexpr float REMOTE_PLAYER_POS_SNAP_DIST_CM = 150.0f;
	constexpr float SOFT_CORRECTION_STOP_DIST_CM = 5.0f;
}