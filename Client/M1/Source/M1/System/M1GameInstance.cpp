// Fill out your copyright notice in the Description page of Project Settings.


#include "System/M1GameInstance.h"
#include "M1AssetManager.h"

UM1GameInstance::UM1GameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UM1GameInstance::Init()
{
    Super::Init();

    // 에셋 매니저 초기화
    UM1AssetManager::Initialize();
}

void UM1GameInstance::Shutdown()
{
    Super::Shutdown();
}

