// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserColumn.h"

/**
 * "Name" column implementation
 */
struct FSubsystemDynamicColumn_Name : public FSubsystemDynamicTextColumn
{
	FSubsystemDynamicColumn_Name();

	virtual bool IsVisibleByDefault() const override { return true; }
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<class SSubsystemTableItem> TableRow) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;
	virtual FText ExtractText(TSharedRef<ISubsystemTreeItem> Item) const override;
	virtual FText ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const override;
};

namespace SubsystemColumns
{
	static const FName ColumnID_Name("Name");
}
