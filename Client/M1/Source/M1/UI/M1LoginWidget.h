#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M1LoginWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class UM1NetworkManager;

UCLASS()
class M1_API UM1LoginWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* Edit_AccountID = nullptr;

    UPROPERTY(meta = (BindWidget))
    UButton* Button_Login = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Message = nullptr;

    UPROPERTY()
    UM1NetworkManager* NetworkManager = nullptr;

private:
    UFUNCTION()
    void OnClickLogin();

    bool ParseAccountID(uint64& OutAccountID) const;
};