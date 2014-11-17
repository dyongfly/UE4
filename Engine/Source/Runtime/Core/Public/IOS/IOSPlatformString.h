// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*================================================================================
	IOSPlatformString.h: Additional NSString methods
==================================================================================*/

#pragma once

#import <Foundation/NSString.h>

class FString;

@interface NSString (FString_Extensions)

/**
* Converts an TCHAR string to an NSString
*/
+ (NSString*) stringWithTCHARString:(const TCHAR*)MyTCHARString;

/**
 * Converts an FString to an NSString
 */
+ (NSString*) stringWithFString:(const FString&)MyFString;

@end
