// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#pragma once
#include "Engine/NiagaraScriptSourceBase.h"
#include "NiagaraScriptSource.generated.h"

UCLASS(MinimalAPI)
class UNiagaraScriptSource : public UNiagaraScriptSourceBase
{
	GENERATED_UCLASS_BODY()

	/** Graph for particle update expression */
	UPROPERTY()
	class UNiagaraGraph*	UpdateGraph;

	// UObject interface.
	virtual void PostLoad() override;

	/** Get the set of outputs from the Update script */
	UNREALED_API void GetUpdateOutputs(TArray<FName>& ScalarOutputs);

	/** Get the set of input attributes that can be used in the Update script */
	UNREALED_API void GetUpdateInputs(TArray<FName>& ScalarInputs);
};
