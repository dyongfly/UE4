// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class FAssetTypeActions_TextureCube : public FAssetTypeActions_Texture
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TextureCube", "Texture Cube"); }
	virtual FColor GetTypeColor() const override { return FColor(255,0,0); }
	virtual UClass* GetSupportedClass() const override { return UTextureCube::StaticClass(); }
	virtual bool CanFilter() override { return true; }
};