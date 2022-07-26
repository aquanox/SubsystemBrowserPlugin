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

	virtual bool IsVisibleByDefault() const override { return true; }

	virtual FText ExtractText(TSharedRef<ISubsystemTreeItem> Item) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;
};

namespace SubsystemColumns
{
	static const FName ColumnID_Config("Config");
}