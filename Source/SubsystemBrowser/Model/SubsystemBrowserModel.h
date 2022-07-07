// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserDescriptor.h"
#include "Model/SubsystemBrowserColumn.h"
#include "Misc/TextFilter.h"

DECLARE_DELEGATE(FOnSubsystemSelectedDelegate);

/* Subsystem text filter */
class SubsystemTextFilter : public TTextFilter<const ISubsystemTreeItem&>
{
public:
	SubsystemTextFilter(const FItemToStringArray& InTransformDelegate)
		: TTextFilter<const ISubsystemTreeItem&>(InTransformDelegate) { }

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

	bool IsSubsystemFilterActive() const { return SubsystemTextFilter.IsValid() && SubsystemTextFilter->HasText(); }

	const TArray<SubsystemTreeItemPtr>& GetAllCategories() const;
	void GetFilteredCategories(TArray<SubsystemTreeItemPtr>& OutCategories) const;

	const TArray<SubsystemTreeItemPtr>& GetAllSubsystems() const;
	void GetAllSubsystemsInCategory(SubsystemTreeItemPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren) const;

	void GetFilteredSubsystems(SubsystemTreeItemPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren) const;

	int32 GetNumSubsystemsFromVisibleCategories() const;

	const TArray<SubsystemColumnPtr>& GetDynamicColumns() const;

private:
	void EmptyModel();
	void PopulateCategories();
	void PopulateSubsystems();
	void PopulateColumns();

	/* Global list of all categories */
	TArray<SubsystemTreeItemPtr> AllCategories;
	/* Global list of all subsystems */
	TArray<SubsystemTreeItemPtr> AllSubsystems;
	/* Global list of all subsystems by category */
	TMap<FName, TArray<SubsystemTreeItemPtr>> AllSubsystemsByCategory;
	/* List of dynamic columns */
	TArray<SubsystemColumnPtr> DynamicColumns;

	/* Pointer to currently browsing world */
	TWeakObjectPtr<UWorld> CurrentWorld;
public:
	TSharedPtr<SubsystemCategoryFilter>  CategoryFilter;
	TSharedPtr<SubsystemTextFilter>		 SubsystemTextFilter;
};

