// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#pragma once
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionTime.generated.h"

UCLASS(collapsecategories, hidecategories=Object)
class UMaterialExpressionTime : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()

	/** This time continues advancing regardless of whether the game is paused. */
	UPROPERTY(EditAnywhere, Category=MaterialExpressionTime)
	uint32 bIgnorePause:1;


	// Begin UMaterialExpression Interface
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex, int32 MultiplexIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual bool NeedsRealtimePreview() override { return true; }
	// End UMaterialExpression Interface

};



