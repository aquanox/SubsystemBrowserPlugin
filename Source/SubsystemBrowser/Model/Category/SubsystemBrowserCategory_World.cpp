// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_World.h"

#include "Misc/EngineVersionComparison.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/UObjectHash.h"
#include "UI/SubsystemTableItemTooltip.h"

FSubsystemCategory_World::FSubsystemCategory_World()
{
	Name = TEXT("WorldSubsystemCategory");
	SettingsName = TEXT("World");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_World", "World Subsystems");
	SortOrder = 400;
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

void FSubsystemCategory_World::SelectSettings(TArray<UObject*>& OutData) const
{
	::GetObjectsOfClass(UWorldSubsystem::StaticClass(), OutData, true, EObjectFlags::RF_NoFlags);
}

void FSubsystemCategory_World::GenerateTooltip(UWorld* InContext, class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const
{
	FText SanityResult;
	if (IsValid(InContext))
	{
		SanityResult = FSubsystemBrowserUtils::WorldTypeToText(InContext->WorldType);
	}
	else
	{
		SanityResult =  NSLOCTEXT("SubsystemBrowser", "WISS_State_NotRunning", "Inactive");
	}

	TooltipBuilder.AddPrimary(
		NSLOCTEXT("SubsystemBrowser", "WISS_State", "World Type"), SanityResult
	);
}
