// Copyright 2022, Aquanox.

#pragma once

#include "Modules/ModuleInterface.h"
#include "SubsystemSettingsManager.h"

class FSubsystemSettingsEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:

	/** register settings editor menus */
	void RegisterMenus();


	//
	bool bEnabled = false;

	// Settings manager
	FSubsystemSettingsManager SettingsManager;
};

DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemSettingsEditor, Log, All);
