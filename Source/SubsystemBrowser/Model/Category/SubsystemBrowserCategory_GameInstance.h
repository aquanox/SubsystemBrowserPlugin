// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserCategory.h"

/**
 * Standard Category for Game Instance Subsystems
 */
struct SUBSYSTEMBROWSER_API FSubsystemCategory_GameInstance : public FSubsystemCategory
{
	FSubsystemCategory_GameInstance();

	virtual bool IsVisibleByDefaultInBrowser() const override { return true; }
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;

	virtual bool IsVisibleInSettings() const override { return true; }
	virtual void SelectSettings(TArray<UObject*>& OutData) const override;
};
