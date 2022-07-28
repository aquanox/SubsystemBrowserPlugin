// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserColumn.h"

/*
 * "Module" column implementation
 */
struct FSubsystemDynamicColumn_Module : public FSubsystemDynamicTextColumn
{
	using Super = FSubsystemDynamicTextColumn;

	FSubsystemDynamicColumn_Module();

	virtual bool IsVisibleByDefault() const override { return true; }

	virtual FText ExtractText(TSharedRef<ISubsystemTreeItem> Item) const override;
	virtual FText ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;

};