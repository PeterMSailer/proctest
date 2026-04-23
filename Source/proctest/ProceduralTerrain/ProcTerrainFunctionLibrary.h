// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ProceduralTerrain/ProcTerrainTypes.h"
#include "ProcTerrainFunctionLibrary.generated.h"

UCLASS()
class UProcTerrainFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	static float SampleLayeredHeight(float WorldX, float WorldY, int32 Seed, const FProcTerrainSettings& TerrainSettings);

	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	static float SampleHeightAtWorldPosition(FVector2D WorldPosition, int32 Seed, const FProcTerrainSettings& TerrainSettings);

	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	static FVector ComputeNormalFromHeights(float WorldX, float WorldY, int32 Seed, const FProcTerrainSettings& TerrainSettings);

	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	static FIntPoint WorldToChunkCoord(const FVector& WorldLocation, const FProcTerrainSettings& TerrainSettings);

	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	static FVector ChunkCoordToWorldOrigin(FIntPoint ChunkCoord, const FProcTerrainSettings& TerrainSettings);
};
