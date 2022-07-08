// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserCategory.h"

#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserFlags.h"
#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "EditorSubsystem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const TArray<SubsystemCategoryPtr>& FSubsystemBrowserModule::GetCategories() const
{
	return Categories;
}

void FSubsystemBrowserModule::RegisterDefaultCategories()
{
	auto CategoryEngine = MakeShared<FSubsystemCategory>();
	CategoryEngine->Name		   = SubsystemCategoryHelpers::CategoryEngine;
	CategoryEngine->Label		   = LOCTEXT("SubsystemBrowser_Engine", "Engine Subsystems");
	CategoryEngine->SubsystemClass = UEngineSubsystem::StaticClass();
	CategoryEngine->Selector	   = FEnumSubsystemsDelegate::CreateStatic(&SubsystemCategoryHelpers::SelectEngineSubsystems);
	CategoryEngine->SortOrder = 100;
	RegisterCategory(CategoryEngine);

	auto CategoryEditor = MakeShared<FSubsystemCategory>();
	CategoryEditor->Name = SubsystemCategoryHelpers::CategoryEditor;
	CategoryEditor->Label = LOCTEXT("SubsystemBrowser_Editor", "Editor Subsystems");
	CategoryEditor->SubsystemClass = UEditorSubsystem::StaticClass();
	CategoryEditor->Selector = FEnumSubsystemsDelegate::CreateStatic(&SubsystemCategoryHelpers::SelectEditorSubsystems);
	CategoryEditor->SortOrder = 200;
	RegisterCategory(CategoryEditor);

	auto CategoryGameInstance = MakeShared<FSubsystemCategory>();
	CategoryGameInstance->Name = SubsystemCategoryHelpers::CategoryGameInstance;
	CategoryGameInstance->Label = LOCTEXT("SubsystemBrowser_GameInstance", "Game Instance Subsystems");
	CategoryGameInstance->SubsystemClass = UGameInstanceSubsystem::StaticClass();
	CategoryGameInstance->Selector = FEnumSubsystemsDelegate::CreateStatic(&SubsystemCategoryHelpers::SelectGameInstanceSubsystems);
	CategoryGameInstance->SortOrder = 300;
	RegisterCategory(CategoryGameInstance);

	auto CategoryWorld = MakeShared<FSubsystemCategory>();
	CategoryWorld->Name = SubsystemCategoryHelpers::CategoryWorld;
	CategoryWorld->Label = LOCTEXT("SubsystemBrowser_World", "World Subsystems");
	CategoryWorld->SubsystemClass = UWorldSubsystem::StaticClass();
	CategoryWorld->Selector = FEnumSubsystemsDelegate::CreateStatic(&SubsystemCategoryHelpers::SelectWorldSubsystems);
	CategoryWorld->SortOrder = 400;
	RegisterCategory(CategoryWorld);

	auto CategoryPlayer = MakeShared<FSubsystemCategory>();
	CategoryPlayer->Name = SubsystemCategoryHelpers::CategoryPlayer;
	CategoryPlayer->Label = LOCTEXT("SubsystemBrowser_Player", "Player Subsystems");
	CategoryPlayer->SubsystemClass = ULocalPlayerSubsystem::StaticClass();
	CategoryPlayer->Selector = FEnumSubsystemsDelegate::CreateStatic(&SubsystemCategoryHelpers::SelectPlayerSubsystems);
	CategoryPlayer->SortOrder = 500;
	RegisterCategory(CategoryPlayer);
}

void FSubsystemBrowserModule::RegisterCategory(TSharedRef<FSubsystemCategory> InCategory)
{
	if (InCategory->Name.IsNone()
		|| !InCategory->SubsystemClass.IsValid()
		|| !InCategory->Selector.IsBound())
	{
		UE_LOG(LogSubsystemBrowser, Error, TEXT("Invalid category being registered"));
		return;
	}

	for (auto& Category : Categories)
	{
		if (Category->Name == InCategory->Name)
		{
			UE_LOG(LogSubsystemBrowser, Error, TEXT("Duplicating category with name %s."), *Category->Name.ToString());
			return;
		}
	}

	Categories.Add(InCategory);
}

void FSubsystemBrowserModule::RemoveCategory(FName CategoryName)
{
	for (auto It = Categories.CreateIterator(); It; ++It)
	{
		if ((*It)->Name == CategoryName)
		{
			It.RemoveCurrent();
		}
	}
}

void SubsystemCategoryHelpers::SelectEngineSubsystems(UWorld* CurrentWorld, TArray<UObject*>& OutData)
{
	OutData.Append(GEngine->GetEngineSubsystemArray<UEngineSubsystem>());
}

void SubsystemCategoryHelpers::SelectEditorSubsystems(UWorld* CurrentWorld, TArray<UObject*>& OutData)
{
	OutData.Append(GEditor->GetEditorSubsystemArray<UEditorSubsystem>());
}

void SubsystemCategoryHelpers::SelectGameInstanceSubsystems(UWorld* CurrentWorld, TArray<UObject*>& OutData)
{
	if (IsValid(CurrentWorld) && CurrentWorld->GetGameInstance())
	{
		OutData.Append(CurrentWorld->GetGameInstance()->GetSubsystemArray<UGameInstanceSubsystem>());
	}
}

void SubsystemCategoryHelpers::SelectWorldSubsystems(UWorld* CurrentWorld, TArray<UObject*>& OutData)
{
	if (IsValid(CurrentWorld) && CurrentWorld->GetGameInstance())
	{
		OutData.Append(CurrentWorld->GetSubsystemArray<UWorldSubsystem>());
	}
}

void SubsystemCategoryHelpers::SelectPlayerSubsystems(UWorld* CurrentWorld, TArray<UObject*>& OutData)
{
	if (IsValid(CurrentWorld) && CurrentWorld->GetGameInstance())
	{
		for (ULocalPlayer* const LocalPlayer : CurrentWorld->GetGameInstance()->GetLocalPlayers())
		{
			OutData.Append(LocalPlayer->GetSubsystemArray<ULocalPlayerSubsystem>());
		}
	}
}

#undef LOCTEXT_NAMESPACE

#if ENABLE_SUBSYSTEM_BROWSER_EXAMPLES

// 1. Define a function to select subsystems
void SelectCustomSubsystems(UWorld* InContext, TArray<UObject*>& OutData)
{
	// Your code here to select subsystems and add them to OutData
}

// 2. Call this in your editor modules StartupModule to register a new category
void RegisterCategoryExample()
{
	// Get a reference to Subsystem Browser module instance or load it
	FSubsystemBrowserModule& Module = FModuleManager::LoadModuleChecked<FSubsystemBrowserModule>(TEXT("SubsystemBrowser"));
	// Construct category
	auto SampleCategory = MakeShared<FSubsystemCategory>();
	SampleCategory->Name = TEXT("Sample");
	SampleCategory->Label = INVTEXT("Sample Subsystems");
	SampleCategory->SubsystemClass = USubsystem::StaticClass();
	SampleCategory->Selector = FEnumSubsystemsDelegate::CreateStatic(&SelectCustomSubsystems);
	SampleCategory->SortOrder = 50;
	// Register category in module
	Module.RegisterCategory(SampleCategory);
}

#endif