// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SpriteEditorOnlyTypes.generated.h"

// A single polygon, may be convex or concave, etc...
USTRUCT()
struct FSpritePolygon
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category=Physics, EditAnywhere)
	TArray<FVector2D> Vertices;

	UPROPERTY()//Category=Physics, EditAnywhere)
	FVector2D BoxSize;
	
	UPROPERTY()//Category=Physics, EditAnywhere)
	FVector2D BoxPosition;
};


// Method of specifying polygons for a sprite's render or collision data
UENUM()
namespace ESpritePolygonMode
{
	enum Type
	{
		// Use the bounding box of the source sprite (no optimization)
		SourceBoundingBox,

		// Tighten the bounding box around the sprite to exclude fully transparent areas (the default)
		TightBoundingBox,

		// Bounding box that can be edited explicitly
		//CustomBox,

		// Shrink-wrapped geometry
		ShrinkWrapped,

		// Fully custom geometry; edited by hand
		FullyCustom,
	};
}

USTRUCT()
struct FSpritePolygonCollection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category=PolygonData, EditAnywhere)
	TArray<FSpritePolygon> Polygons;

	// The geometry type
	UPROPERTY(Category=PolygonData, EditAnywhere)
	TEnumAsByte<ESpritePolygonMode::Type> GeometryType;

	// Experimental: Hint to the triangulation routine that extra vertices should be preserved
	UPROPERTY(Category=PolygonData, EditAnywhere, AdvancedDisplay)
	bool bAvoidVertexMerging;

	// Alpha threshold for a transparent pixel (range 0..1, anything equal or below this value will be considered unimportant)
	UPROPERTY(Category=PolygonData, EditAnywhere, AdvancedDisplay)
	float AlphaThreshold;

	// This is the threshold below which multiple vertices will be merged together when doing shrink-wrapping.  Higher values result in fewer vertices.
	UPROPERTY(Category=PolygonData, EditAnywhere, AdvancedDisplay)
	float SimplifyEpsilon;

	FSpritePolygonCollection()
		: GeometryType(ESpritePolygonMode::TightBoundingBox)
		, bAvoidVertexMerging(false)
		, AlphaThreshold(0.0f)
		, SimplifyEpsilon(2.0f)
	{
	}

	void Reset()
	{
		Polygons.Empty();
		GeometryType = ESpritePolygonMode::TightBoundingBox;
	}
};

UENUM()
namespace ESpritePivotMode
{
	enum Type
	{
		Top_Left,
		Top_Center,
		Top_Right,
		Center_Left,
		Center_Center,
		Center_Right,
		Bottom_Left,
		Bottom_Center,
		Bottom_Right,
		Custom
	};
}
