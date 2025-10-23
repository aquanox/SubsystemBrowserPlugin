// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserCategory.h"

/**
 * Standard Category for World Subsystems
 */
struct SUBSYSTEMBROWSER_API FSubsystemCategory_World : public FSubsystemCategory
{
	FSubsystemCategory_World();

	virtual bool IsVisibleByDefaultInBrowser() const override { return true; }
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;

	virtual bool IsVisibleInSettings() const override { return true; }
	virtual void SelectSettings(TArray<UObject*>& OutData) const override;

	virtual void GenerateTooltip(UWorld* InContext, class FSubsystemTableItemTooltipBuilder& TooltipBuilder) const override;
};
