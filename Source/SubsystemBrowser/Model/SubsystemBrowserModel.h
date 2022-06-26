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

/* Represents a configurable column */
struct FSubsystemColumn
{
public:
	TSharedPtr<class FSubsystemModel> Model;
	FName Id;
	FText Label;

	//virtual ~FSubsystemColumn() = default;
	//virtual FText GetColumnText(SubsystemTreeItemPtr InItem) const { return FText::GetEmpty(); }
	//virtual FColor GetColumnColor(SubsystemTreeItemPtr InItem) const { return FColor::Black; }
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

	const TArray<TSharedRef<FSubsystemColumn>>& GetOptionalColumns() const;

private:
	void EmptyModel();
	void PopulateCategories();

	template<typename T>
	void RegisterCategory(FName Id, FText Label, FEnumSubsystemsDelegate InDelegate)
	{
		auto Category = MakeShared<FSubsystemTreeCategoryItem>(Id, Label, T::StaticClass(), InDelegate);
		Category->Model = AsShared();
		AllCategories.Emplace(MoveTemp(Category));
	}

	void PopulateSubsystems();

	TArray<UObject*> SelectEngineSubsystems() const;
	TArray<UObject*> SelectEditorSubsystems() const;
	TArray<UObject*> SelectGameInstanceSubsystems() const;
	TArray<UObject*> SelectWorldSubsystems() const;
	TArray<UObject*> SelectPlayerSubsystems() const;

	void PopulateColumns();

	template<typename T = FSubsystemColumn>
	void RegisterColumn(FName Id, FText Label)
	{
		auto Column = MakeShared<T>();
		Column->Id = Id;
		Column->Label = Label;
		Column->Model = AsShared();
		OptionalColumns.Emplace(MoveTemp(Column));
	}

	/*  */
	TArray<TSharedRef<FSubsystemColumn>> OptionalColumns;

	/* Global list of all categories */
	TArray<SubsystemTreeItemPtr> AllCategories;
	/* Global list of all subsystems */
	TArray<SubsystemTreeItemPtr> AllSubsystems;
	/* Global list of all subsystems by category */
	TMap<FName, TArray<SubsystemTreeItemPtr>> AllSubsystemsByCategory;

	/* Pointer to currently browsing world */
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
