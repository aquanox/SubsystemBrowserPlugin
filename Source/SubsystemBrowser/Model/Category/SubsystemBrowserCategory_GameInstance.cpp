// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"

FSubsystemCategory_GameInstance::FSubsystemCategory_GameInstance()
{
	Name = TEXT("GameInstanceCategory");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_GameInstance", "Game Instance Subsystems");
	SortOrder = 300;
}

void FSubsystemCategory_GameInstance::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext) && InContext->GetGameInstance())
	{
		OutData.Append(InContext->GetGameInstance()->GetSubsystemArray<UGameInstanceSubsystem>());
	}
}
