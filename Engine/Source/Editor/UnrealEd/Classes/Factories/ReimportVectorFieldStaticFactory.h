// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

//=============================================================================
// ReimportTextureFactory
//=============================================================================

#pragma once
#include "ReimportVectorFieldStaticFactory.generated.h"

UCLASS()
class UReimportVectorFieldStaticFactory : public UVectorFieldStaticFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	// Begin FReimportHandler interface
	virtual bool CanReimport( UObject* Obj, TArray<FString>& OutFilenames ) override;
	virtual void SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths ) override;
	virtual EReimportResult::Type Reimport( UObject* Obj ) override;
	// End FReimportHandler interface
};



