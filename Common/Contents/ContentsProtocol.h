#pragma once

#include "ContentsType.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// 0    ~ 999  : Auth  Protocol
// 1000 ~ 1999 : Field Protocol
// 2000 ~ 2999 : Chat  Protocol
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace AuthProtocol
{
	constexpr uint16 PACKET_CS_CHAT_LOGIN_REQ = 0;
	constexpr uint16 PACKET_SC_CHAT_LOGIN_RES = 1;
}

namespace FieldProtocol
{
	constexpr uint16 PACKET_SC_CREATE_MY_CHARACTER = 1000;
    //---------------------------------------------------------------
    //  My Character Create Msg					Server -> Client
    //
    //
    //	8	-	CharacterID		(uint64)
    //	4	-	Xpos			(float)
    //	4	-	Ypos			(float)
    //  2   -   HP              (uint16)  
    //  2   -   MP              (uint16)
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
    //  4   -   CameraYawX      (float)  
    //  4   -   CameraYawY      (float)  
    //  2   -   HP              (uint16) 
    //  1   -   Action          (uint8)  
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


    constexpr uint16 PACKET_CS_CHARACTER_INPUT_UPDATE = 1003;
    //---------------------------------------------------------------
    // Character Input Data Update Msg          Client -> Server
    //
    // 
    //	4	-   Xpos		(float)
    //	4	-   Ypos		(float)
    //	4	-   CameraYawX	(float)
    //	4	-   CameraYawY	(float)
    //	1	-   InputMask	(uint8)
    //	1	-   Action  	(uint8)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_CHARACTER_INPUT_UPDATE = 1004;
    //---------------------------------------------------------------
    //  Character Input Data Update Msg         Server -> Client
    //
    // 
    //  8   -   CharacterID     (uint64)
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //	4	-   CameraYawX    	(float)
    //	4	-   CameraYawY    	(float)
    //	1	-   InputMask	    (uint8)
    //	1	-   Action  	    (uint8)
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_MY_CHARACTER_POS_SYNC = 1005;
    //---------------------------------------------------------------
    //  My Character Position Sync Msg         Server -> Client
    //
    // 
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_OTHER_CHARACTER_POS_SYNC = 1006;
    //---------------------------------------------------------------
    //  Other Character Position Sync Msg      Server -> Client
    //
    //
    // 
    //  8   -   CharacterID     (uint64)
    //	4	-   Xpos		    (float)
    //	4	-   Ypos		    (float)
    //
    //---------------------------------------------------------------
}