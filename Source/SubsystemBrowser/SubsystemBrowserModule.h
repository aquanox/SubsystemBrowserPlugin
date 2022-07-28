// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
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
	 * Register a new subsystem category
	 */
	template<typename TCategory, typename... TArgs>
	void RegisterCategory(TArgs&&... InArgs);
	/**
	 * Register a new subsystem category to show
	 */
	SUBSYSTEMBROWSER_API void RegisterCategory(TSharedRef<FSubsystemCategory> InCategory);
	/**
	 * Remove a category by its name
	 */
	SUBSYSTEMBROWSER_API void RemoveCategory(FName CategoryName);

	/**
	 * Get a list of all custom dynamic columns
	 */
	const TArray<SubsystemColumnPtr>& GetDynamicColumns() const;
	/**
	 *
	 */
	void RegisterDefaultDynamicColumns();
	/**
	 * Register a new custom dynamic column
	 */
	template<typename TColumn, typename... TArgs>
	void RegisterDynamicColumn(TArgs&&... InArgs);
	/**
	 * Register a new custom dynamic column
	 */
	SUBSYSTEMBROWSER_API void RegisterDynamicColumn(TSharedRef<FSubsystemDynamicColumn> InColumn);
	/**
	 * Populate permanent columns
	 */
	static void AddPermanentColumns(TArray<SubsystemColumnPtr>& Columns);

	/**
	 * Open editor settings tab with plugin settings pre-selected
	 */
	void SummonPluginSettingsTab();

	/**
	 * Open subsystems tab
	 */
	void SummonSubsystemTab();

private:
	static TSharedRef<class SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args);
	static TSharedRef<class SWidget> CreateSubsystemBrowser(const FSpawnTabArgs& Args);

	// Saved instance of Settings section
	TSharedPtr<class ISettingsSection> SettingsSection;
	// Instances of subsystem categories
	TArray<SubsystemCategoryPtr> Categories;
	// Instances of dynamic subsystem columns
	TArray<SubsystemColumnPtr> DynamicColumns;
};

template <typename TCategory, typename... TArgs>
void FSubsystemBrowserModule::RegisterCategory(TArgs&&... InArgs)
{
	RegisterCategory(MakeShared<TCategory>(Forward<TArgs>(InArgs)...));
}

template <typename TColumn, typename... TArgs>
void FSubsystemBrowserModule::RegisterDynamicColumn(TArgs&&... InArgs)
{
	RegisterDynamicColumn(MakeShared<TColumn>(Forward<TArgs>(InArgs)...));
}

#if UE_BUILD_DEBUG
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, All);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, Warning);
#endif
