// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralTerrain/ProcTerrainTypes.h"
#include "ProcTerrainManager.generated.h"

class APawn;
class AProcTerrainChunk;
class UProcBiomeDataAsset;

UCLASS()
class AProcTerrainManager : public AActor
{
	GENERATED_BODY()

public:
	AProcTerrainManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Procedural Terrain")
	void UpdateChunksAroundPlayer();

	UFUNCTION(BlueprintCallable, Category="Procedural Terrain")
	void RegenerateWorld();

	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	FIntPoint WorldToChunkCoord(FVector WorldLocation) const;

	UFUNCTION(BlueprintPure, Category="Procedural Terrain")
	FVector ChunkCoordToWorldOrigin(FIntPoint Coord) const;

	const UProcBiomeDataAsset* ResolveBiomeAtWorldPosition(FVector2D WorldPos) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	TSubclassOf<AProcTerrainChunk> ChunkActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	TObjectPtr<UProcBiomeDataAsset> DefaultBiome = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain", meta=(ClampMin="1"))
	int32 LoadRadiusInChunks = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain", meta=(ClampMin="2"))
	int32 ChunkVertexCountX = 65;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain", meta=(ClampMin="2"))
	int32 ChunkVertexCountY = 65;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain", meta=(ClampMin="1.0"))
	float VertexSpacing = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	bool bGenerateCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool bDrawDebugChunkBounds = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	bool bLogChunkLifecycle = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Debug")
	FColor DebugChunkBoundsColor = FColor::Green;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Terrain")
	FIntPoint CurrentCenterChunk = FIntPoint::ZeroValue;

private:
	void EnsureChunkExists(const FIntPoint& Coord);
	void RemoveFarChunks(const FIntPoint& CenterCoord);
	void DrawChunkDebugBounds() const;
	APawn* ResolvePlayerPawn();
	FProcTerrainSettings BuildEffectiveTerrainSettings() const;

	TMap<FIntPoint, AProcTerrainChunk*> ActiveChunks;
	TWeakObjectPtr<APawn> CachedPlayerPawn;
	bool bChunksInitialized = false;
};
