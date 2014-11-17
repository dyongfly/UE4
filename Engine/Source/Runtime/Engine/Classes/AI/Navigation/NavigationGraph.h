// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AI/Navigation/NavigationData.h"
#include "NavigationGraph.generated.h"

class UNavigationSystem;

USTRUCT()
struct FNavGraphEdge
{
	GENERATED_USTRUCT_BODY()

	enum {
		EgdeFlagsCount = 7
	};

	struct FNavGraphNode* Start;

	struct FNavGraphNode* End;

	int32 Flags : EgdeFlagsCount;
	uint32 bEnabled : 1;

	FNavGraphEdge()
		: Start(NULL)
		, End(NULL)
		, Flags(0)
		, bEnabled(false)
	{
	}
};


USTRUCT()
struct FNavGraphNode
{
	GENERATED_USTRUCT_BODY()

	/** Who's this node referring to? This will most commonly point to an actor or a component */
	UPROPERTY()
	UObject* Owner;

	enum {
		InitialEdgesCount = 4
	};

	TArray<FNavGraphEdge> Edges;
	// Location to be added here
	// Radius might be needed as well

	FNavGraphNode();
};

/** currently abstract since it's not full implemented */
UCLASS(config=Engine, MinimalAPI, abstract)
class ANavigationGraph : public ANavigationData
{
	GENERATED_UCLASS_BODY()

protected:
#if WITH_NAVIGATION_GENERATOR
	virtual FNavDataGenerator* ConstructGenerator(const FNavAgentProperties& AgentProps) override;
#endif // WITH_NAVIGATION_GENERATOR

public:
	static ANavigationData* CreateNavigationInstances(UNavigationSystem* NavSys);
};
