// Copyright 2022, Aquanox.

#pragma once

#include "CoreFwd.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Model/SubsystemBrowserCategory.h" // [no-fwd]
#include "Model/SubsystemBrowserColumn.h" // [no-fwd]

class FSpawnTabArgs;
class UToolMenu;
class SDockTab;
class IDetailsView;
class ISettingsSection;
struct ISubsystemTreeItem;

class SUBSYSTEMBROWSER_API FSubsystemBrowserModule : public IModuleInterface
{
public:
	static const FName SubsystemBrowserTabName;
	static const FName SubsystemBrowserNomadTabName;
	static const FName SubsystemBrowserContextMenuName;

	static FSubsystemBrowserModule& Get()
	{
		return FModuleManager::GetModuleChecked<FSubsystemBrowserModule>(TEXT("SubsystemBrowser"));
	}
protected:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override { return false; }
public:
	/**
	 * Get list of all registered subsystem categories
	 */
	const TArray<SubsystemCategoryPtr>& GetCategories() const;
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
	void RegisterCategory(TSharedRef<FSubsystemCategory> InCategory);
	/**
	 * Remove a category by its name
	 */
	void RemoveCategory(FName CategoryName);

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
	void RegisterDynamicColumn(TSharedRef<FSubsystemDynamicColumn> InColumn);
	/**
	 * Populate permanent columns
	 */
	static void AddPermanentColumns(TArray<SubsystemColumnPtr>& Columns);

	/**
	 * Open subsystems tab
	 */
	void SummonSubsystemTab();

	/**
	 * Open editor settings tab with plugin settings pre-selected
	 */
	void SummonPluginSettingsTab();

	/**
	 * Open subsystem settings panel
	 */
	void SummonSubsystemSettingsTab();

	/**
	 * Callback that is called whenever a tooltip for item needs to be generated
	 * Used to add custom data to tooltips.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGenerateTooltip, TSharedRef<const ISubsystemTreeItem>, class FSubsystemTableItemTooltipBuilder&);
	static FOnGenerateTooltip OnGenerateTooltip;

	/**
	 * Callback that is called whenever a menu for item needs to be generated.
	 * Used to add custom menu actions.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGenerateMenu, TSharedRef<const ISubsystemTreeItem>, UToolMenu*);
	static FOnGenerateMenu OnGenerateContextMenu;

	/**
	 * Apply custom  subsystem  customizations to provided details view
	 * @param DetailsView details view instance to patch
	 * @param Usage which details view being customized
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCustomizeDetails, TSharedRef<IDetailsView>, FName);
	static FOnCustomizeDetails OnCustomizeDetailsView;

protected:
	void RegisterSettings();
	void RegisterMenus();

	/** Handles creating the subsystem browser tab. */
	TSharedRef<SDockTab> HandleSpawnBrowserTab(const FSpawnTabArgs& Args);

private:
	// Is nomad mode enabled
	bool bNomadModeActive = false;
	// Instances of subsystem categories
	TArray<SubsystemCategoryPtr> Categories;
	// Instances of dynamic subsystem columns
	TArray<SubsystemColumnPtr> DynamicColumns;


	// Saved instance of Settings section
	TSharedPtr<ISettingsSection> PluginSettingsSection;
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
