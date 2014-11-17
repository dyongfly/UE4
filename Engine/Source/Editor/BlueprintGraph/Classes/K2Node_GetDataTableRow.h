// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#pragma once
#include "K2Node.h"
#include "K2Node_GetDataTableRow.generated.h"

UCLASS()
class UK2Node_GetDataTableRow : public UK2Node
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	FDataTableRowHandle DataTableRowHandle;

	// Begin UEdGraphNode interface.
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual FString GetTooltip() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FName GetPaletteIcon(FLinearColor& OutColor) const override
    {
        OutColor = GetNodeTitleColor();
        return TEXT("Kismet.AllClasses.FunctionIcon");
    }
	// End UEdGraphNode interface.

	// Begin UK2Node interface
	virtual bool IsNodeSafeToIgnore() const override { return true; }
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	// End UK2Node interface


	/** Create new pins to show properties on archetype */
	void SetReturnTypeForStruct(UScriptStruct* InClass);
	/** See if this is a spawn variable pin, or a 'default' pin */
	bool IsDataTablePin(UEdGraphPin* Pin);

	/** Get the then output pin */
	UEdGraphPin* GetThenPin() const;
	/** Get the Data Table input pin */
	UEdGraphPin* GetDataTablePin(const TArray<UEdGraphPin*>* InPinsToSearch=NULL) const;
	/** Get the spawn transform input pin */	
	UEdGraphPin* GetRowNamePin() const;
    /** Get the exec output pin for when the row was not found */
	UEdGraphPin* GetRowNotFoundPin() const;
	/** Get the result output pin */
	UEdGraphPin* GetResultPin() const;

	/** Get the type of the TableRow to return */
	UScriptStruct* GetDataTableRowStructType(const TArray<UEdGraphPin*>* InPinsToSearch = NULL) const;

private:
	/**
	 * Takes the specified "MutatablePin" and sets its 'PinToolTip' field (according
	 * to the specified description)
	 * 
	 * @param   MutatablePin	The pin you want to set tool-tip text on
	 * @param   PinDescription	A string describing the pin's purpose
	 */
	void SetPinToolTip(UEdGraphPin& MutatablePin, const FText& PinDescription) const;

	/** Tooltip text for this node. */
	FString NodeTooltip;
};
