// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_GameInstance.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"

FSubsystemCategory_GameInstance::FSubsystemCategory_GameInstance()
{
	Name = TEXT("GameInstanceCategory");
	SettingsName = TEXT("GameInstance");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_GameInstance", "Game Instance Subsystems");
	SortOrder = 300;
}

UClass* FSubsystemCategory_GameInstance::GetSubsystemClass() const
{
	return UGameInstanceSubsystem::StaticClass();
}

void FSubsystemCategory_GameInstance::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (IsValid(InContext) && InContext->GetGameInstance())
	{
		OutData.Append(InContext->GetGameInstance()->GetSubsystemArray<UGameInstanceSubsystem>());
	}
}
