// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_GameInstance.h"

#include "Misc/EngineVersionComparison.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/UObjectHash.h"
#include "UI/SubsystemTableItemTooltip.h"

FSubsystemCategory_GameInstance::FSubsystemCategory_GameInstance()
{
	Name = TEXT("GameInstanceCategory");
	SettingsName = TEXT("GameInstance");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_GameInstance", "Game Instance Subsystems");
	SortOrder = 300;
}

void FSubsystemCategory_GameInstance::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext) && InContext->GetGameInstance())
	{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutData.Append(InContext->GetGameInstance()->GetSubsystemArray<UGameInstanceSubsystem>());
#else
		OutData.Append(InContext->GetGameInstance()->GetSubsystemArrayCopy<UGameInstanceSubsystem>());
#endif
	}
}

void FSubsystemCategory_GameInstance::SelectSettings(TArray<UObject*>& OutData) const
{
	::GetObjectsOfClass(UGameInstanceSubsystem::StaticClass(), OutData, true, EObjectFlags::RF_NoFlags);
}

void FSubsystemCategory_GameInstance::GenerateTooltip(UWorld* InContext, class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const
{
	FText SanityResult;
	if (IsValid(InContext) && InContext->GetGameInstance())
	{
		SanityResult =  NSLOCTEXT("SubsystemBrowser", "GISS_State_Running", "Active");
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		int32 NumSubsystems = InContext->GetGameInstance()->GetSubsystemArray<UGameInstanceSubsystem>().Num();
#else
		int32 NumSubsystems = InContext->GetGameInstance()->GetSubsystemArrayCopy<UGameInstanceSubsystem>().Num();
#endif
		if (!NumSubsystems)
		{
			SanityResult =  NSLOCTEXT("SubsystemBrowser", "GISS_State_Bad", "Not Initialized");
		}
	}
	else
	{
		SanityResult =  NSLOCTEXT("SubsystemBrowser", "GISS_State_NotRunning", "Inactive");
	}

	TooltipBuilder.AddPrimary(
		NSLOCTEXT("SubsystemBrowser", "GISS_State", "Game Instance"), SanityResult
	);
}
