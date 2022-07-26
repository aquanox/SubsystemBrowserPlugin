// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserDescriptor.h"
#include "Model/SubsystemBrowserColumn.h"
#include "Misc/TextFilter.h"

/* Subsystem text filter */
class SubsystemTextFilter : public TTextFilter<const ISubsystemTreeItem&>
{
	using Super = TTextFilter<const ISubsystemTreeItem&>;
public:
	SubsystemTextFilter(const FItemToStringArray& InTransformDelegate) : Super(InTransformDelegate) { }

	bool HasText() const { return !GetRawFilterText().IsEmpty(); }
};

/* Subsystem category filter */
class SubsystemCategoryFilter : public IFilter<const ISubsystemTreeItem&>
{
public:
	SubsystemCategoryFilter();

	virtual FChangedEvent& OnChanged() override { return OnChangedInternal; }
	virtual bool PassesFilter(const ISubsystemTreeItem& InItem) const override;
	void ShowCategory(FSubsystemTreeItemID InCategory);
	void HideCategory(FSubsystemTreeItemID InCategory);
	bool IsCategoryVisible(FSubsystemTreeItemID InCategory) const;
private:
	TMap<FSubsystemTreeItemID, bool>	FilterState;
	FChangedEvent						OnChangedInternal;
};

struct SubsystemCategorySorter
{
	bool operator()(const SubsystemTreeItemPtr& A, const SubsystemTreeItemPtr& B) const
	{
		return A->GetSortOrder() < B->GetSortOrder();
	}
};

/* Subsystem list data model */
class FSubsystemModel : public TSharedFromThis<FSubsystemModel>
{
public:
	FSubsystemModel();

	TWeakObjectPtr<UWorld> GetCurrentWorld() const;
	void SetCurrentWorld(TWeakObjectPtr<UWorld> InWorld);

	bool IsSubsystemFilterActive() const;

	int32 GetNumCategories() const;
	const TArray<SubsystemTreeItemPtr>& GetAllCategories() const;
	void GetFilteredCategories(TArray<SubsystemTreeItemPtr>& OutCategories) const;

	const TArray<SubsystemTreeItemPtr>& GetAllSubsystems() const;
	void GetAllSubsystemsInCategory(SubsystemTreeItemPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren) const;

	void GetFilteredSubsystems(SubsystemTreeItemPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren) const;

	/* get total number of subsystems in visible categories */
	int32 GetNumSubsystemsFromVisibleCategories() const;

	/* find a permanent or dynamic column by its name */
	SubsystemColumnPtr FindTableColumn(const FName& ColumnName) const;
	/* returns all visible permanent and dynamic columns in sorted order */
	TArray<SubsystemColumnPtr> GetSelectedTableColumns() const;

	/* returns all dynamic columns in sorted order */
	TArray<SubsystemColumnPtr> GetDynamicTableColumns() const;
	/* return a total number of dynamic columns registered */
	int32 GetNumDynamicColumns() const;
	/* check if dynamic column is enabled by settings */
	bool ShouldShowColumn(SubsystemColumnPtr Column) const;

	bool IsItemSelected(TSharedRef<ISubsystemTreeItem> Item);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemSelectionChange, TSharedPtr<ISubsystemTreeItem> /* Item */);
	/* delegate that is triggered when tree selection changed */
	FOnItemSelectionChange OnSelectionChanged;

	void NotifySelected(TSharedPtr<ISubsystemTreeItem> Item);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemDataChanged, TSharedRef<ISubsystemTreeItem> /* Item */);
	/* delegate that is triggered when one of subsystems in this model is changed and needs possible update */
	FOnItemDataChanged OnDataChanged;

private:
	void EmptyModel();
	void PopulateCategories();
	void PopulateSubsystems();

	/* Global list of all categories */
	TArray<SubsystemTreeItemPtr> AllCategories;
	/* Global list of all subsystems */
	TArray<SubsystemTreeItemPtr> AllSubsystems;
	/* Global list of all subsystems by category */
	TMap<FName, TArray<SubsystemTreeItemPtr>> AllSubsystemsByCategory;
	/* List of permanent columns */
	TArray<SubsystemColumnPtr> PermanentColumns;

	/* Pointer to currently browsing world */
	TWeakObjectPtr<UWorld> CurrentWorld;
	/* Weak pointer to last selected item */
	TWeakPtr<ISubsystemTreeItem> LastSelectedItem;
public:
	TSharedPtr<SubsystemCategoryFilter>  CategoryFilter;
	TSharedPtr<SubsystemTextFilter>		 SubsystemTextFilter;
};
