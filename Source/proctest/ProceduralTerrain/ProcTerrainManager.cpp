// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrain/ProcTerrainManager.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ProceduralTerrain/ProcBiomeDataAsset.h"
#include "ProceduralTerrain/ProcTerrainChunk.h"
#include "ProceduralTerrain/ProcTerrainFunctionLibrary.h"
#include "proctest.h"

AProcTerrainManager::AProcTerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ChunkActorClass = AProcTerrainChunk::StaticClass();
}

void AProcTerrainManager::BeginPlay()
{
	Super::BeginPlay();

	UpdateChunksAroundPlayer();
}

void AProcTerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateChunksAroundPlayer();

	if (bDrawDebugChunkBounds)
	{
		DrawChunkDebugBounds();
	}
}

void AProcTerrainManager::UpdateChunksAroundPlayer()
{
	APawn* PlayerPawn = ResolvePlayerPawn();
	if (!PlayerPawn)
	{
		return;
	}

	const FIntPoint NewCenterChunk = WorldToChunkCoord(PlayerPawn->GetActorLocation());
	if (bChunksInitialized && NewCenterChunk == CurrentCenterChunk)
	{
		return;
	}

	CurrentCenterChunk = NewCenterChunk;
	bChunksInitialized = true;

	for (int32 ChunkY = CurrentCenterChunk.Y - LoadRadiusInChunks; ChunkY <= CurrentCenterChunk.Y + LoadRadiusInChunks; ++ChunkY)
	{
		for (int32 ChunkX = CurrentCenterChunk.X - LoadRadiusInChunks; ChunkX <= CurrentCenterChunk.X + LoadRadiusInChunks; ++ChunkX)
		{
			EnsureChunkExists(FIntPoint(ChunkX, ChunkY));
		}
	}

	RemoveFarChunks(CurrentCenterChunk);
}

void AProcTerrainManager::RegenerateWorld()
{
	for (TPair<FIntPoint, AProcTerrainChunk*>& Pair : ActiveChunks)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}

	ActiveChunks.Empty();
	bChunksInitialized = false;

	UpdateChunksAroundPlayer();
}

FIntPoint AProcTerrainManager::WorldToChunkCoord(FVector WorldLocation) const
{
	return UProcTerrainFunctionLibrary::WorldToChunkCoord(WorldLocation, BuildEffectiveTerrainSettings());
}

FVector AProcTerrainManager::ChunkCoordToWorldOrigin(FIntPoint Coord) const
{
	return UProcTerrainFunctionLibrary::ChunkCoordToWorldOrigin(Coord, BuildEffectiveTerrainSettings());
}

const UProcBiomeDataAsset* AProcTerrainManager::ResolveBiomeAtWorldPosition(FVector2D WorldPos) const
{
	return DefaultBiome;
}

void AProcTerrainManager::EnsureChunkExists(const FIntPoint& Coord)
{
	if (ActiveChunks.Contains(Coord) && IsValid(ActiveChunks[Coord]))
	{
		return;
	}

	if (!ChunkActorClass)
	{
		UE_LOG(Logproctest, Warning, TEXT("ProcTerrainManager '%s' cannot spawn chunks because ChunkActorClass is not set."), *GetName());
		return;
	}

	const FProcTerrainSettings TerrainSettings = BuildEffectiveTerrainSettings();
	const UProcBiomeDataAsset* ResolvedBiome = ResolveBiomeAtWorldPosition(FVector2D(
		Coord.X * TerrainSettings.GetChunkWorldSizeX(),
		Coord.Y * TerrainSettings.GetChunkWorldSizeY()));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AProcTerrainChunk* Chunk = GetWorld()->SpawnActor<AProcTerrainChunk>(
		ChunkActorClass,
		ChunkCoordToWorldOrigin(Coord),
		FRotator::ZeroRotator,
		SpawnParameters);

	if (!Chunk)
	{
		UE_LOG(Logproctest, Warning, TEXT("ProcTerrainManager '%s' failed to spawn chunk (%d, %d)."), *GetName(), Coord.X, Coord.Y);
		return;
	}

	Chunk->InitializeChunk(Coord, const_cast<UProcBiomeDataAsset*>(ResolvedBiome), TerrainSettings, Seed);
	ActiveChunks.Add(Coord, Chunk);

	if (bLogChunkLifecycle)
	{
		UE_LOG(Logproctest, Log, TEXT("Spawned procedural chunk (%d, %d)."), Coord.X, Coord.Y);
	}
}

void AProcTerrainManager::RemoveFarChunks(const FIntPoint& CenterCoord)
{
	for (auto It = ActiveChunks.CreateIterator(); It; ++It)
	{
		const FIntPoint ChunkCoord = It.Key();
		const bool bOutOfRange =
			FMath::Abs(ChunkCoord.X - CenterCoord.X) > LoadRadiusInChunks ||
			FMath::Abs(ChunkCoord.Y - CenterCoord.Y) > LoadRadiusInChunks;

		if (!bOutOfRange)
		{
			continue;
		}

		if (IsValid(It.Value()))
		{
			if (bLogChunkLifecycle)
			{
				UE_LOG(Logproctest, Log, TEXT("Removed procedural chunk (%d, %d)."), ChunkCoord.X, ChunkCoord.Y);
			}

			It.Value()->Destroy();
		}

		It.RemoveCurrent();
	}
}

void AProcTerrainManager::DrawChunkDebugBounds() const
{
	if (!GetWorld())
	{
		return;
	}

	const FProcTerrainSettings TerrainSettings = BuildEffectiveTerrainSettings();
	const FVector BoxExtent(
		TerrainSettings.GetChunkWorldSizeX() * 0.5f,
		TerrainSettings.GetChunkWorldSizeY() * 0.5f,
		100.0f);

	for (const TPair<FIntPoint, AProcTerrainChunk*>& Pair : ActiveChunks)
	{
		const FVector ChunkOrigin = ChunkCoordToWorldOrigin(Pair.Key);
		const FVector BoxCenter = ChunkOrigin + FVector(BoxExtent.X, BoxExtent.Y, 0.0f);

		DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, DebugChunkBoundsColor, false, -1.0f, 0, 4.0f);
	}
}

APawn* AProcTerrainManager::ResolvePlayerPawn()
{
	if (CachedPlayerPawn.IsValid())
	{
		return CachedPlayerPawn.Get();
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
		{
			PlayerPawn = PlayerController->GetPawn();
		}
	}

	CachedPlayerPawn = PlayerPawn;
	return PlayerPawn;
}

FProcTerrainSettings AProcTerrainManager::BuildEffectiveTerrainSettings() const
{
	FProcTerrainSettings TerrainSettings = DefaultBiome ? DefaultBiome->TerrainSettings : FProcTerrainSettings();
	TerrainSettings.Seed = Seed;
	TerrainSettings.ChunkVertsX = ChunkVertexCountX;
	TerrainSettings.ChunkVertsY = ChunkVertexCountY;
	TerrainSettings.VertexSpacing = VertexSpacing;
	TerrainSettings.bGenerateCollision = bGenerateCollision;
	return TerrainSettings;
}
