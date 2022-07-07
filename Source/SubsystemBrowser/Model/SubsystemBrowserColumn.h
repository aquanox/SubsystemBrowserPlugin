// Copyright 2022, Aquanox.

#pragma once

struct ISubsystemTreeItem;

/* Represents a configurable column */
struct SUBSYSTEMBROWSER_API FSubsystemDynamicColumn : public TSharedFromThis<FSubsystemDynamicColumn>
{
	/* Column identifier */
	FName Name;
	/* Column display name */
	FText Label;
	/* Column sort order */
	int32 SortOrder = 0;

	virtual ~FSubsystemDynamicColumn() = default;
	virtual FText GetColumnHeaderText() const { return Label; }
	virtual float GetPreferredWidth() const { return 0.1f; }

	virtual TSharedPtr< SWidget > GenerateColumnWidget(TSharedRef<class SSubsystemTableItem> TableRow) const;
};

using SubsystemColumnPtr = TSharedPtr<FSubsystemDynamicColumn>;

struct SubsystemColumnSorter
{
	bool operator()(const SubsystemColumnPtr& A, const SubsystemColumnPtr& B) const
	{
		return A->SortOrder < B->SortOrder;
	}
};

struct FSubsystemDynamicColumn_Module : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Module();
	virtual float GetPreferredWidth() const override { return 0.25f; }
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<class SSubsystemTableItem> TableRow) const override;

private:
	FText ExtractModuleText(TSharedPtr<ISubsystemTreeItem> Item) const;
	FText ExtractModuleTooltipText(TSharedPtr<ISubsystemTreeItem> Item) const;
};

struct FSubsystemDynamicColumn_Config : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Config();
	virtual float GetPreferredWidth() const override { return 0.15f; }
	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<SSubsystemTableItem> TableRow) const override;
private:
	FText ExtractConfigText(TSharedPtr<ISubsystemTreeItem> Item) const;
};

namespace SubsystemColumns
{
	static const FName ColumnID_Marker("Marker");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Module("Module");
	static const FName ColumnID_Config("Config");
}
