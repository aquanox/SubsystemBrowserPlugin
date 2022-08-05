// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserColumn.h"

/*
 * "Plugin" column implementation
 */
struct FSubsystemDynamicColumn_Plugin : public FSubsystemDynamicTextColumn
{
	using Super = FSubsystemDynamicTextColumn;

	FSubsystemDynamicColumn_Plugin();

	virtual bool IsVisibleByDefault() const override { return false; }

	virtual FText ExtractText(TSharedRef<ISubsystemTreeItem> Item) const override;
	virtual FText ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;

};