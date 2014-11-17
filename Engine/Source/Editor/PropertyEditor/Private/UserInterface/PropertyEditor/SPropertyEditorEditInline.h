// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PropertyEditorConstants.h"

class SPropertyEditorEditInline : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS( SPropertyEditorEditInline ) 
		: _Font( FEditorStyle::GetFontStyle( PropertyEditorConstants::PropertyFontStyle ) )
		{}
		SLATE_ARGUMENT( FSlateFontInfo, Font )
	SLATE_END_ARGS()

	static bool Supports( const TSharedRef< class FPropertyEditor >& InPropertyEditor );
	static bool Supports( const FPropertyNode* InTreeNode, int32 InArrayIdx );

	void Construct( const FArguments& InArgs, const TSharedRef< class FPropertyEditor >& InPropertyEditor );

	void GetDesiredWidth( float& OutMinDesiredWidth, float& OutMaxDesiredWidth );


private:
	/**
	 * @return The current display value for the combo box as a string
	 */
	FString GetDisplayValueAsString() const;

	/**
	 * @return The current display value's icon, if any. Returns nullptr if we have no valid value.
	 */
	const FSlateBrush* GetDisplayValueIcon() const;

	/**
	 * Wrapper method for determining whether a class is valid for use by this property item input proxy.
	 *
	 * @param	InItem			the property window item that contains this proxy.
	 * @param	CheckClass		the class to verify
	 * @param	bAllowAbstract	true if abstract classes are allowed
	 *
	 * @return	true if CheckClass is valid to be used by this input proxy
	 */
	bool IsClassAllowed( UClass* CheckClass, bool bAllowAbstract ) const;
	
        /** 
	 * Generates a class picker with a filter to show only classes allowed to be selected. 
	 *
	 * @return The Class Picker widget.
	 */
	TSharedRef<SWidget> GenerateClassPicker();

	/** 
	 * Callback function from the Class Picker for when a Class is picked.
	 *
	 * @param InClass			The class picked in the Class Picker
	 */
	void OnClassPicked(UClass* InClass);


private:

	TSharedPtr<class FPropertyEditor > PropertyEditor;

	TSharedPtr<class SComboButton> ComboButton;

	/** Whether to use display names or literal class names */
	bool bUseDisplayNames;
};
