// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_Engine.h"

#include "Engine/Engine.h"
#include "Subsystems/EngineSubsystem.h"

FSubsystemCategory_Engine::FSubsystemCategory_Engine()
{
	Name = TEXT("EngineSubsystemCategory");
	SettingsName = TEXT("Engine");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_Engine", "Engine Subsystems");
	SortOrder = 100;
}

UClass* FSubsystemCategory_Engine::GetSubsystemClass() const
{
	return UEngineSubsystem::StaticClass();
}

void FSubsystemCategory_Engine::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (GEngine)
	{
		return OutData.Append(GEngine->GetEngineSubsystemArray<UEngineSubsystem>());
	}
}
