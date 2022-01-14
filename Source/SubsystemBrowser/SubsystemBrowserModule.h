// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class USubsystemBrowserSettings;

class FSubsystemBrowserModule : public IModuleInterface
{
	static const FName SubsystemBrowserTabName;

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	// no hotreload allowed
	virtual bool SupportsDynamicReloading() override { return false; }

private:
	static TSharedRef<class SDockTab> HandleTabManagerSpawnTab(const class FSpawnTabArgs& Args);
	static TSharedRef<class SWidget> CreateSubsystemBrowser(const class FSpawnTabArgs& Args);

	static void SummonSubsystemTab();

	TSharedPtr<class ISettingsSection> SettingsSection;
};

#if UE_BUILD_DEBUG
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, All);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, Warning);
#endif
