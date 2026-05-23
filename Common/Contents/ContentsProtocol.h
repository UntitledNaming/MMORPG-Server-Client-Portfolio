#pragma once

#include "ContentsType.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// 0    ~ 999  : Auth  Protocol
// 1000 ~ 1999 : Field Protocol
// 2000 ~ 2999 : Chat  Protocol
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ContentsProtocol
{
    constexpr uint16 MAX_PACKET_ID = 10000;
}

namespace AuthProtocol
{
	constexpr uint16 PACKET_CS_GAME_LOGIN_REQ = 0;
    //---------------------------------------------------------------
    //  Game Server Login Request Msg			Client -> Server
    //
    //	8	-	AccountNo		(uint64)
    //  64  -   TokenKey        (char[])
    //---------------------------------------------------------------


	constexpr uint16 PACKET_SC_GAME_LOGIN_RES = 1;
    //---------------------------------------------------------------
    //  Game Server Login Response Msg			Server -> Client
    //
    //	8	-	AccountNo		(uint64)
    //---------------------------------------------------------------
}

namespace FieldProtocol
{
	constexpr uint16 PACKET_SC_CREATE_MY_CHARACTER = 1000;
    //---------------------------------------------------------------
    //  My Character Create Msg					Server -> Client
    //
    //  8   -   CharacterID     (uint64) // Use for Hit Result Protocol
    //	4	-	Xpos			(float)
    //	4	-	Ypos			(float)
    //	4	-	Zpos			(float)
    //  2   -   HP              (uint16)  
    //  2   -   MaxHP           (uint16)  
    //  2   -   MP              (uint16)
    //  2   -   MaxMP           (uint16)
    //  2   -   MPRegenPerSec   (uint16)
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_CREATE_OTHER_CHARACTER = 1001;
    //---------------------------------------------------------------
    // 	Other Character Create Msg			   Server -> Client
    //
    //
    //	8	-	CharacterID		(uint64)
    //	4	-	Xpos			(float)
    //	4	-	Ypos			(float)
    //	4	-	Zpos			(float)
    //  4   -   MoveYaw         (float)  
    //  2   -   HP              (uint16) 
    //  2   -   MaxHP           (uint16) 
    //  1   -   MoveFlag        (bool)
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_DELETE_CHARACTER = 1002;
    //---------------------------------------------------------------
    // Character Delete Msg 					Server -> Client
    //
    //
    //	8	-	CharacterID		(uint64)
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_CS_UPDATE_CHARACTER_MOVEMENT_INPUT = 1003;
    //---------------------------------------------------------------
    // Character Input Data Update Msg          Client -> Server
    //
    // 
    //	4	-   Xpos		(float)
    //	4	-   Ypos		(float)
    //	4	-   Zpos		(float)
    //	4	-   MoveYaw     (float)
    //	1	-   MoveFlag 	(bool)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_UPDATE_CHARACTER_MOVEMENT_INPUT = 1004;
    //---------------------------------------------------------------
    //  Character Movement Data Update Msg         Server -> Client
    //
    // 
    //  8   -   CharacterID     (uint64)
    //  8   -   ServerTimestamp (uint64)           Server UTC ms
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //	4	-   Zpos		    (float)
    //	4	-   MoveYaw         (float)
    //	1	-   MoveFlag 	    (bool)
    //  
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_SYNC_MY_CHARACTER_POS = 1005;
    //---------------------------------------------------------------
    //  My Character Position Sync Msg         Server -> Client
    //
    // 
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //	4	-   Zpos		    (float)
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_SYNC_OTHER_CHARACTER_POS = 1006;
    //---------------------------------------------------------------
    //  Other Character Position Sync Msg      Server -> Client
    //
    // 
    //  8   -   CharacterID     (uint64)
    //  8   -   ServerTimestamp (uint64)        Server UTC ms
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //	4	-   Zpos		    (float)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_CHANGE_CHARACTER_MOVEMODE = 1007;
    //---------------------------------------------------------------
    //  Character MoveMode Change Msg         Client -> Server
    //
    //  1   -   MoveMode        (uint8)  
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_CHANGE_CHARACTER_MOVEMODE = 1008;
    //---------------------------------------------------------------
    //  Character MoveMode Change Msg         Server -> Client
    // 
    //  8   -   CharacterID     (uint64)
    //  1   -   MoveMode        (uint8)  
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_RTT_SEND = 1009;
    //---------------------------------------------------------------
    //  Character MoveMode Change Msg         Client -> Server
    // 
    //  8   -   Time            (uint64)  
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_RTT_ECHO = 1010;
    //---------------------------------------------------------------
    //  Character MoveMode Change Msg         Server -> Client
    //
    //  8   -   Time            (uint64)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_SWING_LEFT_ATTACK = 1011;
    //---------------------------------------------------------------
    //  Attack Start Msg                      Client -> Server
    //
    //  4   -   AttackYaw       (float)
    //  1   -   SwingIdx        (uint8)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_STOP_LEFT_ATTACK = 1012;
    //---------------------------------------------------------------
    //  Attack Stop Msg                       Client -> Server
    //
    //  (no payload)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_SWING_LEFT_ATTACK = 1013;
    //---------------------------------------------------------------
    //  Attack Start Broadcast                Server -> Client
    //
    //  8   -   CharacterID     (uint64)
    //  4   -   AttackYaw       (float)
    //  1   -   SwingIdx        (uint8)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_STOP_LEFT_ATTACK = 1014;
    //---------------------------------------------------------------
    //  Attack Stop Broadcast                 Server -> Client
    //
    //  8   -   CharacterID     (uint64)
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_ATTACK_HIT_RESULT = 1015;
    //---------------------------------------------------------------
    //  Attack Hit Result Broadcast           Server -> Client
    //
    //  1   -   PlayerHitCount    (uint8)
    //  1   -   MonsterHitCount   (uint8)
    //  per hit:
    //  8   -   CharacterID       (uint64)
    //  2   -   NewHP             (uint16)
    //      
    //  per hit:
    //  8   -   MonsterID         (uint64)
    //  2   -   NewHP             (uint16)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_USE_SKILL = 1016;
    //---------------------------------------------------------------
    //  Skill Use Request                     Client -> Server
    //
    //  1   -   SkillSlot       (uint8)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_USE_SKILL_RES = 1017;
    //---------------------------------------------------------------
    //  Skill Use Response                    Server -> Client
    //
    //  1   -   SkillSlot       (uint8)
    //  1   -   Success         (uint8)   1=success, 0=fail
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_USE_SKILL_BROADCAST = 1018;
    //---------------------------------------------------------------
    //  Skill Use Broadcast                   Server -> Client
    //
    //  8   -   CharacterID     (uint64)
    //  1   -   SkillSlot       (uint8)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_CREATE_MONSTER = 1019;
    //---------------------------------------------------------------
    // 	Monster Create Msg      			   Server -> Client
    //
    //	8	-	MonsterID		(uint64)
    //	4	-	Xpos			(float)
    //	4	-	Ypos			(float)
    //	4	-	Zpos			(float)
    //  4   -   Yaw             (float)  
    //  2   -   MonsterType     (uint16)
    //  2   -   HP              (uint16) 
    //  2   -   MaxHP           (uint16) 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_DELETE_MONSTER = 1020;
    //---------------------------------------------------------------
    //  Monster Delete Msg 					Server -> Client
    //
    //
    //	8	-	MonsterID		(uint64)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_MOVE_MONSTER = 1021;
    //---------------------------------------------------------------
    // Monster Move Msg 					Server -> Client
    //
    //
    //	8	-	MonsterID		(uint64)
    //	4	-	Xpos    		(float)
    //	4	-	Ypos    		(float)
    //	4	-	Zpos    		(float)
    //	4	-	MoveYaw 		(float)
    //	4	-	MoveSpeed 		(float)
    //	4	-	DesXpos  		(float)
    //	4	-	DesYpos 		(float)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_HIT_TOPLAYER = 1022;
    //---------------------------------------------------------------
    //  Monster Attack Msg 					Server -> Client
    //
    //
    //	8	-	MonsterID		(uint64)
    //	8	-	TargetID	 	(uint64)
    //  4   -   AttackYaw       (float)
    //	2	-	TargetNewHP 	(uint16)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_RESPAWN_PLAYER = 1023;
    //---------------------------------------------------------------
    // Character Delete Msg 					Client -> Server
    //
    //  (no payload)
    // 
    //---------------------------------------------------------------
}

