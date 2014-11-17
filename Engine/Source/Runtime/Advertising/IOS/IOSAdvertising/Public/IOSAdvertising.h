// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IAdvertisingProvider.h"
#include "Core.h"

class FIOSAdvertisingProvider : public IAdvertisingProvider
{
public:
	virtual void ShowAdBanner( bool bShowOnBottomOfScreen ) override;
	virtual void HideAdBanner() override;
	virtual void CloseAdBanner() override;
};
