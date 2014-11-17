// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once



/**
 * Represents the root node in a hierarchy of DockNodes.
 */
class SLATE_API SDockingArea : public SDockingSplitter
{
public:

	SLATE_BEGIN_ARGS(SDockingArea)
		: _ParentWindow()
		, _ShouldManageParentWindow(true)
		, _InitialContent()
		{
			// Visible by default, but don't absorb clicks
			_Visibility = EVisibility::SelfHitTestInvisible;
		}
		
		/* The window whose content area this dock area is directly embedded within.  By default, ShouldManageParentWindow is
		   set to true, which means the dock area will also destroy the window when the last tab goes away.  Assigning a
		   parent window also allows the docking area to embed title area widgets (minimize, maximize, etc) into its content area */
		SLATE_ARGUMENT( TSharedPtr<SWindow>, ParentWindow )

		/** True if this docking area should close the parent window when the last tab in this docking area goes away */
		SLATE_ARGUMENT( bool, ShouldManageParentWindow )

		 /**
		 * What to put into the DockArea initially. Usually a TabStack, so that some tabs can be added to it.
		 */
		SLATE_ARGUMENT( TSharedPtr<SDockingNode>, InitialContent )

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, const TSharedRef<FTabManager>& InTabManager, const TSharedRef<FTabManager::FArea>& PersistentNode );

	virtual Type GetNodeType() const
	{
		return SDockingNode::DockArea;
	}
	
	/** Returns this dock area */
	virtual TSharedPtr<SDockingArea> GetDockArea() override;
	virtual TSharedPtr<const SDockingArea> GetDockArea() const override;

	/** Returns the window that this dock area resides in directly and also manages */
	TSharedPtr<SWindow> GetParentWindow() const;

	virtual void OnDragEnter( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	virtual void OnDragLeave( const FDragDropEvent& DragDropEvent ) override;
	
	virtual FReply OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;

	virtual FReply OnUserAttemptingDock( SDockingNode::RelativeDirection Direction, const FDragDropEvent& DragDropEvent ) override;

	void OnTabFoundNewHome( const TSharedRef<SDockTab>& RelocatedTab, const TSharedRef<SWindow>& NewOwnerWindow );

	// Show the dock-from-outside dock targets
	void ShowCross();

	// Hide the dock-from-outside dock targets
	void HideCross();

	/**
	 * Removes redundant stack and splitters. Collapses any widgets that any no longer showing live content.
	 */
	void CleanUp( ELayoutModification RemovalMethod );

	void SetParentWindow( const TSharedRef<SWindow>& NewParentWindow );

	virtual TSharedPtr<FTabManager::FLayoutNode> GatherPersistentLayout() const override;

	TSharedRef<FTabManager> GetTabManager() const;

protected:
	virtual SDockingNode::ECleanupRetVal CleanUpNodes() override;

private:
	EVisibility TargetCrossVisibility() const;
	EVisibility TargetCrossCenterVisibility() const;
	/** Dock a tab along the outer edge of this DockArea */
	void DockFromOutside(SDockingNode::RelativeDirection Direction, const FDragDropEvent& DragDropEvent);

	/** We were placed in a window, and it is being destroyed */
	void OnOwningWindowBeingDestroyed(const TSharedRef<SWindow>& WindowBeingDestroyed);

	virtual void OnLiveTabAdded() override;

private:	
	
	/** The window this dock area is embedded within.  If bIsManagingParentWindow is true, the dock area will also
	    destroy the window when the last tab goes away. */
	TWeakPtr<SWindow> ParentWindowPtr;

	/** True if this docking area should close the parent window when the last tab in this docking area goes away */
	bool bManageParentWindow;

	/** The tab manager that controls this DockArea */
	TWeakPtr<FTabManager> MyTabManager;

	/** The overlay is visible when the user is dragging a tab over the dock area */
	bool bIsOverlayVisible;

	/** The center target is visible when the overlay is visible and there are no live tabs */
	bool bIsCenterTargetVisible;

	/** True when the last tab has been pulled from this area, meaning that this DockArea will not be necessary once that tab finds a new home. */
	bool bCleanUpUponTabRelocation;
};
