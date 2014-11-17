// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "IPropertyTableCellPresenter.h"

class FBooleanPropertyTableCellPresenter : public TSharedFromThis< FBooleanPropertyTableCellPresenter >, public IPropertyTableCellPresenter
{
public:

	FBooleanPropertyTableCellPresenter( const TSharedRef< class FPropertyEditor >& InPropertyEditor );

	virtual TSharedRef< class SWidget > ConstructDisplayWidget() override;

	virtual bool RequiresDropDown() override;

	virtual TSharedRef< class SWidget > ConstructEditModeCellWidget() override;

	virtual TSharedRef< class SWidget > ConstructEditModeDropDownWidget() override;

	virtual TSharedRef< class SWidget > WidgetToFocusOnEdit() override;

	virtual FString GetValueAsString() override;

	virtual FText GetValueAsText() override;

	virtual bool HasReadOnlyEditMode() override { return false; }


private:

	TSharedPtr< class SWidget > FocusWidget;
	TSharedRef< class FPropertyEditor > PropertyEditor;
};