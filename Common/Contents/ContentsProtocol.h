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
    //	4	-	Xpos			(float)
    //	4	-	Ypos			(float)
    //  2   -   HP              (uint16)  
    //  2   -   MaxHP           (uint16)  
    //  2   -   MP              (uint16)
    //  2   -   MaxMP           (uint16)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_CREATE_OTHER_CHARACTER = 1001;
    //---------------------------------------------------------------
    // 	Other Character Create Msg			   Server -> Client
    //
    // 
    //
    //	8	-	CharacterID		(uint64)
    //	4	-	Xpos			(float)
    //	4	-	Ypos			(float)
    //  4   -   MoveYaw         (float)  
    //  4   -   MoveSpeed       (uint32)  
    //  2   -   HP              (uint16) 
    //  2   -   MaxHP           (uint16) 
    //  1   -   Action          (uint8)  
    //  1   -   MoveMode        (uint8)  
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
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //	4	-   MoveYaw         (float)
    //	4	-   MoveSpeed       (uint32)
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


    constexpr uint16 PACKET_SC_CHANGE_CHARACTER_MOVEMODE = 1007;
    //---------------------------------------------------------------
    //  Character MoveMode Change Msg         Server -> Client
    // 
    //  8   -   CharacterID     (uint64)
    //  1   -   MoveMode        (uint8)  
    //
    //---------------------------------------------------------------


}