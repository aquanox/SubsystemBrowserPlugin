// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserModel.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "EditorSubsystem.h"
#include "Engine/LocalPlayer.h"

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
}

TWeakObjectPtr<UWorld> FSubsystemModel::GetCurrentWorld() const
{
	return CurrentWorld;
}

void FSubsystemModel::SetCurrentWorld(TWeakObjectPtr<UWorld> InWorld)
{
	CurrentWorld = InWorld;

	EmptyModel();

	PopulateColumns();
	PopulateCategories();
	PopulateSubsystems();
}

const TArray<SubsystemTreeItemPtr>& FSubsystemModel::GetAllCategories() const
{
	return AllCategories;
}

void FSubsystemModel::GetFilteredCategories(TArray<SubsystemTreeItemPtr>& OutCategories) const
{
	OutCategories.Empty();

	for (const SubsystemTreeItemPtr& Item : GetAllCategories())
	{
		check(Item->GetAsCategoryDescriptor());
		if (!CategoryFilter.IsValid() || CategoryFilter->PassesFilter(*Item))
		{
			OutCategories.Add(Item);
		}
	}
}

const TArray<SubsystemTreeItemPtr>& FSubsystemModel::GetAllSubsystems() const
{
	return AllSubsystems;
}

void FSubsystemModel::GetAllSubsystemsInCategory(SubsystemTreeItemPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren) const
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

void FSubsystemModel::GetFilteredSubsystems(SubsystemTreeItemPtr Category, TArray<SubsystemTreeItemPtr>& OutChildren) const
{
	FSubsystemTreeCategoryItem* AsCategory = Category->GetAsCategoryDescriptor();
	check(AsCategory);

	OutChildren.Empty();

	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	if (AllSubsystemsByCategory.Contains(AsCategory->GetID()))
	{
		for (const SubsystemTreeItemPtr& Item : AllSubsystemsByCategory.FindChecked(AsCategory->GetID()))
		{
			if (Settings->ShouldShowOnlyGame() && !Item->IsGameModule())
				continue;

			if (!SubsystemTextFilter.IsValid() || SubsystemTextFilter->PassesFilter(*Item))
			{
				OutChildren.Add(Item);
			}
		}
	}

	if (OutChildren.Num() > 1)
	{
		OutChildren.Sort([](SubsystemTreeItemPtr Item1, SubsystemTreeItemPtr Item2) {
			return Item1->GetDisplayNameString() < Item2->GetDisplayNameString();
		});
	}
}

int32 FSubsystemModel::GetNumSubsystemsFromVisibleCategories() const
{
	int32 Count = 0;

	TArray<SubsystemTreeItemPtr> VisibleCategories;
	GetFilteredCategories(VisibleCategories);

	TArray<SubsystemTreeItemPtr> Subsystems;

	for (const auto & Category : VisibleCategories)
	{
		GetAllSubsystemsInCategory(Category, Subsystems);

		Count += Subsystems.Num();
	}

	return Count;
}

void FSubsystemModel::PopulateColumns()
{
	RegisterColumn(
		SubsystemColumns::ColumnID_Package,
		LOCTEXT("SubsystemBrowser_Column_Package", "Module")
	);

	RegisterColumn(
		SubsystemColumns::ColumnID_ConfigClass,
		LOCTEXT("SubsystemBrowser_Column_ConfigClass", "Config")
	);
}

const TArray<TSharedRef<FSubsystemColumn>>& FSubsystemModel::GetOptionalColumns() const
{
	return OptionalColumns;
}

void FSubsystemModel::EmptyModel()
{
	for (const auto & Category : AllCategories)
	{
		Category->RemoveAllChildren();
	}
	AllCategories.Empty();

	AllSubsystems.Empty();
	AllSubsystemsByCategory.Empty();

	OptionalColumns.Empty();
}

