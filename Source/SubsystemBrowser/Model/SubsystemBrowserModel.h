// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemDescriptor.h"
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

	struct FSubsystemColumn
	{
		FName Id;
		FText Label;
	};

	const TArray<struct FSubsystemColumn>& GetOptionalColumns() const;

private:
	void EmptyModel();
	void PopulateCategories();
	void PopulateSubsystems();

	TArray<UObject*> SelectEngineSubsystems() const;
	TArray<UObject*> SelectEditorSubsystems() const;
	TArray<UObject*> SelectGameInstanceSubsystems() const;
	TArray<UObject*> SelectWorldSubsystems() const;
	TArray<UObject*> SelectPlayerSubsystems() const;

	/*  */
	TArray<FSubsystemColumn> OptionalColumns;

	/* Global list of all categories */
	TArray<SubsystemTreeItemPtr> AllCategories;
	/* Global list of all subsystems */
	TArray<SubsystemTreeItemPtr> AllSubsystems;
	/* Global list of all subsystems by category */
	TMap<FName, TArray<SubsystemTreeItemPtr>> AllSubsystemsByCategory;

	TWeakObjectPtr<UWorld> CurrentWorld;
public:
	TSharedPtr<SubsystemCategoryFilter>  CategoryFilter;
	TSharedPtr<SubsystemTextFilter>		 SubsystemTextFilter;
};

namespace SubsystemColumns
{
	static const FName ColumnID_Marker("Marker");
	static const FName ColumnID_Name("Name");
	static const FName ColumnID_Package("Package");
	static const FName ColumnID_ConfigClass("Config");
}

namespace SubsystemCategories
{
	static const FName CategoryEngine("EngineSubsystemCategory");
	static const FName CategoryEditor("EditorSubsystemCategory");
	static const FName CategoryGameInstance("GameInstanceCategory");
	static const FName CategoryWorld("WorldSubsystemCategory");
	static const FName CategoryPlayer("PlayerCategory");

}
