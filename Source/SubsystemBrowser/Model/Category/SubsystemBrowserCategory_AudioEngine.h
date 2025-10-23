// Copyright 2022, Aquanox.

#pragma once

#include "SubsystemBrowserFlags.h"
#include "Model/SubsystemBrowserCategory.h"

#if UE_VERSION_NEWER_THAN(5, 1, 0)

/**
 * Standard Category for Audio Engine Subsystems
 */
struct SUBSYSTEMBROWSER_API FSubsystemCategory_AudioEngine : public FSubsystemCategory
{
	FSubsystemCategory_AudioEngine();
	
	virtual bool IsVisibleByDefaultInBrowser() const override { return false; }
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;

	virtual bool IsVisibleInSettings() const override { return true; }
	virtual void SelectSettings(TArray<UObject*>& OutData) const override;
};

#endif
