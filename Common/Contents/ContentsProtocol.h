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
    //  4   -   Yaw             (float)
    //  2   -   HP              (int16)
    //  2   -   MP              (int16)
    //  2   -   Level           (uint16)
    //  4   -   CurrentExp      (int32)
    // 
    //  // Inventory No Empty Item Slot Data
    //  1   -   Count           (uint8)
    //  per Slot:
    //  2   -   SlotIndex       (int16)
    //  4   -   ItemID          (uint32)
    //  2   -   ItemCount       (uint16)
    //  1   -   RandomStatCount (uint8)
    //      per Random Stat:
    //      1   - StatType      (uint8)
    //      2   - StatValue     (int16)
    // 
    //  // Equipment No Empty Item Slot Data 
    //  1   -   Count           (uint8)
    //  per Slot:
    //  1   -   EquipSlot       (uint8)
    //  4   -   ItemID          (uint32)
    //  1   -   RandomStatCount (uint8)
    //      per Random Stat:
    //      1   - StatType      (uint8)
    //      2   - StatValue     (int16)
    // 
    // 
    //  // QuickSlot No Empty Item Slot Data 
    //  1   -   Count           (uint8)
    //  per Slot:
    //  4   -   ItemID          (uint32)
    //  2   -   ItemCount       (uint16)
    // 
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
    //  2   -   HP              (int16) 
    //  2   -   MaxHP           (int16) 
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
    //  2   -   NewHP             (int16)
    //      
    //  per hit:
    //  8   -   MonsterID         (uint64)
    //  2   -   NewHP             (int16)
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
    //  2   -   HP              (int16) 
    //  2   -   MaxHP           (int16) 
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
    //	4	-	MoveSpeed 		(float)
    //	4	-	DesXpos  		(float)
    //	4	-	DesYpos 		(float)
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_STOP_MONSTER = 1022;
    //---------------------------------------------------------------
    //  Monster Stop Msg 					Server -> Client
    //
    //
    //	8	-	MonsterID		(uint64)
    //  4   -   Xpos            (float)
    //  4   -   Ypos            (float)
    //  4   -   Zpos            (float)
    // 
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_HIT_TOPLAYER = 1023;
    //---------------------------------------------------------------
    //  Monster Attack Msg 					Server -> Client
    //
    //
    //	8	-	MonsterID		(uint64)
    //	8	-	TargetID	 	(uint64)
    //	2	-	TargetNewHP 	(int16)
    //
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_CREATE_FIELD_DROP_ITEM = 1024;
    //---------------------------------------------------------------
    //  Create Drop Item Msg 					Server -> Client
    //
    //	8	-	DropID	     	(uint64)
    //  4   -   ItemID          (uint32)
    //  4   -   Xpos            (float)
    //  4   -   Ypos            (float)
    //  4   -   Zpos            (float)
    //  2   -   Count           (uint16)
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_DELETE_FIELD_DROP_ITEM = 1025;
    //---------------------------------------------------------------
    //  Delete Drop Item Msg 					Server -> Client
    //
    //	8	-	DropID	     	(uint64)
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_PICK_UP_ITEM = 1026;
    //---------------------------------------------------------------
    //  Pick Up Item Msg 					    Client -> Server
    //
    //  8   -  DropUID          (uint64)
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_USE_ITEM = 1027;
    //---------------------------------------------------------------
    //  Use Item Msg 					        Client -> Server
    //
    //  1   -  SlotType         (uint8)
    //  2   -  SlotIndex        (int16)
    // 
    //---------------------------------------------------------------


    constexpr uint16 PACKET_CS_DELETE_ITEM = 1028;
    //---------------------------------------------------------------
    //  Delete Slot Item Msg 					Client -> Server
    //
    //  1   -  SlotType         (uint8)
    //  2   -  SlotIndex        (int16)
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_CS_SWAP_SLOT = 1029;
    //---------------------------------------------------------------
    //  Swap Item Msg 					        Client -> Server
    //
    //  1   -  FromSlotType     (uint8)
    //  2   -  FromSlotIndex    (int16)
    //  1   -  ToSlotType       (uint8)
    //  2   -  ToSlotIndex      (int16)
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_PICKUP_EQUIPMENT_ITEMS = 1030;
    //---------------------------------------------------------------
    //  Pick Up Items Msg 					    Server -> Client
    //
    //  4   -  ItemID                  (uint32)
    //  2   -  SlotIndex               (int16)
    //  2   -  Count                   (uint16)
    //  1   -  RandomStatCount         (uint8)
    //  
    //  per RandomStatCount
    //  1   -  RandomStatType          (uint8)
    //  2   -  RandomStatValue         (int16)
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_PICKUP_CONSUMABLE_ITEMS = 1031;
    //---------------------------------------------------------------
    //  Pick Up Items Msg 					    Server -> Client
    //
    //  4   -  ItemID                  (uint32)
    //  2   -  UpdateSlotCount         (uint16)
    // 
    //  per 
    //  2   -  SlotIndex               (int16)
    //  2   -  NewItemCount            (uint16)
    //  
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_USE_CONSUMABLE_ITEM = 1032;
    //---------------------------------------------------------------
    //  Use Consumable Item Msg     		    Server -> Client
    //
    //  1   -  Success          (uint8)
    // 
    //  if Success
    //  1   -  SlotType         (uint8)
    //  2   -  NewItemCount     (uint16)
    //  2   -  SlotIndex        (int16) 
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_EQUIP_ITEM = 1033;
    //---------------------------------------------------------------
    //  Use EquipmentItem Msg     			    Server -> Client
    //  
    //  1   -  Success          (uint8)
    // 
    //  1   -  SlotType         (uint8)
    //  2   -  SlotIndex        (int16)
    //  4   -  ItemID           (uint32)
    // 
    //  1   -  SlotType         (uint8)
    //  2   -  SlotIndex        (int16)
    //  4   -  ItemID           (uint32)
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_UNEQUIP_ITEM = 1034;
    //---------------------------------------------------------------
    //  Use Item Msg     					    Server -> Client
    //
    //  1   -  Success                   (uint8)
    // 
    //  if Success
    //  2   -  InventorySlotIndex        (int16)
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_DELETE_ITEM = 1035;
    //---------------------------------------------------------------
    //  Delete Item Msg 					    Server -> Client
    //
    //  1   -  Success                   (uint8) 
    // 
    //---------------------------------------------------------------


    constexpr uint16 PACKET_SC_SWAP_SLOT = 1036;
    //---------------------------------------------------------------
    //  Swap Item Msg   					    Server -> Client
    //
    //  1   -  Success                   (uint8) 
    // 
    //---------------------------------------------------------------

    constexpr uint16 PACKET_SC_GAIN_EXP = 1037;
    //---------------------------------------------------------------
    //  Level Up Msg    					    Server -> Client
    //
    //  1   -  LevelUp                   (bool)
    //  4   -  CurExp                    (int32)
    // 
    //  if LevelUp True
    //  2   -  CurLevel                  (uint16) 
    //  2   -  HP                        (int16)
    //  2   -  MP                        (int16)
    //---------------------------------------------------------------

}

