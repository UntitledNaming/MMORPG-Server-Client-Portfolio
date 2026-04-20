// Fill out your copyright notice in the Description page of Project Settings.


#include "System/M1AssetManager.h"

UM1AssetManager::UM1AssetManager()
{

}


UM1AssetManager& UM1AssetManager::Get()
{
    if (UM1AssetManager* Singleton = Cast<UM1AssetManager>(GEngine->AssetManager))
        return *Singleton;

    UE_LOG(LogM1, Fatal, TEXT("Can't find UR1AssetManager"));

    return *NewObject<UM1AssetManager>();
}

void UM1AssetManager::Initialize()
{
    Get().LoadPreloadAssets();
}

void UM1AssetManager::LoadSyncByPath(const FSoftObjectPath& AssetPath)
{
    if (AssetPath.IsValid())
    {
        UObject* LoadedAsset = AssetPath.ResolveObject();
        if (LoadedAsset == nullptr)
        {
            if (UAssetManager::IsInitialized())
            {
                LoadedAsset = UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
            }
            else
            {
                LoadedAsset = AssetPath.TryLoad();
            }
        }
        

        if (LoadedAsset)
        {
            Get().AddLoadedAsset(AssetPath.GetAssetFName(), LoadedAsset);
        }
        else
        {
            UE_LOG(LogM1, Fatal, TEXT("Failed to load asset [%s]"), *AssetPath.ToString());
        }
    }
}

void UM1AssetManager::LoadSyncByName(const FName& AssetName)
{
    UM1PrimaryDataAsset* AssetData = Get().LoadedAssetData;
    check(AssetData);

    const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByName(AssetName);
    LoadSyncByPath(AssetPath);
}

void UM1AssetManager::LoadSyncByLabel(const FName& Label)
{
    if (UAssetManager::IsInitialized() == false)
    {
        UE_LOG(LogM1, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    UM1PrimaryDataAsset* AssetData = Get().LoadedAssetData;
    check(AssetData);

    TArray<FSoftObjectPath> AssetPaths;

    const FAssetSet& AssetSet = AssetData->GetAssetSetByLabel(Label);
    for (const FAssetEntry& AssetEntry : AssetSet.AssetEntries)
    {
        const FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
        LoadSyncByPath(AssetPath);
    }


    for (const FAssetEntry& AssetEntry : AssetSet.AssetEntries)
    {
        const FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
        if (AssetPath.IsValid())
        {
            if (UObject* LoadedAsset = AssetPath.ResolveObject())
            {
                Get().AddLoadedAsset(AssetEntry.AssetName, LoadedAsset);
            }
            else
            {
                UE_LOG(LogM1, Fatal, TEXT("Failed to load asset [%s]"), *AssetPath.ToString());
            }
        }
    }
}

void UM1AssetManager::LoadAsyncByPath(const FSoftObjectPath& AssetPath, FAsyncLoadCompletedDelegate CompletedDelegate)
{
    if (UAssetManager::IsInitialized() == false)
    {
        return;
    }

    if (AssetPath.IsValid())
    {
        if (UObject* LoadedAsset = AssetPath.ResolveObject())
        {
            Get().AddLoadedAsset(AssetPath.GetAssetFName(), LoadedAsset);
        }
        else
        {
            TArray<FSoftObjectPath> AssetPaths;
            AssetPaths.Add(AssetPath);

            TSharedPtr<FStreamableHandle> Handle = GetStreamableManager().RequestAsyncLoad(AssetPaths);

            Handle->BindCompleteDelegate(FStreamableDelegate::CreateLambda([AssetName = AssetPath.GetAssetFName(), AssetPath, CompleteDelegate = MoveTemp(CompletedDelegate)]()
                {
                    UObject* LoadedAsset = AssetPath.ResolveObject();
                    Get().AddLoadedAsset(AssetName, LoadedAsset);
                    if (CompleteDelegate.IsBound())
                        CompleteDelegate.Execute(AssetName, LoadedAsset);
                }));
        }
    }
}

void UM1AssetManager::LoadAsyncByName(const FName& AssetName, FAsyncLoadCompletedDelegate CompletedDelegate)
{
    if (UAssetManager::IsInitialized() == false)
    {
        UE_LOG(LogM1, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    UM1PrimaryDataAsset* AssetData = Get().LoadedAssetData;
    check(AssetData);

    const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByName(AssetName);
    LoadAsyncByPath(AssetPath, CompletedDelegate);
}


void UM1AssetManager::ReleaseSyncByPath(const FSoftObjectPath& AssetPath)
{

}

void UM1AssetManager::ReleaseSyncByName(const FName& AssetName)
{

}

void UM1AssetManager::ReleaseSyncByLabel(const FName& Label)
{

}

void UM1AssetManager::ReleaseAll()
{

}

void UM1AssetManager::AddLoadedAsset(const FName& AssetName, const UObject* Asset)
{
    if (AssetName.IsValid() && Asset)
    {
        //FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);

        if (NameToLoadedAsset.Contains(AssetName) == false)
        {
            NameToLoadedAsset.Add(AssetName, Asset);
        }
    }
}

void UM1AssetManager::LoadPreloadAssets()
{
    if (LoadedAssetData)
        return;

    UM1PrimaryDataAsset* AssetData = nullptr;
    FPrimaryAssetType PrimaryAssetType(UM1PrimaryDataAsset::StaticClass()->GetFName());
    TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
    if (Handle.IsValid())
    {
        Handle->WaitUntilComplete(0.f, false);
        AssetData = Cast<UM1PrimaryDataAsset>(Handle->GetLoadedAsset());
    }

    if (AssetData)
    {
        LoadedAssetData = AssetData;
        LoadSyncByLabel("Preload");
    }
    else
    {
        UE_LOG(LogM1, Fatal, TEXT("Failed to load AssetData asset type [%s]."), *PrimaryAssetType.ToString());
    }
}