void FSubsystemModel::PopulateCategories()
{
	RegisterCategory<UEngineSubsystem>(
		SubsystemCategories::CategoryEngine,
		LOCTEXT("SubsystemBrowser_Engine", "Engine Subsystems"),
		FEnumSubsystemsDelegate::CreateSP(this, &FSubsystemModel::SelectEngineSubsystems)
	);

	RegisterCategory<UEditorSubsystem>(
		SubsystemCategories::CategoryEditor,
		LOCTEXT("SubsystemBrowser_Editor", "Editor Subsystems"),
		FEnumSubsystemsDelegate::CreateSP(this, &FSubsystemModel::SelectEditorSubsystems)
	);

	RegisterCategory<UGameInstanceSubsystem>(
		SubsystemCategories::CategoryGameInstance,
		LOCTEXT("SubsystemBrowser_GameInstance", "Game Instance Subsystems"),
		FEnumSubsystemsDelegate::CreateSP(this, &FSubsystemModel::SelectGameInstanceSubsystems)
	);

	RegisterCategory<UWorldSubsystem>(
		SubsystemCategories::CategoryWorld,
		LOCTEXT("SubsystemBrowser_World", "World Subsystems"),
		FEnumSubsystemsDelegate::CreateSP(this, &FSubsystemModel::SelectWorldSubsystems)
	);

	RegisterCategory<ULocalPlayerSubsystem>(
		SubsystemCategories::CategoryPlayer,
		LOCTEXT("SubsystemBrowser_Player", "Player Subsystems"),
		FEnumSubsystemsDelegate::CreateSP(this, &FSubsystemModel::SelectPlayerSubsystems)
	);
}

void FSubsystemModel::PopulateSubsystems()
{
	check(!AllSubsystems.Num());
	check(!AllSubsystemsByCategory.Num());

	for (const auto & Category : AllCategories)
	{
		const FSubsystemTreeCategoryItem* AsCategory = Category->GetAsCategoryDescriptor();

		for (UObject* Impl : AsCategory->Selector.Execute())
		{
			auto Descriptor = MakeShared<FSubsystemTreeSubsystemItem>(Impl);
			Descriptor->Model = AsShared();
			Descriptor->Parent = Category;
			AllSubsystems.Add(Descriptor);
			AllSubsystemsByCategory.FindOrAdd(AsCategory->GetID()).Add(Descriptor);
		}
	}
}

TArray<UObject*> FSubsystemModel::SelectEngineSubsystems() const
{
	TArray<UObject*> Result;
	Result.Append(GEngine->GetEngineSubsystemArray<UEngineSubsystem>());
	return Result;
}

TArray<UObject*> FSubsystemModel::SelectEditorSubsystems() const
{
	TArray<UObject*> Result;
	Result.Append(GEditor->GetEditorSubsystemArray<UEditorSubsystem>());
	return Result;
}

TArray<UObject*> FSubsystemModel::SelectGameInstanceSubsystems() const
{
	TArray<UObject*> Result;
	if (CurrentWorld.IsValid() && CurrentWorld->GetGameInstance())
	{
		Result.Append(CurrentWorld->GetGameInstance()->GetSubsystemArray<UGameInstanceSubsystem>());
	}
	return Result;
}

TArray<UObject*> FSubsystemModel::SelectWorldSubsystems() const
{
	TArray<UObject*> Result;
	if (CurrentWorld.IsValid())
	{
		Result.Append(CurrentWorld->GetSubsystemArray<UWorldSubsystem>());
	}
	return Result;
}

TArray<UObject*> FSubsystemModel::SelectPlayerSubsystems() const
{
	TArray<UObject*> Result;
	if (CurrentWorld.IsValid() && CurrentWorld->GetGameInstance())
	{
		for (ULocalPlayer* const LocalPlayer : CurrentWorld->GetGameInstance()->GetLocalPlayers())
		{
			Result.Append(LocalPlayer->GetSubsystemArray<ULocalPlayerSubsystem>());
		}
	}
	return Result;
}

#undef LOCTEXT_NAMESPACE
