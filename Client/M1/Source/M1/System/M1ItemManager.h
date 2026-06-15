#pragma once

#include "CoreMinimal.h"
#include "Type\M1Type.h"
#include "Item\M1ItemTable.h"
#include "ContentsStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "M1ItemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlotCoolTimeStart, uint8, SlotType, int16, SlotIndex, float, Duration);

class UM1NetworkManager;
class UM1ItemTooltipWidget;

UCLASS()
class M1_API UM1ItemManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    UM1NetworkManager*    NetworkManager;
	TArray<FItemSlotData> Inventory;
	TArray<FItemSlotData> Equipment;
	TArray<FItemSlotData> QuickSlot;
    FPendingSlotRequest   PendingRequest; 
	bool                  bPendingSlotRequest;

    UPROPERTY()
    TObjectPtr<class UM1DraggedItemWidget> ActiveDragWidget;

    UPROPERTY()
    TObjectPtr<UM1ItemTooltipWidget> ActiveTooltipWidget;

    bool                  bIsDragging = false;
    SLOT_TYPE             DragStartSourceType = SLOT_TYPE::NONE;     // 드래그 시작할 때 위치의 슬롯 타입
    int16                 DragStartSourceIndex = -1;                 // 드래그 시작할 때 위치의 슬롯 인덱스
    SLOT_TYPE             DragHoverType = SLOT_TYPE::NONE;           // 마우스가 현재 올려진 슬롯 타입
    int16                 DragHoverIndex = -1;                       // 마우스가 현재 올려진 슬롯 인덱스

public:
	// ── 델리게이트 (Blueprint 바인딩용) ──────────────
	UPROPERTY(BlueprintAssignable) FOnInventoryChanged OnInventoryChanged;
	UPROPERTY(BlueprintAssignable) FOnQuickSlotChanged OnQuickSlotChanged;
	UPROPERTY(BlueprintAssignable) FOnEquipmentChanged OnEquipmentChanged;

public:
    // ── PacketHandler가 호출 (패킷 수신 시) ──────────
    void OnPickupEquipmentItem(const PickUpEquipResult& Result);
    void OnPickupConsumableItem(const PickUpConsumableResult& Result);
    void OnUseConsumableResult(bool bSuccess, USE_CONSUMABLE_ITEM_RESULT& Result);
    void OnEquipItemResult(bool bSuccess, EQUIP_ITEM_RESULT& Result);
    void OnUnequipItemResult(bool bSuccess, UNEQUIP_ITEM_RESULT& Result);
    void OnDeleteItemResult(bool bSuccess);
    void OnSwapSlotResult(bool bSuccess);

    // ── UI가 호출  ────────────────────────
    void TrySendUseItem(uint8 SlotType, int16 SlotIndex);
    void TrySendDeleteItem(uint8 SlotType, int16 SlotIndex);
    void TrySendSwapSlot(uint8 FromType, int16 FromIdx, uint8 ToType, int16 ToIdx);

    const FItemSlotData& GetInventorySlot(int16 Index) const { return Inventory[Index]; }
    const FItemSlotData& GetEquipmentSlot(int16 Index) const { return Equipment[Index]; }
    const FItemSlotData& GetQuickSlot(int16 Index) const { return QuickSlot[Index]; }

    void      StartDrag(SLOT_TYPE, int16, UM1DraggedItemWidget*);
    void      EndDrag();
    void      ShowTooltip(SLOT_TYPE, int16, TSubclassOf<UM1ItemTooltipWidget>);
    void      HideTooltip();
    bool      IsDragging() const { return bIsDragging; };
    SLOT_TYPE GetDragStartSourceType() const { return DragStartSourceType; };
    int16     GetDragStartSourceIndex() const { return DragStartSourceIndex; };
    SLOT_TYPE GetDragHoverType() const { return DragHoverType; };
    int16     GetDragHoverIndex() const { return DragHoverIndex; };
    void      SetDragHover(SLOT_TYPE type, int16 index) { DragHoverType = type; DragHoverIndex = index; };
    void      ClearDragHover() { DragHoverType = SLOT_TYPE::NONE; DragHoverIndex = -1; };


    // ── Packet Handler 내 캐릭터 생성시 호출  ────────────────────────
    void  ParseAndInitFromSpawn(class CMessage* pMessage);

private:
    void RefreshEquipStat();
    FM1CharacterStat CalculateEquipStat();

public:
    UPROPERTY(BlueprintAssignable)
    FOnSlotCoolTimeStart OnSlotCoolTimeStart;

    UFUNCTION(BlueprintCallable)
    bool IsSlotOnCoolTime(uint8 SlotType, int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable)
    float GetSlotCoolTimeRatio(uint8 SlotType, int32 SlotIndex) const;

private:
    struct FSlotCoolTimeInfo
    {
        FTimerHandle TimerHandle;  // 쿨타임 끝낼 타이머
        float Duration = 0.f;      // 전체 쿨타임 시간
        double EndTime = 0.0;      // 쿨타임 끝나는 월드 시간
    };
    TMap<int32, FSlotCoolTimeInfo> SlotCoolTimeMap;

    int32 MakeSlotCoolTimeKey(SLOT_TYPE SlotType, int16 SlotIndex) const;
    void StartSlotCoolTime(SLOT_TYPE SlotType, int16 SlotIndex, ITEM_ID UsedItemID);
};
