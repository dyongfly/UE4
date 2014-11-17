// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "EnginePrivate.h"
#include "AI/Navigation/NavFilters/RecastFilter_UseDefaultArea.h"

URecastFilter_UseDefaultArea::URecastFilter_UseDefaultArea(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
}

void URecastFilter_UseDefaultArea::InitializeFilter(const ANavigationData* NavData, FNavigationQueryFilter* Filter) const
{
#if WITH_RECAST
	Filter->SetFilterImplementation((const INavigationQueryFilterInterface*)ARecastNavMesh::GetNamedFilter(ERecastNamedFilter::FilterOutAreas));
#endif // WITH_RECAST

	Super::InitializeFilter(NavData, Filter);
}
