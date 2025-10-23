// Copyright 2022, Aquanox.

#include "Model/Category/SubsystemBrowserCategory_Engine.h"

#include "Misc/EngineVersionComparison.h"
#include "Engine/Engine.h"
#include "Subsystems/EngineSubsystem.h"
#include "UObject/UObjectHash.h"

FSubsystemCategory_Engine::FSubsystemCategory_Engine()
{
	Name = TEXT("EngineSubsystemCategory");
	SettingsName = TEXT("Engine");
	Label = NSLOCTEXT("SubsystemBrowser", "SubsystemBrowser_Engine", "Engine Subsystems");
	SortOrder = 100;
}

void FSubsystemCategory_Engine::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	if (GEngine)
	{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		return OutData.Append(GEngine->GetEngineSubsystemArray<UEngineSubsystem>());
#else
		return OutData.Append(GEngine->GetEngineSubsystemArrayCopy<UEngineSubsystem>());
#endif
	}
}

void FSubsystemCategory_Engine::SelectSettings(TArray<UObject*>& OutData) const
{
	::GetObjectsOfClass(UEngineSubsystem::StaticClass(), OutData, true, EObjectFlags::RF_NoFlags);
}
