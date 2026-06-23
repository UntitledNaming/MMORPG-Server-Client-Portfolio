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

	// 서버가 보낸 "캐릭터 선택 응답" 처리
	static void Handle_SC_CHARACTER_SELECT_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "내 캐릭터 생성" 처리
	static void Handle_SC_CREATE_MY_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "다른 캐릭터 생성" 처리
	static void Handle_SC_CREATE_0THER_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "캐릭터 삭제" 처리
	static void Handle_SC_DELETE_CHARACTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "이동 업데이트" 처리
	static void Handle_SC_UPDATE_CHARACTER_MOVEMENT_INPUT(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "내 캐릭터 위치 싱크" 처리
	static void Handle_SC_SYNC_MY_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "다른 캐릭터 위치 싱크" 처리
	static void Handle_SC_SYNC_OTHER_CHARACTER_POS(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "RTT 응답" 처리
	static void Handle_SC_RTT_ECHO(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "공격 시작" 처리
	static void Handle_SC_SWING_ATTACK(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "피격 결과 (HP 갱신)" 처리
	static void Handle_SC_ATTACK_HIT_RESULT(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "스킬 사용 응답 (로컬 플레이어)" 처리
	static void Handle_SC_USE_SKILL_RES(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "스킬 사용 브로드캐스트 (타 캐릭터)" 처리
	static void Handle_SC_USE_SKILL_BROADCAST(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "몬스터 생성" 처리
	static void Handle_SC_CREATE_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "몬스터 삭제" 처리
	static void Handle_SC_DELETE_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "몬스터 이동" 처리
	static void Handle_SC_MOVE_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "몬스터 정지" 처리
	static void Handle_SC_STOP_MONSTER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "몬스터 공격" 처리 (타겟 본인 = 절대 HP)
	static void Handle_SC_HIT_TOPLAYER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "몬스터 공격(관전)" 처리 (타인 = ratio overhead)
	static void Handle_SC_HIT_TO_OTHERPLAYER(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "이동 모드 변경" 처리
	static void Handle_SC_CHANGE_CHARACTER_MOVEMODE(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "필드 드랍 아이템 생성" 처리
	static void Handle_SC_CREATE_FIELD_DROP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "필드 드랍 아이템 삭제" 처리
	static void Handle_SC_DELETE_FIELD_DROP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "장비 아이템 픽업" 처리
	static void Handle_SC_PICKUP_EQUIPMENT_ITEMS(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "소비 아이템 픽업" 처리
	static void Handle_SC_PICKUP_CONSUMABLE_ITEMS(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "소비 아이템 사용" 처리
	static void Handle_SC_USE_CONSUMABLE_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "장비 장착" 처리
	static void Handle_SC_EQUIP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "장비 해제" 처리
	static void Handle_SC_UNEQUIP_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "아이템 삭제" 처리
	static void Handle_SC_DELETE_ITEM(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "슬롯 교환" 처리
	static void Handle_SC_SWAP_SLOT(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "경험치 획득 및 레벨업" 처리
	static void Handle_SC_GAIN_EXP(CMessage* pMessage, UM1NetworkManager* NetworkManager);

	// 서버가 보낸 "리스폰" 처리(본인: {HP,MP} / 주변: {CharacterID})
	static void Handle_SC_RESPAWN_RES_TO_ME(CMessage* pMessage, UM1NetworkManager* NetworkManager);
	static void Handle_SC_RESPAWN_RES_TO_OTHER(CMessage* pMessage, UM1NetworkManager* NetworkManager);
};
