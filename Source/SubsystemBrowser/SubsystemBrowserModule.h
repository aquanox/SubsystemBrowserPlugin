// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Model/SubsystemBrowserCategory.h" // [no-fwd]
#include "Model/SubsystemBrowserColumn.h" // [no-fwd]
#include "SubsystemSettingsManager.h"

class FSpawnTabArgs;
class UToolMenu;
class SDockTab;
struct ISubsystemTreeItem;

class FSubsystemBrowserModule : public IModuleInterface
{
public:
	static const FName SubsystemBrowserTabName;
	static const FName SubsystemBrowserContextMenuName;

	SUBSYSTEMBROWSER_API static FSubsystemBrowserModule& Get()
	{
		return FModuleManager::GetModuleChecked<FSubsystemBrowserModule>(TEXT("SubsystemBrowser"));
	}

	FSubsystemSettingsManager& GetSettingsManager()
	{
		return SettingsManager;
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
	 * Open subsystems tab
	 */
	void SummonSubsystemTab();

	/**
	 * Callback that is called whenever a tooltip for item needs to be generated
	 * Used to add custom data to tooltips.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGenerateTooltip, TSharedRef<const ISubsystemTreeItem>, class FSubsystemTableItemTooltipBuilder&);
	static SUBSYSTEMBROWSER_API FOnGenerateTooltip OnGenerateTooltip;

	/**
	 * Callback that is called whenever a menu for item needs to be generated.
	 * Used to add custom menu actions.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGenerateMenu, TSharedRef<const ISubsystemTreeItem>, UToolMenu*);
	static SUBSYSTEMBROWSER_API FOnGenerateMenu OnGenerateContextMenu;

protected:
	/** */
	void RegisterMenus();

	/** Handles creating the subsystem browser tab. */
	TSharedRef<SDockTab> HandleSpawnBrowserTab(const FSpawnTabArgs& Args);

private:
	// Settings manager
	FSubsystemSettingsManager SettingsManager;

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

#if UE_BUILD_DEBUG || defined(WITH_SB_HOST_PROJECT)
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, All);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogSubsystemBrowser, Log, Warning);
#endif
