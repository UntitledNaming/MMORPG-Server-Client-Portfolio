// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M1OverheadStatusWidget.generated.h"

class UTextBlock;
class USizeBox;


UCLASS()
class M1_API UM1OverheadStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void InitOverheadStatus(const FText& InName, int32 InCurrentHP, int32 InMaxHP);

	UFUNCTION(BlueprintCallable)
	void SetOverheadName(const FText& InName);

	UFUNCTION(BlueprintCallable)
	void SetOverheadHP(int32 InCurrentHP, int32 InMaxHP);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Name = nullptr;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SizeBox_HPClip = nullptr;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overhead HP", meta = (AllowPrivateAccess = "true"))
	float MaxBarWidth = 200.f;
};
