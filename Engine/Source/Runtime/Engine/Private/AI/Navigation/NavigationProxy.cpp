// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "EnginePrivate.h"
#include "AI/Navigation/NavigationProxy.h"

UNavigationProxy::UNavigationProxy(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	bDirty = false;
}
