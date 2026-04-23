// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ProcTerrainTypes.generated.h"

USTRUCT(BlueprintType)
struct FProcTerrainSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="2"))
	int32 ChunkVertsX = 65;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="2"))
	int32 ChunkVertsY = 65;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="1.0"))
	float VertexSpacing = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float HeightScale = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float BaseNoiseScale = 0.0008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	bool bUseHillNoise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float HillNoiseScale = 0.00018f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float HillNoiseStrength = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float DetailNoiseScale = 0.004f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float DetailNoiseStrength = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	bool bUseDetailNoise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float HeightAmplitudeMultiplier = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	bool bUseUpliftNoise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float UpliftNoiseScale = 0.00005f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0"))
	float UpliftStrength = 4800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UpliftThreshold = 0.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain", meta=(ClampMin="0.1"))
	float UpliftExponent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float HeightOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	bool bGenerateCollision = true;

	int32 GetQuadCountX() const
	{
		return FMath::Max(ChunkVertsX - 1, 1);
	}

	int32 GetQuadCountY() const
	{
		return FMath::Max(ChunkVertsY - 1, 1);
	}

	float GetChunkWorldSizeX() const
	{
		return GetQuadCountX() * VertexSpacing;
	}

	float GetChunkWorldSizeY() const
	{
		return GetQuadCountY() * VertexSpacing;
	}
};
