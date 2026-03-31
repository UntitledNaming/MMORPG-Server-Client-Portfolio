#pragma once
#include <windows.h>



namespace GameServerConst
{
	constexpr UINT16 MODULE_MAX_COUNT = 100;
	constexpr UINT16 PROTOCAL_RANGE   = 1000;
	constexpr UINT   NONUSER_TIMEOUT  = 4000;
	constexpr UINT   USER_TIMEOUT     = 40000;
}

namespace AuthConst
{
	constexpr UINT TOKEN_KEY_MAX = 64;
}

namespace FieldConst
{
	constexpr UINT SECTOR_SIZE               = 100;
	constexpr UINT SECTOR_USER_DEFAULT_COUNT = 100;
	constexpr WORD SECTOR_Y_MAX              = 50;
	constexpr WORD SECTOR_X_MAX              = 50;
}

namespace UserConst
{
	constexpr UCHAR NICK_MAX = 64;
	constexpr UCHAR ID_MAX   = 64;
}

namespace InputMask
{
    constexpr WORD None = 1 << 0;
    constexpr WORD North = 1 << 1;
    constexpr WORD South = 1 << 2;
    constexpr WORD East = 1 << 3;
    constexpr WORD West = 1 << 4;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//    <�������� ����>
// 0    ~ 999  : ���� ���
// 1000 ~ 1999 : �ʵ� ���
// 2000 ~ 2999 : ä�� ���
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace AuthProtocol
{
	constexpr WORD PACKET_CS_LOGIN_REQ = 0;
}

namespace FieldProtocol
{
	constexpr WORD PACKET_SC_CREATE_MY_CHARACTER    = 1000;
	//---------------------------------------------------------------
    // ���� ĳ���� ����					Server -> Client
    //
    // ������ ������ �������� ĳ���� ���� �޼����� ����
    //
    //	8	-	CharacterID		(UINT64)
    //	2	-	Xpos			(WORD)
    //	2	-	Ypos			(WORD)
    //  2   -   HP              (WORD)  
    //  2   -   MP              (WORD)
    //
    //---------------------------------------------------------------
    

	constexpr WORD PACKET_SC_CREATE_OTHER_CHARACTER = 1001;
    //---------------------------------------------------------------
    // �ٸ� ĳ���� ����					Server -> Client
    //
    // ������ �� �������� �ٸ� ���� ĳ���� ���� �޼����� ����
    //
    //	8	-	CharacterID		(UINT64)
    //	2	-	Xpos			(WORD)
    //	2	-	Ypos			(WORD)
    //  2   -   HP              (WORD)  
    //  2   -   Action          (WORD)  
    //  2   -   InputMask       (WORD)  
    //
    //---------------------------------------------------------------


	constexpr WORD PACKET_SC_DELETE_CHARACTER       = 1002;
    //---------------------------------------------------------------
    // ĳ���� ����					Server -> Client
    //
    // ������ �� �������� ID�� �����ϴ� ĳ���� ���� �޼��� ����
    //
    //	8	-	CharacterID		(UINT64)
    //
    //---------------------------------------------------------------


	constexpr WORD PACKET_CS_INPUT_UPDATE             = 1003;
    //---------------------------------------------------------------
    // ĳ������ Input�� ����					Client -> Server
    //
    // Ŭ�� Input�� �����ϸ� ������ ������ ��������
    // ��ġ ��ǥ�� �ʿ������ ������� ���� �ʿ���.
    // 
    //	2	-	InputMask		(WORD)
    //	2	-	Xpos	    	(WORD) 
    //	2	-	Ypos		    (WORD)
    //  
    //---------------------------------------------------------------


    constexpr WORD PACKET_SC_INPUT_UPDATE = 1005;
    //---------------------------------------------------------------
    // ĳ���� Input ������ ����				   Server -> Client
    //
    // ������ ���ŵ� Ŭ���̾�Ʈ�� Input�� �ֺ� Ŭ���̾�Ʈ���� �����ϴ� ��������
    // ������ ��ǥ�� Ŭ�� �ٶ󺸴� �ش� ������ ��ǥ�� �Ȱ��� ����. ������ ���� �� ������ 
    // ������ ��ǥ�� ���� ���� �����ϴµ� ����ϰ� ��.
    // 
    //  8   -   CharacterID     (UINT64)
    //	2	-	InputMask		(WORD)
    //	2	-	Xpos	    	(WORD) 
    //	2	-	Ypos		    (WORD)
    //  
    //---------------------------------------------------------------

}

namespace ChatProtocol
{

}