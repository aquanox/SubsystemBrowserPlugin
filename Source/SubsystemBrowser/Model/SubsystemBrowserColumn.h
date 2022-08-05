// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserDescriptor.h"

/* Represents a configurable column */
struct SUBSYSTEMBROWSER_API FSubsystemDynamicColumn : public TSharedFromThis<FSubsystemDynamicColumn>
{
	/* Column config identifier */
	FName Name;
	/* Column display name for table header */
	FText TableLabel;
	/* Column display name for menu */
	FText ConfigLabel;
	/* Column preferred width ratio (from 0 to 1) */
	float PreferredWidthRatio = 0.1f;
	/* Column sort order */
	int32 SortOrder = 0;

	FSubsystemDynamicColumn();
	virtual ~FSubsystemDynamicColumn() = default;

	const FName& GetID() const { return Name; }
	int32 GetSortOrder() const { return SortOrder; }

	/**
	 * Generate visual representation of column in header row
	 */
	virtual SHeaderRow::FColumn::FArguments GenerateHeaderColumnWidget() const;

	/**
	 * Generate visual representation of column in table row
	 */
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<class SSubsystemTableItem> TableRow) const = 0;

	/**
	 * Gather searchable strings for column
	 */
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const {}

	/**
	 * Does this column support sorting? WIP
	 */
	virtual bool SupportsSorting() const { return false; }

	/**
	 * Perform sorting of table items. WIP
	 */
	virtual void SortItems(TArray<SubsystemTreeItemPtr>& RootItems, const EColumnSortMode::Type SortMode) const {}

	/**
	 * Test if custom column name is valid (not None or permanent column)
	 */
	static bool IsValidColumnName(FName InName);

	/**
	 * Get default column visibility state
	 */
	virtual bool IsVisibleByDefault() const { return false; }
};

using SubsystemColumnPtr = TSharedPtr<FSubsystemDynamicColumn>;

struct SubsystemColumnSorter
{
	bool operator()(const SubsystemColumnPtr& A, const SubsystemColumnPtr& B) const
	{
		return A->SortOrder < B->SortOrder;
	}
};

/**
 * A prefab type for simple columns that have text representation
 */
struct SUBSYSTEMBROWSER_API FSubsystemDynamicTextColumn : public FSubsystemDynamicColumn
{
	using Super = FSubsystemDynamicTextColumn;

	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<class SSubsystemTableItem> TableRow) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override { }
protected:
	/* get text to display for specified item */
	virtual FText ExtractText(TSharedRef<ISubsystemTreeItem> Item) const = 0;
	/* get tooltip text to display for specified item */
	virtual FText ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const;
	/* get color and opacity of text for specified item */
	virtual FSlateColor ExtractColor(TSharedRef<ISubsystemTreeItem> Item) const;
	/* internal */
	FSlateColor ExtractColorIfEnabled(TSharedRef<ISubsystemTreeItem> Item) const;
	/* get font of text for specified item */
	virtual FSlateFontInfo ExtractFont(TSharedRef<ISubsystemTreeItem> Item) const;
	/* text columns support sorting by default */
	virtual bool SupportsSorting() const override { return true; }
	virtual void SortItems(TArray<SubsystemTreeItemPtr>& RootItems, const EColumnSortMode::Type SortMode) const override;
};

// FSubsystemDynamicImageColumn
