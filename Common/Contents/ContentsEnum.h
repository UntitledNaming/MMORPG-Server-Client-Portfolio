#pragma once
#include "ContentsType.h"

/////////////////////////////////////////
//           <MoveMode>
// 0 : Stop  / 1 : Walk / 2 : Run
/////////////////////////////////////////
enum class EM1MoveMode : uint8
{
	Walk = 0,   // Shift 안눌렀을 때
	Run         // Shift 눌렀을 때
};

// 상태관련 타입
enum class EM1ActionStateType : uint8
{
	None = 0,
	Attack,
	Skill,
	Hit,
	Dead
};

enum class EAIState : uint8
{
	None = 0,
	Idle,
	Patrol,
	Chase,
	Combat,
	Return,
	Dead
};

enum class EServerAbilitySlot : uint8
{
	Skill1,
	Skill2,
	Skill3,
	Skill4,
};

enum class ESkillDamageType : uint8
{
	None,
	Physical,
	Magic,
	TrueDamage,
};

enum class EHitShape : uint8
{
	None,
	Circle,
	Cone,
	Box,
};

enum class EMonsterState : uint8
{
	Idle,           // 대기
	Patrol,         // 주변 순찰
	Chase,          // 추격
	Combat,         // 공격 
	Return,         // 스폰 위치로 복귀
	Dead            // 사망, 리젠 대기
};

enum class ITEM_TYPE : uint8
{
	NONE,
	CONSUMABLE,
	EQUIPMENT,
};

enum class EQUIP_SLOT : uint8
{
	NONE,
	HELMET,
	CHEST,
	PANTS,
	BOOTS,
	WEAPON,
	MAX,
};

enum class ITEM_RARITY : uint8
{
	NORMAL,
	MAGIC,
};

enum class FIELD_DROP_TYPE : uint8
{
	NONE,
	HP_POTION,
	MP_POTION,
	NORMAL_EQUIPMENT,
	MAGIC_EQUIPMENT,
};

enum class RANDOM_STAT_TYPE : uint8
{
	NONE,
	ATK,
	DEF,
	MAX_HP,
	MAX_MP,
	HP_REGEN,
	MP_REGEN,
	MAX,
};

enum class SLOT_TYPE : uint8
{
	NONE,
	INVENTORY,
	EQUIPMENT,
	QUICKSLOT,
};

enum class USE_ITEM_RESULT : uint8
{
	NONE,
	CONSUME,
	EQUIP,
	UNEQUIP,
};

enum class CONSUMABLE_ITEM_TYPE : uint8
{
	SMALL_HP_POTION,
	SMALL_MP_POTION,
	MAX,
};

enum class FieldRecvType
{
	CharacterMovement,
	RTT,
	LeftSwing,
	SkillUse,
	PickUpItems,
	UseItem,
	DeleteItem,
	SwapItem,
	Respawn,
	Max
};

enum class FrameProcType
{
	Whole,
	User,
	Monster,
	FieldDropItem,
	Max
};

enum class BroadCastType
{
	CreateMyCharacterToOther,
	DeleteCharacter,
	UpdateCharacterMovementInput,
	SyncOtherPos,
	LeftSwing,
	AttackHitResult,
	UseSkill,
	CreateMonster,
	DeleteMonster,
	MoveMonster,
	StopMonster,
	MonsterHitPlayer,
	CreateFieldDropItem,
	DeleteFieldDropItem,
	Respawn,
	Max
};

enum class AuthRecvType
{
	Login,
	CharacterSelect,
	Max
};

enum class DBJobCount
{
	CharacterSelect,
	InsertItem,
	DeleteItem,
	ItemUpdateCount,
	ItemSlotUpdate,
	CharacterProgress,
	LogOut,
	Max,
};