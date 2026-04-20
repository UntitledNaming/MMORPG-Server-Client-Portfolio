// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/M1PrimaryDataAsset.h"

#include "UObject/ObjectSaveContext.h"

void UM1PrimaryDataAsset::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
    Super::PreSave(ObjectSaveContext);

    AssetNameToPath.Empty();
    AssetLabelToSet.Empty();
    AssetGroupNameToSet.KeySort([](const FName& A, const FName& B)
        {
            return (A.Compare(B) < 0);
        }
    );

    for (const auto& Pair : AssetGroupNameToSet)
    {
        const FAssetSet& AssetSet = Pair.Value;
        for (FAssetEntry AssetEntry : AssetSet.AssetEntries)
        {
            FSoftObjectPath& AssetPath = AssetEntry.AssetPath;
            const FString& AssetName = AssetPath.GetAssetName();
            if (AssetName.StartsWith(TEXT("BP_")) || AssetName.StartsWith(TEXT("B_")) ||
                AssetName.StartsWith(TEXT("GET_")) || AssetName.StartsWith(TEXT("GA_")))
            {
                FString AssetPathString = AssetPath.GetAssetPathString();
                AssetPathString.Append(TEXT("_C"));
                AssetPath = FSoftObjectPath(AssetPathString);
            }

            AssetNameToPath.Emplace(AssetEntry.AssetName, AssetEntry.AssetPath);
            for (const FName& Label : AssetEntry.AssetLabels)
            {
                AssetLabelToSet.FindOrAdd(Label).AssetEntries.Emplace(AssetEntry);
            }

        }
    }
}


FSoftObjectPath UM1PrimaryDataAsset::GetAssetPathByName(const FName& AssetName)
{
    FSoftObjectPath* AssetPath = AssetNameToPath.Find(AssetName);
    ensureAlwaysMsgf(AssetPath, TEXT("Can't find Asset Path from Asset Name [%s]"), *AssetName.ToString());
    return *AssetPath;
}

const FAssetSet& UM1PrimaryDataAsset::GetAssetSetByLabel(const FName& Label)
{
    const FAssetSet* AssetSet = AssetLabelToSet.Find(Label);
    ensureAlwaysMsgf(AssetSet, TEXT("Can't find Asset Path from Asset Label [%s]"), *Label.ToString());
    return *AssetSet;
}