// Copyright 2022, Aquanox.

#include "SubsystemBrowserTestSubsystem.h"

#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/EngineVersionComparison.h"

USubsystemBrowserTestSubsystem::USubsystemBrowserTestSubsystem()
{
}

bool USubsystemBrowserTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#ifdef WITH_SB_HOST_PROJECT
	return WITH_SB_HOST_PROJECT && Super::ShouldCreateSubsystem(Outer);
#else
	return GetClass() != USubsystemBrowserTestSubsystem::StaticClass() && Super::ShouldCreateSubsystem(Outer);
#endif
}

void USubsystemBrowserTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USubsystemBrowserTestSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FString USubsystemBrowserTestSubsystem::GetSBOwnerName() const
{
	return TEXT("Hello from ") + GetNameSafe(GetOuter());
}

void USubsystemBrowserTestSubsystem::EditorFunction()
{
	++EditorFunctionCallCounter;
}

void USubsystemBrowserTestSubsystem::EditorFunctionReset()
{
	EditorFunctionCallCounter = 0;
}

void USubsystemBrowserTestSubsystem::RegisterSettings()
{
	ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));

	SettingsModule.RegisterSettings(TEXT("Subsystem"), TEXT("Sample"),
		GetClass()->GetFName(),
		GetClass()->GetDisplayNameText(),
		INVTEXT("Sample subsystem self-registered"),
		this);
}

void USubsystemBrowserTestSubsystem::UnRegisterSettings()
{
	ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));

	SettingsModule.UnregisterSettings(TEXT("Subsystem"), TEXT("Sample"), GetClass()->GetFName());
}

void USubsystemBrowserTestSubsystem::KillSettings()
{
#if UE_VERSION_OLDER_THAN(5,0,0)
	MarkPendingKill();
#else
	MarkAsGarbage();
#endif
}
