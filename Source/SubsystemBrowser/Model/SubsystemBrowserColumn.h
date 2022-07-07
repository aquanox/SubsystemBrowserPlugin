// Copyright 2022, Aquanox.

#pragma once

struct ISubsystemTreeItem;

/* Represents a configurable column */
struct SUBSYSTEMBROWSER_API FSubsystemDynamicColumn
{
	/* Column identifier */
	FName Name;
	/* Column display name */
	FText Label;
	/* Column sort order */
	int32 SortOrder = 0;

	virtual ~FSubsystemDynamicColumn() = default;
	virtual FText GetColumnText(ISubsystemTreeItem& InItem) const = 0;
	virtual FSlateColor GetColumnColor(ISubsystemTreeItem& InItem, bool bSelected) const;
	virtual TSharedPtr< SWidget > GetColumnWidget(ISubsystemTreeItem& InItem) const;
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
	virtual FText GetColumnText(ISubsystemTreeItem& InItem) const override;
	virtual FSlateColor GetColumnColor(ISubsystemTreeItem& InItem, bool bSelected) const override;
};

struct FSubsystemDynamicColumn_Config : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Config();
	virtual FText GetColumnText(ISubsystemTreeItem& InItem) const override;
};

namespace SubsystemColumns
{
	static const FName ColumnID_Marker("Marker");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Module("Module");
	static const FName ColumnID_Config("Config");

	static const FName ColumnID_DynamicSlot0("Dynamic0");
	static const FName ColumnID_DynamicSlot1("Dynamic1");
	static const FName ColumnID_DynamicSlot2("Dynamic2");
	static const FName ColumnID_DynamicSlot3("Dynamic3");
	static const FName ColumnID_DynamicSlot4("Dynamic4");

	bool IsDefaultColumn(FName InName);
	bool IsDynamicColumn(FName InName);

	inline std::initializer_list<FName> GetDynamicColumns()
	{
		return {
			ColumnID_DynamicSlot0,
			ColumnID_DynamicSlot1,
			ColumnID_DynamicSlot2,
			ColumnID_DynamicSlot3,
			ColumnID_DynamicSlot4
		};
	}

	int32 GetDynamicColumnIndex(FName InName);

}
