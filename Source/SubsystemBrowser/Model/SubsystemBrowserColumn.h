// Copyright 2022, Aquanox.

#pragma once

struct ISubsystemTreeItem;
class SSubsystemTableItem;

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

	/**
	 * Generate visual representation of column in header row
	 */
	virtual SHeaderRow::FColumn::FArguments GenerateHeaderColumnWidget() const;

	/**
	 * Generate visual representation of column in table row
	 */
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const = 0;

	/**
	 * Gather searchable strings for column
	 */
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const {}

	/**
	 * Does this column support sorting? WIP
	 */
	//virtual bool SupportsSorting() const { return false; }

	/**
	 * Perform sorting of table items. WIP
	 */
	//virtual void SortItems(TArray<ISubsystemTreeItem>& RootItems, const EColumnSortMode::Type SortMode) const {}
};

using SubsystemColumnPtr = TSharedPtr<FSubsystemDynamicColumn>;

struct SubsystemColumnSorter
{
	bool operator()(const SubsystemColumnPtr& A, const SubsystemColumnPtr& B) const
	{
		return A->SortOrder < B->SortOrder;
	}
};

/*
 * "Module" column implementation
 */
struct FSubsystemDynamicColumn_Module : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Module();
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;

private:
	FText ExtractModuleText(TSharedRef<SSubsystemTableItem> TableRow) const;
	FText ExtractModuleTooltipText(TSharedRef<SSubsystemTableItem> TableRow) const;
};

/**
 * "Config" column implementation
 */
struct FSubsystemDynamicColumn_Config : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Config();
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const override;
	virtual void PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const override;
private:
	FText ExtractConfigText(TSharedRef<SSubsystemTableItem> TableRow) const;
};

namespace SubsystemColumns
{
	static const FName ColumnID_Marker("Marker");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Module("Module");
	static const FName ColumnID_Config("Config");
}
