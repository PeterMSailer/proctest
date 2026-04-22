// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralTerrain/ProcTerrainTypes.h"
#include "ProcTerrainChunk.generated.h"

class UProcBiomeDataAsset;
class USceneComponent;

UCLASS()
class AProcTerrainChunk : public AActor
{
	GENERATED_BODY()

public:
	AProcTerrainChunk();

	void InitializeChunk(const FIntPoint& InCoord, UProcBiomeDataAsset* InBiomeData, const FProcTerrainSettings& InTerrainSettings, int32 InSeed);
	virtual void GenerateChunkMesh();
	float SampleHeight(float WorldX, float WorldY) const;

	FIntPoint GetChunkCoord() const
	{
		return ChunkCoord;
	}

	const FProcTerrainSettings& GetTerrainSettings() const
	{
		return TerrainSettings;
	}

protected:
	void BuildMeshData(
		TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles,
		TArray<FVector>& OutNormals,
		TArray<FVector2D>& OutUVs,
		TArray<FLinearColor>& OutVertexColors,
		TArray<FProcMeshTangent>& OutTangents) const;

	FVector ComputeVertexNormal(float WorldX, float WorldY) const;
	FProcMeshTangent ComputeVertexTangent(float WorldX, float WorldY) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UProceduralMeshComponent> ProcMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Terrain")
	FIntPoint ChunkCoord = FIntPoint::ZeroValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Terrain")
	TObjectPtr<UProcBiomeDataAsset> BiomeData = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Terrain")
	FProcTerrainSettings TerrainSettings;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Terrain")
	int32 Seed = 1337;
};
