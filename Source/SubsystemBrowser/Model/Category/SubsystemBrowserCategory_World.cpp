// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_World.h"

#include "Misc/EngineVersionComparison.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"

FSubsystemCategory_World::FSubsystemCategory_World()
{
	Name = TEXT("WorldSubsystemCategory");
	SettingsName = TEXT("World");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_World", "World Subsystems");
	SortOrder = 400;
}

UClass* FSubsystemCategory_World::GetSubsystemClass() const
{
	return UWorldSubsystem::StaticClass();
}

void FSubsystemCategory_World::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext))
	{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutData.Append(InContext->GetSubsystemArray<UWorldSubsystem>());
#else
		OutData.Append(InContext->GetSubsystemArrayCopy<UWorldSubsystem>());
#endif
	}
}
