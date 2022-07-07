// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "SubsystemBrowserFlags.h"
#include "Model/SubsystemBrowserCategory.h" // [no-fwd]
#include "Model/SubsystemBrowserColumn.h" // [no-fwd]

class USubsystemBrowserSettings;

class FSubsystemBrowserModule : public IModuleInterface
{
	static const FName SubsystemBrowserTabName;

public:
	static FSubsystemBrowserModule& Get()
	{
		return FModuleManager::GetModuleChecked<FSubsystemBrowserModule>(TEXT("SubsystemBrowser"));
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override { return false; }

	/**
	 * Get list of all registered subsystem categories
	 */
	SUBSYSTEMBROWSER_API const TArray<SubsystemCategoryPtr>& GetCategories() const;
	/**
	 * Register default subsystem categories
	 */
	void RegisterDefaultCategories();
	/**
	 * Register a new subsystem category to show
	 */
	SUBSYSTEMBROWSER_API void RegisterCategory(TSharedRef<FSubsystemCategory> InCategory);
	/**
	 * Remove a category by its name
	 */
	SUBSYSTEMBROWSER_API void RemoveCategory(FName CategoryName);

#if ENABLE_SUBSYSTEM_BROWSER_CUSTOM_COLUMNS
	/**
	 * Get a list of all custom dynamic columns
	 */
	const TArray<SubsystemColumnPtr>& GetCustomDynamicColumns() const;
	/**
	 * Register a new custom dynamic column
	 */
	void RegisterDynamicColumn(TSharedRef<FSubsystemDynamicColumn> InColumn);

#endif
private:
	static TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args);
	static TSharedRef<SWidget> CreateSubsystemBrowser(const FSpawnTabArgs& Args);

	static void SummonSubsystemTab();

	// Saved instance of Settings section
	TSharedPtr<class ISettingsSection> SettingsSection;
	// Instances of subsystem categories
	TArray<SubsystemCategoryPtr> Categories;

#if ENABLE_SUBSYSTEM_BROWSER_CUSTOM_COLUMNS
	// Instances of dynamic subsystem columns
	TArray<SubsystemColumnPtr> CustomDynamicColumns;
#endif
};

#if UE_BUILD_DEBUG
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, All);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, Warning);
#endif
