// Copyright 2022, Aquanox.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"

class FSubsystemSettingsEditorModule : public IModuleInterface
{
public:
	static FSubsystemSettingsEditorModule& Get()
	{
		return FModuleManager::GetModuleChecked<FSubsystemSettingsEditorModule>(TEXT("SubsystemSettingsEditor"));
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override { return false; }

	SUBSYSTEMSETTINGSEDITOR_API void RequestDiscoverSubsystems() const;
private:

	/** register settings editor menus */
	void RegisterMenus();

	/** Settings manager instance */
	TSharedPtr<class FSubsystemSettingsManager> SettingsManager;
};

DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemSettingsEditor, Log, All);
