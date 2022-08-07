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

	virtual FText ExtractText(TSharedRef<const ISubsystemTreeItem> Item) const override;
	virtual FText ExtractTooltipText(TSharedRef<const ISubsystemTreeItem> Item) const override { return FText::GetEmpty(); }
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;

};