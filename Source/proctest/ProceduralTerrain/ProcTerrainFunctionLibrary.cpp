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

	static FVector2D MakeSeedOffset2D(int32 Seed, int32 SaltX, int32 SaltY)
	{
		return FVector2D(MakeSeedOffset(Seed, SaltX), MakeSeedOffset(Seed, SaltY));
	}

	static float SampleZeroCenteredNoise(float WorldX, float WorldY, int32 Seed, float NoiseScale, int32 SaltX, int32 SaltY)
	{
		if (FMath::IsNearlyZero(NoiseScale))
		{
			return 0.0f;
		}

		const FVector2D NoiseOffset = MakeSeedOffset2D(Seed, SaltX, SaltY);
		const FVector2D NoiseInput((WorldX + NoiseOffset.X) * NoiseScale, (WorldY + NoiseOffset.Y) * NoiseScale);
		const FVector2D OriginInput(NoiseOffset.X * NoiseScale, NoiseOffset.Y * NoiseScale);

		return FMath::PerlinNoise2D(NoiseInput) - FMath::PerlinNoise2D(OriginInput);
	}

	static float SampleNoiseLayer(float WorldX, float WorldY, int32 Seed, float NoiseScale, float Strength, int32 SaltX, int32 SaltY)
	{
		if (FMath::IsNearlyZero(Strength))
		{
			return 0.0f;
		}

		return SampleZeroCenteredNoise(WorldX, WorldY, Seed, NoiseScale, SaltX, SaltY) * Strength;
	}

	static float SampleNormalizedNoise(float WorldX, float WorldY, int32 Seed, float NoiseScale, int32 SaltX, int32 SaltY)
	{
		if (FMath::IsNearlyZero(NoiseScale))
		{
			return 0.5f;
		}

		const FVector2D NoiseOffset = MakeSeedOffset2D(Seed, SaltX, SaltY);
		const FVector2D NoiseInput((WorldX + NoiseOffset.X) * NoiseScale, (WorldY + NoiseOffset.Y) * NoiseScale);
		return FMath::Clamp((FMath::PerlinNoise2D(NoiseInput) * 0.5f) + 0.5f, 0.0f, 1.0f);
	}

	static float SampleThresholdedNoiseLayer(
		float WorldX,
		float WorldY,
		int32 Seed,
		float NoiseScale,
		float Strength,
		float Threshold,
		float Exponent,
		int32 SaltX,
		int32 SaltY)
	{
		if (FMath::IsNearlyZero(Strength))
		{
			return 0.0f;
		}

		const float NormalizedNoise = SampleNormalizedNoise(WorldX, WorldY, Seed, NoiseScale, SaltX, SaltY);
		const float Denominator = FMath::Max(1.0f - Threshold, KINDA_SMALL_NUMBER);
		const float ThresholdedNoise = FMath::Clamp((NormalizedNoise - Threshold) / Denominator, 0.0f, 1.0f);
		return FMath::Pow(ThresholdedNoise, FMath::Max(Exponent, 0.1f)) * Strength;
	}
}

float UProcTerrainFunctionLibrary::SampleLayeredHeight(float WorldX, float WorldY, int32 Seed, const FProcTerrainSettings& TerrainSettings)
{
	float Height = ProcTerrain::SampleNoiseLayer(
		WorldX,
		WorldY,
		Seed,
		TerrainSettings.BaseNoiseScale,
		TerrainSettings.HeightScale,
		11,
		29);

	if (TerrainSettings.bUseHillNoise)
	{
		Height += ProcTerrain::SampleNoiseLayer(
			WorldX,
			WorldY,
			Seed,
			TerrainSettings.HillNoiseScale,
			TerrainSettings.HillNoiseStrength,
			131,
			173);
	}

	if (TerrainSettings.bUseDetailNoise)
	{
		Height += ProcTerrain::SampleNoiseLayer(
			WorldX,
			WorldY,
			Seed,
			TerrainSettings.DetailNoiseScale,
			TerrainSettings.DetailNoiseStrength,
			53,
			97);
	}

	if (TerrainSettings.bUseUpliftNoise)
	{
		Height += ProcTerrain::SampleThresholdedNoiseLayer(
			WorldX,
			WorldY,
			Seed,
			TerrainSettings.UpliftNoiseScale,
			TerrainSettings.UpliftStrength,
			TerrainSettings.UpliftThreshold,
			TerrainSettings.UpliftExponent,
			211,
			257);
	}

	return (Height * TerrainSettings.HeightAmplitudeMultiplier) + TerrainSettings.HeightOffset;
}

float UProcTerrainFunctionLibrary::SampleHeightAtWorldPosition(FVector2D WorldPosition, int32 Seed, const FProcTerrainSettings& TerrainSettings)
{
	return SampleLayeredHeight(WorldPosition.X, WorldPosition.Y, Seed, TerrainSettings);
}

FVector UProcTerrainFunctionLibrary::ComputeNormalFromHeights(float WorldX, float WorldY, int32 Seed, const FProcTerrainSettings& TerrainSettings)
{
	const float SampleOffset = FMath::Max(TerrainSettings.VertexSpacing, 1.0f);
	const float LeftHeight = SampleLayeredHeight(WorldX - SampleOffset, WorldY, Seed, TerrainSettings);
	const float RightHeight = SampleLayeredHeight(WorldX + SampleOffset, WorldY, Seed, TerrainSettings);
	const float DownHeight = SampleLayeredHeight(WorldX, WorldY - SampleOffset, Seed, TerrainSettings);
	const float UpHeight = SampleLayeredHeight(WorldX, WorldY + SampleOffset, Seed, TerrainSettings);

	const FVector TangentX(SampleOffset * 2.0f, 0.0f, RightHeight - LeftHeight);
	const FVector TangentY(0.0f, SampleOffset * 2.0f, UpHeight - DownHeight);

	return FVector::CrossProduct(TangentX, TangentY).GetSafeNormal();
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
