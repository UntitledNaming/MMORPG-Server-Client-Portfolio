#pragma once
#include "ContentsType.h"

namespace AuthConst
{
	constexpr uint32 NONUSER_TIMEOUT = 3500;
}

namespace FieldConst
{
	constexpr uint32  UPDATE_LOOP_TIME                = 25;                          // 25ms Update Thread loop
	constexpr uint32  UPDATE_FRAME                    = 1000 / UPDATE_LOOP_TIME;     // Frame Loop Count per sec
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
	constexpr uint16  MAX_SECTOR_ITEM_COUNT           = 30;
	constexpr uint32  MAX_GROSS_FIELD_MONSTER_COUNT   = 2533;
	constexpr uint32  GROSS_FIELD_SECTOR_COUNT        = 2533;
	constexpr uint32  GROSS_FIELD_AREA_ARRAY_COUNT    = 115;
	constexpr uint16  SECTOR_SPAWN_OFFSETX            = 10;
	constexpr uint16  SECTOR_SPAWN_OFFSETY            = 10;
}

namespace UserConst
{
	constexpr uint16  NICK_MAX = 64;
	constexpr float   WALK_SPEED = 600.0; // 600 cm/s
	constexpr float   RUN_SPEED = 1200.0; // 1200 cm/s
	constexpr uint16  USER_SKILL_SLOT_COUNT = 4;
	constexpr uint16  USER_BUFF_SKILL_SLOT_COUNT = 1;
	constexpr uint16  USER_ACTIVE_SKILL_SLOT_COUNT = 3;
	constexpr uint16  USER_MAX_LEVEL = 10;
	constexpr uint32  USER_HP_REGEN_TIME = 1;
	constexpr uint32  USER_MP_REGEN_TIME = 1;
	constexpr uint32  USER_ITEM_SLOT_UPDATE_MIN_TIME = 30000;
	constexpr uint32  USER_ITEM_SLOT_UPDATE_MAX_TIME = 60000;
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
	//  Base Attack
	///////////////////////////////////////////////
	constexpr float  LEFATTACK_HALF_ANGLE = 60.0f;
	constexpr float  LEFTATTACK_RANGE = 250.0f;
	constexpr uint32 LEFTATTACK_MAX_HIT_COUNT = 2;
	constexpr uint32 LEFTATTACK_SWING_INTERVAL = 700;

	////////////////////////////////////////////////
	//  1 Skill
	////////////////////////////////////////////////
	constexpr uint16 BUFF_REQUIRED_MANA = 25;
	constexpr uint32 BUFF_COOLTIME_SEC  = 15000;
	constexpr uint32 BUFF_DURATION      = 7000;
	constexpr uint16 BUFF_DEF_ADD_AMOUNT    = 2;
	constexpr uint16 BUFF_ATK_ADD_AMOUNT    = 5;

	////////////////////////////////////////////////
    //  2 Skill
    ////////////////////////////////////////////////
	constexpr uint16 SPINSLASH_REQUIRED_MANA = 20;
	constexpr uint32 SPINSLASH_COOLTIME_SEC = 5000;
	constexpr uint32 SPINSLASH_MAX_HIT_COUNT = 4;
	constexpr float  SPINSLASH_RANGE = 400.0f;


	////////////////////////////////////////////////
    //  3 Skill
    ////////////////////////////////////////////////
	constexpr uint16 GROUNDSMASH_REQUIRED_MANA = 30;
	constexpr uint32 GROUNDSMASH_COOLTIME_SEC = 8000;
	constexpr uint32 GROUNDSMASH_MAX_HIT_COUNT = 8;
	constexpr float  GROUNDSMASH_RANGE = 600.0f;

	////////////////////////////////////////////////
	//  4 Skill
	////////////////////////////////////////////////
	constexpr uint16 ULTIMATE_REQUIRED_MANA = 80;
	constexpr uint32 ULTIMATE_COOLTIME_SEC = 120000;
	constexpr uint32 ULTIMATE_MAX_HIT_COUNT = 8;
	constexpr float  ULTIMATE_RANGE = 1200.0f;
}

