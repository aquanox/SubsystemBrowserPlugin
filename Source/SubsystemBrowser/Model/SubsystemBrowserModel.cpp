// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserModel.h"

#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

SubsystemCategoryFilter::SubsystemCategoryFilter()
{
	// load initial state from config
	USubsystemBrowserSettings::Get()->LoadCategoryStates(FilterState);
}

bool SubsystemCategoryFilter::PassesFilter(const ISubsystemTreeItem& InItem) const
{
	return IsCategoryVisible(InItem.GetID());
}

void SubsystemCategoryFilter::ShowCategory(FSubsystemTreeItemID InCategory)
{
	FilterState.Add(InCategory, true);
	USubsystemBrowserSettings::Get()->SetCategoryState(InCategory, true);
	OnChangedInternal.Broadcast();
}

void SubsystemCategoryFilter::HideCategory(FSubsystemTreeItemID InCategory)
{
	FilterState.Add(InCategory, false);
	USubsystemBrowserSettings::Get()->SetCategoryState(InCategory, false);
	OnChangedInternal.Broadcast();
}

bool SubsystemCategoryFilter::IsCategoryVisible(FSubsystemTreeItemID InCategory) const
{
	return !FilterState.Contains(InCategory) ? true : FilterState.FindChecked(InCategory);
}

FSubsystemModel::FSubsystemModel()
{
	FSubsystemBrowserModule::AddPermanentColumns(PermanentColumns);
}

TWeakObjectPtr<UWorld> FSubsystemModel::GetCurrentWorld() const
{
	return CurrentWorld;
}

void FSubsystemModel::SetCurrentWorld(TWeakObjectPtr<UWorld> InWorld)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("World Switch %s => %s"), *GetNameSafe(CurrentWorld.Get()), *GetNameSafe(InWorld.Get()));

	CurrentWorld = InWorld;

	EmptyModel();

	PopulateCategories();
	PopulateSubsystems();
}

bool FSubsystemModel::IsSubsystemFilterActive() const
{
	if (SubsystemTextFilter.IsValid() && SubsystemTextFilter->HasText())
		return true;

	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	if (Settings->ShouldShowOnlyGame())
		return true;
	if (Settings->ShouldShowOnlyPlugins())
		return true;
	if (Settings->ShouldShowOnlyViewable())
		return true;
	if (Settings->HasIgnoredSubsystems())
		return true;
		
	return false;
}

int32 FSubsystemModel::GetNumCategories() const
{
	return AllCategories.Num();
}

const TArray<SubsystemTreeItemPtr>& FSubsystemModel::GetAllCategories() const
{
	return AllCategories;
}

void FSubsystemModel::GetFilteredCategories(TArray<SubsystemTreeItemPtr>& OutCategories)
{
	OutCategories.Empty();

	for (const SubsystemTreeItemPtr& Item : GetAllCategories())
	{
		check(Item->GetAsCategoryDescriptor());
		if (CategoryFilter.IsValid() && !CategoryFilter->PassesFilter(*Item))
		{
			continue;
		}

		if (USubsystemBrowserSettings::Get()->ShouldHideEmptyCategories()
			&& !GetNumSubsystemsFromCategory(Item))
		{
			continue;
		}

		OutCategories.Add(Item);
	}
}

const TArray<SubsystemTreeItemPtr>& FSubsystemModel::GetAllSubsystems() const
{
	return AllSubsystems;
}

void FSubsystemModel::GetAllSubsystemsInCategory(SubsystemTreeItemConstPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren)
{
	check(Category->GetAsCategoryDescriptor());

	OutChildren.Empty();
	if (AllSubsystemsByCategory.Contains(Category->GetID()))
	{
		for (const SubsystemTreeItemPtr& Item : AllSubsystemsByCategory.FindChecked(Category->GetID()))
		{
			OutChildren.Add(Item);
		}
	}
}

void FSubsystemModel::GetFilteredSubsystems(SubsystemTreeItemConstPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren)
{
	const FSubsystemTreeCategoryItem* AsCategory = Category->GetAsCategoryDescriptor();
	check(AsCategory);

	OutChildren.Empty();

	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	if (AllSubsystemsByCategory.Contains(AsCategory->GetID()))
	{
		for (const SubsystemTreeItemPtr& Item : AllSubsystemsByCategory.FindChecked(AsCategory->GetID()))
		{
			if (Settings->ShouldShowOnlyGame() && !Item->IsGameModule())
				continue;
			if (Settings->ShouldShowOnlyPlugins() && !Item->IsPluginModule())
				continue;
			if (Settings->ShouldShowOnlyViewable() && !Item->HasViewableElements())
				continue;
			
			if (const FSubsystemTreeSubsystemItem* AsSubsystem = Item->GetAsSubsystemDescriptor())
			{
				if (Settings->IsSubsystemIgnored(AsSubsystem->ScriptName))
					continue;
			}

			if (!SubsystemTextFilter.IsValid() || SubsystemTextFilter->PassesFilter(*Item))
			{
				OutChildren.Add(Item);
			}
		}
	}
}

