// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProceduralTerrain/ProcTerrainTypes.h"
#include "ProcBiomeDataAsset.generated.h"

class UMaterialInterface;

UCLASS(BlueprintType)
class UProcBiomeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
	FName BiomeName = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
	FProcTerrainSettings TerrainSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
	TObjectPtr<UMaterialInterface> TerrainMaterial = nullptr;
};
