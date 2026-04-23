// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrain/ProcTerrainChunk.h"

#include "Components/SceneComponent.h"
#include "ProceduralTerrain/ProcBiomeDataAsset.h"
#include "ProceduralTerrain/ProcTerrainFunctionLibrary.h"

AProcTerrainChunk::AProcTerrainChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	ProcMesh->SetupAttachment(SceneRoot);
	ProcMesh->bUseAsyncCooking = true;
	ProcMesh->bUseComplexAsSimpleCollision = true;
	ProcMesh->CanCharacterStepUpOn = ECB_Yes;
	ProcMesh->SetMobility(EComponentMobility::Movable);
	ProcMesh->SetCollisionProfileName(TEXT("BlockAll"));
	ProcMesh->SetCollisionObjectType(ECC_WorldStatic);
}

void AProcTerrainChunk::InitializeChunk(const FIntPoint& InCoord, UProcBiomeDataAsset* InBiomeData, const FProcTerrainSettings& InTerrainSettings, int32 InSeed)
{
	ChunkCoord = InCoord;
	BiomeData = InBiomeData;
	TerrainSettings = InTerrainSettings;
	Seed = InSeed;
	TerrainSettings.Seed = Seed;

	SetActorLocation(UProcTerrainFunctionLibrary::ChunkCoordToWorldOrigin(ChunkCoord, TerrainSettings));

	if (BiomeData && BiomeData->TerrainMaterial)
	{
		ProcMesh->SetMaterial(0, BiomeData->TerrainMaterial);
	}

	ProcMesh->SetCollisionEnabled(TerrainSettings.bGenerateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	GenerateChunkMesh();
}

void AProcTerrainChunk::GenerateChunkMesh()
{
	ProcMesh->ClearAllMeshSections();

	if (TerrainSettings.ChunkVertsX < 2 || TerrainSettings.ChunkVertsY < 2 || TerrainSettings.VertexSpacing <= UE_SMALL_NUMBER)
	{
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	BuildMeshData(Vertices, Triangles, Normals, UVs, VertexColors, Tangents);

	ProcMesh->CreateMeshSection_LinearColor(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		Tangents,
		TerrainSettings.bGenerateCollision);

	if (BiomeData && BiomeData->TerrainMaterial)
	{
		ProcMesh->SetMaterial(0, BiomeData->TerrainMaterial);
	}
}

float AProcTerrainChunk::SampleHeight(float WorldX, float WorldY) const
{
	return UProcTerrainFunctionLibrary::SampleHeightAtWorldPosition(FVector2D(WorldX, WorldY), Seed, TerrainSettings);
}

void AProcTerrainChunk::BuildMeshData(
	TArray<FVector>& OutVertices,
	TArray<int32>& OutTriangles,
	TArray<FVector>& OutNormals,
	TArray<FVector2D>& OutUVs,
	TArray<FLinearColor>& OutVertexColors,
	TArray<FProcMeshTangent>& OutTangents) const
{
	const int32 VertexCountX = TerrainSettings.ChunkVertsX;
	const int32 VertexCountY = TerrainSettings.ChunkVertsY;
	const int32 QuadCountX = TerrainSettings.GetQuadCountX();
	const int32 QuadCountY = TerrainSettings.GetQuadCountY();
	const int32 TotalVertexCount = VertexCountX * VertexCountY;
	const int32 TotalTriangleIndexCount = QuadCountX * QuadCountY * 6;
	const FVector ChunkOrigin = GetActorLocation();

	OutVertices.Reserve(TotalVertexCount);
	OutNormals.Reserve(TotalVertexCount);
	OutUVs.Reserve(TotalVertexCount);
	OutVertexColors.Reserve(TotalVertexCount);
	OutTangents.Reserve(TotalVertexCount);
	OutTriangles.Reserve(TotalTriangleIndexCount);

	const auto VertexIndex = [VertexCountX](int32 VertexX, int32 VertexY)
	{
		return (VertexY * VertexCountX) + VertexX;
	};

	for (int32 VertexY = 0; VertexY < VertexCountY; ++VertexY)
	{
		for (int32 VertexX = 0; VertexX < VertexCountX; ++VertexX)
		{
			const float LocalX = VertexX * TerrainSettings.VertexSpacing;
			const float LocalY = VertexY * TerrainSettings.VertexSpacing;
			const float WorldX = ChunkOrigin.X + LocalX;
			const float WorldY = ChunkOrigin.Y + LocalY;
			const float Height = SampleHeight(WorldX, WorldY);

			OutVertices.Add(FVector(LocalX, LocalY, Height));
			OutNormals.Add(ComputeVertexNormal(WorldX, WorldY));
			OutUVs.Add(FVector2D(
				static_cast<float>(VertexX) / static_cast<float>(QuadCountX),
				static_cast<float>(VertexY) / static_cast<float>(QuadCountY)));
			OutVertexColors.Add(FLinearColor::White);
			OutTangents.Add(ComputeVertexTangent(WorldX, WorldY));
		}
	}

	for (int32 QuadY = 0; QuadY < QuadCountY; ++QuadY)
	{
		for (int32 QuadX = 0; QuadX < QuadCountX; ++QuadX)
		{
			const int32 BottomLeft = VertexIndex(QuadX, QuadY);
			const int32 BottomRight = VertexIndex(QuadX + 1, QuadY);
			const int32 TopLeft = VertexIndex(QuadX, QuadY + 1);
			const int32 TopRight = VertexIndex(QuadX + 1, QuadY + 1);

			OutTriangles.Add(BottomLeft);
			OutTriangles.Add(TopRight);
			OutTriangles.Add(BottomRight);

			OutTriangles.Add(BottomLeft);
			OutTriangles.Add(TopLeft);
			OutTriangles.Add(TopRight);
		}
	}
}

FVector AProcTerrainChunk::ComputeVertexNormal(float WorldX, float WorldY) const
{
	return UProcTerrainFunctionLibrary::ComputeNormalFromHeights(WorldX, WorldY, Seed, TerrainSettings);
}

FProcMeshTangent AProcTerrainChunk::ComputeVertexTangent(float WorldX, float WorldY) const
{
	const float SampleOffset = TerrainSettings.VertexSpacing;
	const float LeftHeight = SampleHeight(WorldX - SampleOffset, WorldY);
	const float RightHeight = SampleHeight(WorldX + SampleOffset, WorldY);
	const FVector Tangent = FVector(SampleOffset * 2.0f, 0.0f, RightHeight - LeftHeight).GetSafeNormal();

	return FProcMeshTangent(Tangent, false);
}
