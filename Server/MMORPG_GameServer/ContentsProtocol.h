#pragma once
#include <windows.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//    <프로토콜 범위>
// 0    ~ 999  : 인증 그룹
// 1000 ~ 1999 : 필드 그룹
// 2000 ~ 2999 : 채팅 서비스
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace AuthProtocol
{
	constexpr WORD PACKET_CS_CHAT_LOGIN_REQ = 0;
	constexpr WORD PACKET_SC_CHAT_LOGIN_RES = 1;
}

namespace FieldProtocol
{
	constexpr WORD PACKET_SC_CREATE_MY_CHARACTER = 1000;
    //---------------------------------------------------------------
    // 본인 캐릭터 생성					Server -> Client
    //
    // 서버가 접속한 유저에게 캐릭터 생성 메세지를 전송
    //
    //	8	-	CharacterID		(UINT64)
    //	4	-	Xpos			(FLOAT)
    //	4	-	Ypos			(FLOAT)
    //  2   -   HP              (WORD)  
    //  2   -   MP              (WORD)
    //
    //---------------------------------------------------------------

    constexpr WORD PACKET_SC_CREATE_OTHER_CHARACTER = 1001;
    //---------------------------------------------------------------
    // 다른 캐릭터 생성					Server -> Client
    //
    // 서버가 이 유저에게 다른 유저 캐릭터 생성 메세지를 전송
    //
    //	8	-	CharacterID		(UINT64)
    //	4	-	Xpos			(FLOAT)
    //	4	-	Ypos			(FLOAT)
    //  4   -   CameraYaw       (FLOAT)  
    //  2   -   HP              (WORD) 
    //  1   -   Action          (BYTE)  
    //
    //---------------------------------------------------------------


    constexpr WORD PACKET_SC_DELETE_CHARACTER = 1002;
    //---------------------------------------------------------------
    // 캐릭터 삭제					Server -> Client
    //
    // 서버가 이 유저에게 ID에 대응하는 캐릭터 삭제 메세지 전송
    //
    //	8	-	CharacterID		(UINT64)
    //
    //---------------------------------------------------------------


    constexpr WORD PACKET_CS_CHARACTER_INPUT_UPDATE = 1003;
    //---------------------------------------------------------------
    // 캐릭터 Input 이벤트 발생     Client -> Server
    //
    // 클라이언트에서 이벤트 발생 감지하여 Action, InputMask, Yaw, Pos 값을 보내면 이를 검증하고 반영함.
    // 
    //	4	-   Xpos		(FLOAT)
    //	4	-   Ypos		(FLOAT)
    //	4	-   CameraYaw	(FLOAT)
    //	1	-   InputMask	(BYTE)
    //	1	-   Action  	(BYTE)
    //
    //---------------------------------------------------------------

    constexpr WORD PACKET_SC_CHARACTER_INPUT_UPDATE = 1004;
    //---------------------------------------------------------------
    // 캐릭터 Input 이벤트 발생     Server -> Client
    //
    // 클라이언트에서 이벤트 발생 감지하여 Action, InputMask, Yaw, Pos 값을 보내면 이를 검증하고 반영하고
    // 다른 클라이언트들에게 해당 Input값 통지
    // 
    //  8   -   CharacterID     (UINT64)
    //	4	-   Xpos		    (FLOAT)
    //	4	-   Ypos		    (FLOAT)
    //	4	-   CameraYaw    	(FLOAT)
    //	1	-   InputMask	    (BYTE)
    //	1	-   Action  	    (BYTE)
    //
    //---------------------------------------------------------------


    constexpr WORD PACKET_SC_MY_CHARACTER_POS_SYNC = 1005;
    //---------------------------------------------------------------
    // 캐릭터 좌표 차이 발생     Server -> Client
    //
    // 클라이언트에서 보낸 좌표와 서버에서 바라본 해당 캐릭터의 좌표가 크게 차이날 경우 
    // 해당 클라이언트 포함 주변 섹터에 해당 캐릭터에 대한 싱크 패킷을 보냄.
    // 
    //	4	-   Xpos		    (FLOAT)
    //	4	-   Ypos		    (FLOAT)
    //
    //---------------------------------------------------------------


    constexpr WORD PACKET_SC_OTHER_CHARACTER_POS_SYNC = 1006;
    //---------------------------------------------------------------
    // 캐릭터 좌표 차이 발생     Server -> Client
    //
    // 클라이언트에서 보낸 좌표와 서버에서 바라본 해당 캐릭터의 좌표가 크게 차이날 경우 
    // 해당 클라이언트 포함 주변 섹터에 해당 캐릭터에 대한 싱크 패킷을 보냄.
    // 
    //  8   -   CharacterID     (UINT64)
    //	4	-   Xpos		    (FLOAT)
    //	4	-   Ypos		    (FLOAT)
    //
    //---------------------------------------------------------------
}