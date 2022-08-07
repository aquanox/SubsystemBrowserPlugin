// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserColumn.h"

/**
 * "Name" column implementation
 */
struct FSubsystemDynamicColumn_Name : public FSubsystemDynamicTextColumn
{
	using Super = FSubsystemDynamicTextColumn;

	FSubsystemDynamicColumn_Name();

	virtual bool IsVisibleByDefault() const override { return true; }
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<const ISubsystemTreeItem> Item, TSharedRef<class SSubsystemTableItem> TableRow) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;
	virtual FText ExtractText(TSharedRef<const ISubsystemTreeItem> Item) const override;
	virtual FText ExtractTooltipText(TSharedRef<const ISubsystemTreeItem> Item) const override { return FText::GetEmpty(); }
	virtual FSlateColor ExtractColor(TSharedRef<const ISubsystemTreeItem> Item) const override;
};
