// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Credit screen widget that displays a scrolling list contributors.  
 */
class SCreditsScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SCreditsScreen )
		{}
	SLATE_END_ARGS()

	/**
	 * Constructs the credits screen widgets                   
	 */
	UNREALED_API void Construct(const FArguments& InArgs);

	UNREALED_API virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	/** Handles the user clicking the play/pause toggle button. */
	FReply HandleTogglePlayPause();

	/** Handles when the user scrolls so that we can stop the auto-scrolling when they scroll backwards. */
	void HandleUserScrolled(float ScrollOffset);

	/** Handles the user clicking links in the credits */
	void OnBrowserLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata);

	/** Gets the current brush (play or pause) icon for the play/pause button */
	const FSlateBrush* GetTogglePlayPauseBrush() const;

	/** The widget that scrolls the credits text */
	TSharedPtr<SScrollBox> ScrollBox;

	/** The auto scroll rate in pixels per second */
	float ScrollPixelsPerSecond;

	/** The last recorded scroll position so we can detect the user scrolling up */
	float PreviousScrollPosition;

	/** If we are playing the auto scroll effect */
	bool bIsPlaying;
};

