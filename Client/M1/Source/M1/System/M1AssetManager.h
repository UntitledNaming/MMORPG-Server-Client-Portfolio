// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M1LogClass.h"
#include "CoreMinimal.h"
#include "Data/M1PrimaryDataAsset.h"
#include "Engine/AssetManager.h"
#include "M1AssetManager.generated.h"

class UM1PrimaryDataAsset;

DECLARE_DELEGATE_TwoParams(FAsyncLoadCompletedDelegate, const FName&, UObject*);

UCLASS()
class M1_API UM1AssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
    UM1AssetManager();

    static UM1AssetManager& Get();

public:
    static void Initialize();

    template<typename AssetType>
    static AssetType* GetAssetByName(const FName& AssetName);

    static void LoadSyncByPath(const FSoftObjectPath& AssetPath);
    static void LoadSyncByName(const FName& AssetName);
    static void LoadSyncByLabel(const FName& Label);

    static void LoadAsyncByPath(const FSoftObjectPath& AssetPath, FAsyncLoadCompletedDelegate CompletedDelegate = FAsyncLoadCompletedDelegate());
    static void LoadAsyncByName(const FName& AssetName, FAsyncLoadCompletedDelegate CompletedDelegate = FAsyncLoadCompletedDelegate());

    static void ReleaseSyncByPath(const FSoftObjectPath& AssetPath);
    static void ReleaseSyncByName(const FName& AssetName);
    static void ReleaseSyncByLabel(const FName& Label);
    static void ReleaseAll();


private:
    void AddLoadedAsset(const FName& AssetName, const UObject* Asset);
    void LoadPreloadAssets();

private:
    UPROPERTY()
    TObjectPtr<UM1PrimaryDataAsset> LoadedAssetData;

    UPROPERTY()
    TMap<FName, TObjectPtr<const UObject>> NameToLoadedAsset;
};

template<typename AssetType>
AssetType* UM1AssetManager::GetAssetByName(const FName& AssetName)
{
    UM1PrimaryDataAsset* AssetData = Get().LoadedAssetData;
    check(AssetData);

    AssetType* LoadedAsset = nullptr;
    const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByName(AssetName);
    if (AssetPath.IsValid())
    {
        LoadedAsset = Cast<AssetType>(AssetPath.ResolveObject());
        if (LoadedAsset == nullptr)
        {
            UE_LOG(LogM1, Warning, TEXT("Attempted sync loading because asset hadn't loaded yet [%s]."), *AssetPath.ToString());
            LoadedAsset = Cast<AssetType>(AssetPath.TryLoad());
        }
    }
    return LoadedAsset;
}