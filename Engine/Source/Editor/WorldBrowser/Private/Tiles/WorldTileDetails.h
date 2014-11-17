// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "WorldTileDetails.generated.h"

class FWorldTileModel;

/////////////////////////////////////////////////////
// FTileLODEntryDetails
//
// Helper class to hold tile LOD level description
//
USTRUCT()
struct FTileLODEntryDetails
{
	GENERATED_USTRUCT_BODY()
	
	// Maximum deviation of details percentage
	UPROPERTY(Category=LODDetails, VisibleAnywhere)
	int32		LODIndex;
			
	// Relative to original tile streaming distance
	UPROPERTY(Category=LODDetails, EditAnywhere, meta=(ClampMin = "10", ClampMax = "10000000", UIMin = "10", UIMax = "500000"))
	int32		Distance;

	// Percentage of details relative to main tile details
	UPROPERTY(Category=ReductionSettings, EditAnywhere, meta=(ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))	
	float		DetailsPercentage;
	
	// Maximum deviation of details percentage
	UPROPERTY(Category=ReductionSettings, EditAnywhere, meta=(ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))	
	float		MaxDeviation;

	FTileLODEntryDetails();
};

/////////////////////////////////////////////////////
// UWorldTileDetails
// 
// Helper class to hold world tile information
// Holding this information in UObject gives us ability to use property editors and support undo operations
// 
UCLASS()
class UWorldTileDetails : public UObject
{
	GENERATED_UCLASS_BODY()
	
	// Whether this tile properties can be edited via details panel
	UPROPERTY(Category=Tile, VisibleAnywhere)
	bool							bTileEditable;
		
	// Tile long package name (readonly)	
	UPROPERTY(Category=Tile, VisibleAnywhere)
	FName							PackageName;
	
	// Parent tile long package name	
	UPROPERTY(Category=Tile, EditAnywhere)
	FName							ParentPackageName;
	
	// Tile position in the world, relative to parent 
	UPROPERTY(Category=Tile, EditAnywhere )
	FIntPoint						Position;

	// Tile absolute position in the world (readonly)
	UPROPERTY(Category=Tile, VisibleAnywhere)
	FIntPoint						AbsolutePosition;

	// Tile sorting order
	UPROPERTY(Category=Tile, EditAnywhere, meta=(ClampMin = "-1000", ClampMax = "1000", UIMin = "-1000", UIMax = "1000"))
	int32							ZOrder;
	
	// LOD entries number
	UPROPERTY(Category=LODSettings, EditAnywhere, meta=(ClampMin = "0", ClampMax = "4", UIMin = "0", UIMax = "4"))
	int32							NumLOD;

	UPROPERTY(Category=LODSettings, EditAnywhere)
	FTileLODEntryDetails			LOD1;

	UPROPERTY(Category=LODSettings, EditAnywhere)
	FTileLODEntryDetails			LOD2;

	UPROPERTY(Category=LODSettings, EditAnywhere)
	FTileLODEntryDetails			LOD3;

	UPROPERTY(Category=LODSettings, EditAnywhere)
	FTileLODEntryDetails			LOD4;
			
	// Tile layer information
	FWorldTileLayer					Layer;
	
	// Tile bounds
	FBox							Bounds;

	// Whether this tile is a persistent level
	bool							bPersistentLevel;

	// Events when user edits tile properties
	DECLARE_EVENT( UWorldTileDetails, FTilePropertyChanged )

	FTilePropertyChanged			PositionChangedEvent;
	FTilePropertyChanged			ParentPackageNameChangedEvent;
	FTilePropertyChanged			LODSettingsChangedEvent;
	FTilePropertyChanged			ZOrderChangedEvent;
	
	// UObject interface
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
	// todo: undo operations

public:
	// Initialize tile details with values stored in FWorldTileInfo object
	void SetInfo(const FWorldTileInfo& Info);
	
	// Gets the initialized FWorldTileInfo from this details values
	FWorldTileInfo GetInfo() const;

};
