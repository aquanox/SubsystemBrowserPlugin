// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserCategory.h"

#include "SubsystemBrowserFlags.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemCategory::FSubsystemCategory(const FName& Name, const FText& Label, int32 SortOrder)
	: Name(Name), Label(Label), SortOrder(SortOrder)
{
}

FSimpleSubsystemCategory::FSimpleSubsystemCategory(const FName& Name, const FText& Label, const FEnumSubsystemsDelegate& Selector, int32 SortOrder)
	: FSubsystemCategory(Name, Label, SortOrder), Selector(Selector)
{
	ensure(Selector.IsBound());
}

void FSimpleSubsystemCategory::Select(UWorld* InContext, TArray<UObject*>& OutData) const
{
	Selector.Execute(InContext, OutData);
}

#undef LOCTEXT_NAMESPACE

#if ENABLE_SUBSYSTEM_BROWSER_EXAMPLES

// 2. Call this in your editor modules StartupModule to register a new category
void RegisterCategoryExample()
{
	// Get a reference to Subsystem Browser module instance or load it
	FSubsystemBrowserModule& Module = FModuleManager::LoadModuleChecked<FSubsystemBrowserModule>(TEXT("SubsystemBrowser"));
	// Construct category
	auto SampleCategory = MakeShared<FSimpleSubsystemCategory>();
	SampleCategory->Name = TEXT("Sample");
	SampleCategory->Label = INVTEXT("Sample Subsystems");
	SampleCategory->SortOrder = 50;
	SampleCategory->Selector = FEnumSubsystemsDelegate::CreateLambda([](UWorld* InContext, TArray<UObject*>& OutData)
	{
		// Fill Data
	});
	// Register category in module
	Module.RegisterCategory(SampleCategory);
}

#endif

