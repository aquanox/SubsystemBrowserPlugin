// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_World.h"
#include "Subsystems/WorldSubsystem.h"

FSubsystemCategory_World::FSubsystemCategory_World()
{
	Name = TEXT("WorldSubsystemCategory");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_World", "World Subsystems");
	SortOrder = 400;
}

void FSubsystemCategory_World::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext))
	{
		OutData.Append(InContext->GetSubsystemArray<UWorldSubsystem>());
	}
}