void FSubsystemModel::GetSubsystemSubobjects(SubsystemTreeItemConstPtr Subsystem, TArray<SubsystemTreeItemPtr>& OutChildren)
{
	const FSubsystemTreeSubsystemItem* AsSubsystem = Subsystem->GetAsSubsystemDescriptor();
	check(AsSubsystem);
	const FSubsystemTreeCategoryItem* AsCategory = AsSubsystem->GetParent()->GetAsCategoryDescriptor();
	check(AsCategory);
	
	OutChildren.Empty();
	
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	if (!Settings->ShouldShowSubobjbects())
		return;

	TArray<UObject*> Result;
	FSubsystemBrowserUtils::DefaultSelectSubsystemSubobjects(AsSubsystem->GetObjectForDetails(), Result);

	for (UObject* Object : Result)
	{
		auto Item = MakeShared<FSubsystemTreeObjectItem>(SharedThis(this), ConstCastSharedPtr<ISubsystemTreeItem>(Subsystem), Object);

		OutChildren.Emplace(MoveTemp(Item));
	}
}

int32 FSubsystemModel::GetNumSubsystemsFromCategory(SubsystemTreeItemConstPtr Category)
{
	TArray<SubsystemTreeItemPtr> Subsystems;
	GetFilteredSubsystems(Category, Subsystems);
	return Subsystems.Num();
}

int32 FSubsystemModel::GetNumSubsystemsFromVisibleCategories()
{
	int32 Count = 0;

	TArray<SubsystemTreeItemPtr> VisibleCategories;
	GetFilteredCategories(VisibleCategories);

	TArray<SubsystemTreeItemPtr> Subsystems;

	for (const SubsystemTreeItemPtr& Category : VisibleCategories)
	{
		GetAllSubsystemsInCategory(Category, Subsystems);

		Count += Subsystems.Num();
	}

	return Count;
}

int32 FSubsystemModel::GetNumDynamicColumns() const
{
	return FSubsystemBrowserModule::Get().GetDynamicColumns().Num();
}

bool FSubsystemModel::ShouldShowColumn(SubsystemColumnPtr Column) const
{
	if (PermanentColumns.Contains(Column))
		return true;

	return USubsystemBrowserSettings::Get()->GetTableColumnState(Column->Name);
}

bool FSubsystemModel::IsItemSelected(TSharedRef<const ISubsystemTreeItem> Item)
{
	return LastSelectedItem == Item;
}

void FSubsystemModel::NotifySelected(TSharedPtr<ISubsystemTreeItem> Item)
{
	if (Item.IsValid())
	{
		LastSelectedItem = Item;
	}

	OnSelectionChanged.Broadcast(Item);
}

TArray<SubsystemColumnPtr> FSubsystemModel::GetSelectedTableColumns() const
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	TArray<SubsystemColumnPtr> Result;
	Result.Append(PermanentColumns);

	for (const SubsystemColumnPtr& Column : FSubsystemBrowserModule::Get().GetDynamicColumns())
	{
		if (Settings->GetTableColumnState(Column->Name))
		{
			Result.Add(Column);
		}
	}

	Result.StableSort(SubsystemColumnSorter());
	return Result;
}

TArray<SubsystemColumnPtr> FSubsystemModel::GetDynamicTableColumns() const
{
	TArray<SubsystemColumnPtr> Result;
	Result.Append(FSubsystemBrowserModule::Get().GetDynamicColumns());
	Result.StableSort(SubsystemColumnSorter());
	return Result;
}

SubsystemColumnPtr FSubsystemModel::FindTableColumn(const FName& ColumnName) const
{
	TArray<SubsystemColumnPtr, TInlineAllocator<8>> Result;
	Result.Append(PermanentColumns);
	Result.Append(FSubsystemBrowserModule::Get().GetDynamicColumns());

	for (const SubsystemColumnPtr& Column : Result)
	{
		if (Column->Name == ColumnName)
		{
			return Column;
		}
	}
	return nullptr;
}

void FSubsystemModel::EmptyModel()
{
	for (const SubsystemTreeItemPtr& Category : AllCategories)
	{
		Category->RemoveAllChildren();
	}
	AllCategories.Empty();

	AllSubsystems.Empty();
	AllSubsystemsByCategory.Empty();

	LastSelectedItem.Reset();
}

void FSubsystemModel::PopulateCategories()
{
	FSubsystemBrowserModule& BrowserModule = FSubsystemBrowserModule::Get();
	for (auto& SubsystemCategory : BrowserModule.GetCategories())
	{
		auto Category = MakeShared<FSubsystemTreeCategoryItem>(SharedThis(this), SubsystemCategory.ToSharedRef());

		AllCategories.Add(MoveTemp(Category));
	}

	// sort categories after populating them
	AllCategories.StableSort(SubsystemCategorySorter());
}

void FSubsystemModel::PopulateSubsystems()
{
	check(!AllSubsystems.Num());
	check(!AllSubsystemsByCategory.Num());

	UWorld* const LocalWorld = CurrentWorld.Get();

	for (const auto & Category : AllCategories)
	{
		const FSubsystemTreeCategoryItem* AsCategory = Category->GetAsCategoryDescriptor();

		TArray<UObject*> Result;
		AsCategory->Data->Select(LocalWorld, Result);
		for (UObject* Impl : Result)
		{
			auto Descriptor = MakeShared<FSubsystemTreeSubsystemItem>(SharedThis(this), Category, Impl);

			AllSubsystems.Add(Descriptor);
			AllSubsystemsByCategory.FindOrAdd(AsCategory->GetID()).Add(Descriptor);
		}
	}
}

#undef LOCTEXT_NAMESPACE