namespace MonsterConst
{
	constexpr uint16  GET_EXP    = 35;
	constexpr uint16  BASE_ATK   = 15;
	constexpr uint16  BASE_DEF   = 3;
	constexpr uint16  BASE_HP    = 160;
	constexpr uint16  BASE_MAXHP = 160;
	constexpr uint16  BASE_EXP   = 35;
	constexpr float   POS_SNAP_DIST_CM = 300.0f;
	constexpr float   PATROL_SPEED = 300.0f;
	constexpr float   CHASE_SPEED = 550.0f;
	constexpr float   RETURN_SPEED = 350.0f;
	constexpr float   DETECT_RANGE = 1000.0f;
	constexpr float   ATTACK_RANGE = 200.0f;
	constexpr float   ATTACK_HALF_ANGLE = 60.0f;
	constexpr float   RETURN_RANGE = 1200.0f;
	constexpr float   PATROL_DISTANCE = 1200.0f;
	constexpr float   CHASE_STOP_DISTANCE = ATTACK_RANGE * 0.75f;
	constexpr float   TARGET_UPDATE_CM = 40.0f;
	constexpr uint32  ATTACK_COOLDOWN_MS = 1500;
	constexpr uint32  PATROL_PAUSE_MS    = 3000;
	constexpr uint32  IDLE_MIN_DURATION_MS = 2000;
	constexpr uint32  IDLE_MAX_DURATION_MS = 6000;
	constexpr float   CHASE_ANGLE_THRESHOLD = 15.0f;
	constexpr float   CHASE_REENTER_RANGE = 100;
	constexpr uint32  CHASE_UPDATE_MIN_MS = 200;
	constexpr uint32  MOVE_SYNC_INTERVAL_MS = 1000;
	constexpr uint32  COMBAT_EXIT_DELAY_MS = 400;
}

namespace UserItemStorage
{
	constexpr  int  MAX_ITEM_STORAGE_COUNT = 100;
}

namespace UserInventory
{
	constexpr uint16 INVENTORY_SLOT_MAX = 40;
}

namespace ItemUID
{
	constexpr uint64 ITEM_UID_INVALID_ID    = 0;
	constexpr uint64 ITEM_UID_RESERVE_COUNT = 100000;
	constexpr float  ITEM_RESERVE_PERCENT   = 80;
}

namespace UserQuickSlot
{
	constexpr uint8 QUICK_SLOT_MAX = 2;
}

namespace ItemIDConst
{
	// Consumable
	constexpr ITEM_ID SMALL_HP_POTION = 10001;
	constexpr ITEM_ID SMALL_MP_POTION = 10002;

	// Normal Equipment
	constexpr ITEM_ID NORMAL_HELMET   = 10003;
	constexpr ITEM_ID NORMAL_CHEST    = 10004;
	constexpr ITEM_ID NORMAL_PANTS    = 10005;
	constexpr ITEM_ID NORMAL_BOOTS    = 10006;
	constexpr ITEM_ID NORMAL_WEAPON   = 10007;

	// Magic Equipment
	constexpr ITEM_ID MAGIC_HELMET    = 10008;
	constexpr ITEM_ID MAGIC_CHEST     = 10009;
	constexpr ITEM_ID MAGIC_PANTS     = 10010;
	constexpr ITEM_ID MAGIC_BOOTS     = 10011;
	constexpr ITEM_ID MAGIC_WEAPON    = 10012;
}

namespace FieldDropItemConst
{
	constexpr uint16 FIELD_DROP_ITEM_RANDOM_STAT_MAX = 6;
	constexpr uint32 FIELD_DROP_ITEM_EXPIRED_TIME    = 60000;
	constexpr uint32 CREATE_SMALL_HP_POTION_MAX      = 5;
	constexpr uint32 CREATE_SMALL_MP_POTION_MAX      = 5;
	constexpr float  FIELD_DROP_PICKUP_RANGE         = 400.f;
}

namespace ConsumableConst
{
	constexpr uint32 HP_POTION_USE_COOLTIME_MS = 5000;
	constexpr uint32 MP_POTION_USE_COOLTIME_MS = 5000;
}