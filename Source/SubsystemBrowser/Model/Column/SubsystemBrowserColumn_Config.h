// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserColumn.h"

/**
 * "Config" column implementation
 */
struct FSubsystemDynamicColumn_Config : public FSubsystemDynamicTextColumn
{
	using Super = FSubsystemDynamicTextColumn;

	FSubsystemDynamicColumn_Config();

	virtual bool IsVisibleByDefault() const override { return false; }

	virtual FText ExtractText(TSharedRef<const ISubsystemTreeItem> Item) const override;
	virtual FText ExtractTooltipText(TSharedRef<const ISubsystemTreeItem> Item) const override { return FText::GetEmpty(); }
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;
};