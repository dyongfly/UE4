// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ComponentVisualizer.h"

struct FViewportClick;

/** Class that managed active component visualizer and routes input to it */
class UNREALED_API FComponentVisualizerManager
{
public:
	FComponentVisualizerManager() {}
	virtual ~FComponentVisualizerManager() {}


	/** Activate a component visualizer given a clicked proxy */
	bool HandleProxyForComponentVis(HHitProxy *HitProxy);
	/** Clear active component visualizer */
	void ClearActiveComponentVis();

	/** Handle a click on the specified level editor viewport client */
	bool HandleClick(FLevelEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click);
	/** Pass key input to active visualizer */
	bool HandleInputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) const;
	/** Pass delta input to active visualizer */
	bool HandleInputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) const;
	/** Get widget location from active visualizer */
	bool GetWidgetLocation(FVector& OutLocation) const;

	/** Generate context menu for the component visualizer */
	TSharedPtr<SWidget> GenerateContextMenuForComponentVis() const;

private:
	/** Currently 'active' visualizer that we should pass input to etc */
	TWeakPtr<class FComponentVisualizer> EditedVisualizer;
};