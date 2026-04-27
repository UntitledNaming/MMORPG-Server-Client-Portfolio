// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentsProtocol.h"

class CMessage;
class UM1NetworkManager;

class M1_API M1PacketHandler
{
public:
	M1PacketHandler();
	~M1PacketHandler();

public:
	// 서버가 보낸 "로그인 응답" 처리
	static void Handle_SC_LOGIN_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "내 캐릭터 생성" 처리
	static void Handle_SC_CREATE_MY_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "다른 캐릭터 생성" 처리
	static void Handle_SC_CREATE_0THER_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "캐릭터 삭제" 처리
	static void Handle_SC_DELETE_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "이동 업데이트" 처리
	static void Handle_SC_UPDATE_CHARACTER_INPUT(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "내 캐릭터 위치 싱크" 처리
	static void Handle_SC_SYNC_MY_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "다른 캐릭터 위치 싱크" 처리
	static void Handle_SC_SYNC_OTHER_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "RTT 응답" 처리
	static void Handle_SC_RTT_ECHO(CMessage* pMessage, UM1NetworkManager* NetworkManager);
};
