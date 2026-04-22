// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrain/ProcTerrainFunctionLibrary.h"

#include "Templates/TypeHash.h"

namespace ProcTerrain
{
	static float MakeSeedOffset(int32 Seed, int32 Salt)
	{
		const uint32 Hash = HashCombineFast(GetTypeHash(Seed), GetTypeHash(Salt));
		return static_cast<float>(Hash % 1048576);
	}
}

float UProcTerrainFunctionLibrary::SampleLayeredHeight(float WorldX, float WorldY, int32 Seed, const FProcTerrainSettings& TerrainSettings)
{
	const FVector2D BaseOffset(ProcTerrain::MakeSeedOffset(Seed, 11), ProcTerrain::MakeSeedOffset(Seed, 29));
	const FVector2D BaseInput((WorldX + BaseOffset.X) * TerrainSettings.BaseNoiseScale, (WorldY + BaseOffset.Y) * TerrainSettings.BaseNoiseScale);
	const FVector2D BaseOrigin(BaseOffset.X * TerrainSettings.BaseNoiseScale, BaseOffset.Y * TerrainSettings.BaseNoiseScale);

	float Height = (FMath::PerlinNoise2D(BaseInput) - FMath::PerlinNoise2D(BaseOrigin)) * TerrainSettings.HeightScale;

	if (!FMath::IsNearlyZero(TerrainSettings.DetailNoiseScale) && !FMath::IsNearlyZero(TerrainSettings.DetailNoiseStrength))
	{
		const FVector2D DetailOffset(ProcTerrain::MakeSeedOffset(Seed, 53), ProcTerrain::MakeSeedOffset(Seed, 97));
		const FVector2D DetailInput((WorldX + DetailOffset.X) * TerrainSettings.DetailNoiseScale, (WorldY + DetailOffset.Y) * TerrainSettings.DetailNoiseScale);
		const FVector2D DetailOrigin(DetailOffset.X * TerrainSettings.DetailNoiseScale, DetailOffset.Y * TerrainSettings.DetailNoiseScale);
		Height += (FMath::PerlinNoise2D(DetailInput) - FMath::PerlinNoise2D(DetailOrigin)) * TerrainSettings.DetailNoiseStrength;
	}

	return Height;
}

FIntPoint UProcTerrainFunctionLibrary::WorldToChunkCoord(const FVector& WorldLocation, const FProcTerrainSettings& TerrainSettings)
{
	const float ChunkSizeX = TerrainSettings.GetChunkWorldSizeX();
	const float ChunkSizeY = TerrainSettings.GetChunkWorldSizeY();

	if (ChunkSizeX <= UE_SMALL_NUMBER || ChunkSizeY <= UE_SMALL_NUMBER)
	{
		return FIntPoint::ZeroValue;
	}

	return FIntPoint(
		FMath::FloorToInt(WorldLocation.X / ChunkSizeX),
		FMath::FloorToInt(WorldLocation.Y / ChunkSizeY));
}

FVector UProcTerrainFunctionLibrary::ChunkCoordToWorldOrigin(FIntPoint ChunkCoord, const FProcTerrainSettings& TerrainSettings)
{
	return FVector(
		ChunkCoord.X * TerrainSettings.GetChunkWorldSizeX(),
		ChunkCoord.Y * TerrainSettings.GetChunkWorldSizeY(),
		0.0f);
}
