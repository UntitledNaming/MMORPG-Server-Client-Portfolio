
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M1RespawnWidget.generated.h"

class UButton;
class UTextBlock;
class UM1NetworkManager;

UCLASS()
class M1_API UM1RespawnWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY(meta = (BindWidget))
    UButton* Button_Respawn = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Message = nullptr;

    UPROPERTY()
    UM1NetworkManager* NetworkManager = nullptr;

private:
    UFUNCTION()
    void OnClickRespawn();
};