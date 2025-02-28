// Copyright 2022, Aquanox.

#pragma once

#include "CoreFwd.h"
#include "Styling/SlateColor.h"
#include "SubsystemBrowserSettings.generated.h"

USTRUCT()
struct FSubsystemBrowserConfigItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Item)
	FName Name;
	UPROPERTY(EditAnywhere, Category=Item)
	bool bValue = true;

	FSubsystemBrowserConfigItem() = default;
	FSubsystemBrowserConfigItem(const FName& InName, bool InValue) : Name(InName), bValue(InValue) { }

	bool operator==(const FName& OtherName) const { return Name == OtherName; }
};

UENUM()
enum class ESubsystemBrowserSplitterOrientation
{
	Horizontal,
	Vertical,
	// Vertical in Panel mode, Horizontal in Nomad mode
	Auto
};

DECLARE_DELEGATE_RetVal(FString, FSubsystemBrowserGetStringProperty);
DECLARE_DELEGATE_RetVal(FText, FSubsystemBrowserGetTextProperty);
DECLARE_DELEGATE_RetVal(TArray<UObject*>, FSubsystemBrowserGetSubobjects);

struct SUBSYSTEMBROWSER_API FSubsystemBrowserUserMeta
{
	// Subsystem Browser - Subsystem name color in list
	static const FName MD_SBColor;
	// Subsystem Browser - Extra tooltip text when hovering
	static const FName MD_SBTooltip;
	// Subsystem Browser - Owner name provider function/property name (will be called on subsystem)
	static const FName MD_SBOwnerName;
	// Subsystem Browser Details - Hide property from display
	static const FName MD_SBHidden;

	// Subsystem Browser Details - Subobject collection function
	// Specifies a function name that will provide list of subobjects to display
	// TArray<UObject*> MyGetSubobjectsFunction() const;
	static const FName MD_SBGetSubobjects;
	// Subsystem Browser Details - Subobject display
	// Automatically gather market subobjbects for display
	static const FName MD_SBAutoGetSubobjects;
	
};

struct SUBSYSTEMBROWSER_API FSubsystemBrowserConfigMeta
{
	// Config property affects subsystem table view and requires data refresh (no reconstruct)
	static const FName MD_ConfigAffectsView;
	// Config property affects subsystem table and requires table reconstruct
	static const FName MD_ConfigAffectsColumns;
	// Config property affects details view and requires details view refresh
	static const FName MD_ConfigAffectsDetails;
	// Config property affects subsystem settings panel
	static const FName MD_ConfigAffectsSettings;
};

/**
 * Class that holds settings for subsystem browser plugin.
 *
 * It is possible to register it within ISettingsModule to see in Editor Settings.
 */
UCLASS(config=EditorPerProjectUserSettings, meta=(DisplayName="Subsystem Browser Settings"))
class SUBSYSTEMBROWSER_API USubsystemBrowserSettings : public UObject
{
	GENERATED_BODY()
public:
	USubsystemBrowserSettings();

	static USubsystemBrowserSettings* Get()
	{
		return GetMutableDefault<USubsystemBrowserSettings>();
	}

	// Called when browser settings being opened
	void OnSettingsSelected();
	// Called when browser settings were modified
	bool OnSettingsModified();
	// Called when settings reset is requested
	bool OnSettingsReset();

	DECLARE_MULTICAST_DELEGATE_OneParam(FSettingChangedEvent, FName /* InPropertyName */);
	static FSettingChangedEvent& OnSettingChanged() { return SettingChangedEvent; }

	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void SyncCategorySettings();
	void LoadCategoryStates(TMap<FName, bool>& States);
	void SetCategoryStates(const TMap<FName, bool>& States);
	void SetCategoryState(FName Category, bool State);

	void SyncColumnSettings();
	bool GetTableColumnState(FName Column) const;
	void SetTableColumnState(FName Column, bool State);

	bool GetTreeExpansionState(FName Category) const;
	void LoadTreeExpansionStates(TMap<FName, bool>& States);
	void SetTreeExpansionStates(const TMap<FName, bool>& States);

	ESubsystemBrowserSplitterOrientation GetSeparatorOrientation() const;
	float GetSeparatorLocation() const { return SeparatorLocation; }
	void SetSeparatorLocation(float NewValue);

	bool IsColoringEnabled() const { return bEnableColoring; }
	void SetColoringEnabled(bool bNewValue);
	void ToggleColoringEnabled() { SetColoringEnabled(!bEnableColoring); }

	FSlateColor GetSelectedColor() const;
	FSlateColor GetStaleColor() const;
	FSlateColor GetModuleColor(bool bGameModule);

	bool ShouldShowSubobjbects() const { return bShowSubobjects; }
	void SetShowSubobjects(bool bNewValue);
	void ToggleShowSubobjbects() { SetShowSubobjects(!bShowSubobjects); }
	
	bool ShouldForceHiddenPropertyVisibility() const { return bForceHiddenPropertyVisibility; }
	void SetForceHiddenPropertyVisibility(bool bNewValue);
	void ToggleForceHiddenPropertyVisibility() { SetForceHiddenPropertyVisibility(!bForceHiddenPropertyVisibility); }

	bool ShouldShowAnyProperties() const { return bShowAnyProperties; }
	bool ShouldShowAnyConfigProperties() const { return bShowAnyConfigProperties; }

	bool ShouldEditAnyProperties() const { return bEditAnyProperties; }
	bool ShouldEditAnyConfigProperties() const { return bEditAnyConfigProperties; }

	bool ShouldShowOnlyGame() const { return bShowOnlyGameModules; }
	void SetShouldShowOnlyGame(bool bNewValue);
	void ToggleShouldShowOnlyGame() { SetShouldShowOnlyGame(!bShowOnlyGameModules); }

	bool ShouldShowOnlyPlugins() const { return bShowOnlyPluginModules; }
	void SetShouldShowOnlyPlugins(bool bNewValue);
	void ToggleShouldShowOnlyPlugins() { SetShouldShowOnlyPlugins(!bShowOnlyPluginModules); }

	bool ShouldShowOnlyViewable() const { return bShowOnlyWithViewableElements; }
	void SetShouldShowOnlyViewable(bool bNewValue);
	void ToggleShouldShowOnlyViewable() { SetShouldShowOnlyViewable(!bShowOnlyWithViewableElements); }

	bool ShouldHideEmptyCategories() const { return bHideEmptyCategories; }
	void SetShouldHideEmptyCategories(bool bNewValue);
	void ToggleShouldHideEmptyCategories() { SetShouldHideEmptyCategories(!bHideEmptyCategories); }

	int32 GetMaxColumnTogglesToShow() const { return MaxColumnTogglesToShow; }
	int32 GetMaxCategoryTogglesToShow() const { return MaxCategoryTogglesToShow; }

	bool ShouldShowDetailsTooltips() const { return bShowDetailedTooltips; }

	bool ShouldUseSubsystemSettings() const { return bUseSubsystemSettings; }
	bool ShouldUseCustomSettingsWidget() const { return bUseCustomSettingsWidget; }
	bool ShouldUseCustomPropertyFilter() const { return bUseCustomPropertyFilter; }

	bool ShouldUseNomadMode() const { return bUseNomadMode; }

private:

	template<typename TList, typename TMap>
	void LoadDataFromConfig(const TList& InConfigList, TMap& OutMap);

	template<typename TList, typename TMap>
	void StoreDataToConfig(const TMap& InMap, TList& OutConfigList);

	template<typename TMap>
	void SetConfigFlag(TMap& InMap, FName Category, bool State);

protected:
	
	// Should spawn Subsystem Browser Panel as a nomad tab insteaf of Level Editor Panel tab?
	// Editor Restart is required to apply value change. Default is False. 
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigRestartRequired=true))
	bool bUseNomadMode = false;

	// Should show subsystems only from Game Modules?
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowOnlyGameModules = false;

	// Should show subsystems only from Plugins?
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowOnlyPluginModules = false;

	// Should hide categories with no subsystems to show?
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bHideEmptyCategories = false;

	// Should show subsystems that have at least one viewable property or callable function?
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowOnlyWithViewableElements = false;
	
	// Should display subsystem important subobjects?
	// List of subobjects controlled by metadata specifiers
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowSubobjects = true;

	// Enforces display of all hidden object properties in details panel. Results filtered with options below.
	// WARNING: May be unsafe in some use cases.
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsDetails))
	bool bForceHiddenPropertyVisibility = false;

	// Should display hidden properties (without Edit property specifier)
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsDetails, EditCondition="bForceHiddenPropertyVisibility"))
	bool bShowAnyProperties = false;
	
	// Should display hidden properties with Config property specifier
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsDetails, EditCondition="bForceHiddenPropertyVisibility"))
	bool bShowAnyConfigProperties = false;

	// Enforces editing of all object properties in details panel.
	// WARNING: May be unsafe in some use cases.
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsDetails))
	bool bEditAnyProperties = false;

	// Enforces editing of all object properties that have Config specifier.
	UPROPERTY(config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsDetails))
	bool bEditAnyConfigProperties = false;

	// Maximum number of column toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance")
	int32 MaxColumnTogglesToShow = 4;

	// Maximum number of category toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance")
	int32 MaxCategoryTogglesToShow = 6;

	// Should color some data in table?
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(ConfigAffectsView))
	bool bEnableColoring = false;

	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableStaleColor = false;
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableStaleColor"))
	FLinearColor StaleStateColor = FLinearColor(0.75, 0.75, 0.75, 1.0);

	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableSelectedColor = false;
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableSelectedColor"))
	FLinearColor SelectedStateColor = FLinearColor(0.828, 0.364, 0.003, 1.0);

	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableColoringGameModule = false;
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableColoringGameModule"))
	FLinearColor GameModuleColor = FLinearColor(0.4, 0.4, 1.0, 1.0);

	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableColoringEngineModule = false;
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableColoringEngineModule"))
	FLinearColor EngineModuleColor = FLinearColor(0.75, 0.75, 0.75, 1.0);

	// Display additional information in subsystem tooltips
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance")
	bool bShowDetailedTooltips = false;

	// Control TreeView|DetailsView splitter mode
	// Respawning panel or restarting editor is required to apply change. 
	UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance", meta=(ConfigRestartRequired=true))
	ESubsystemBrowserSplitterOrientation SeparatorOrientation = ESubsystemBrowserSplitterOrientation::Auto;
	
	//
	//UPROPERTY(config, EditAnywhere, Category="Browser Panel Appearance")
	UPROPERTY(config)
	float SeparatorLocation = 0.33f;
	
	//
	//UPROPERTY(config, EditAnywhere, Category="Browser Panel State", meta=(ConfigAffectsView, TitleProperty="Name"))
	UPROPERTY(config)
	TArray<FSubsystemBrowserConfigItem> CategoryVisibilityState;

	//
	//UPROPERTY(config, VisibleAnywhere, Category="Browser Panel State", meta=(ConfigAffectsView, TitleProperty="Name"))
	UPROPERTY(config)
	TArray<FSubsystemBrowserConfigItem> TreeExpansionState;

	//
	//UPROPERTY(config, EditAnywhere, Category="Browser Panel State", meta=(ConfigAffectsColumns, TitleProperty="Name"))
	UPROPERTY(config)
	TArray<FSubsystemBrowserConfigItem> TableColumnVisibilityState;

	// Enables subsystem settings panel.
	// Requires editor restart to apply value change.
	UPROPERTY(config, EditAnywhere, Category="Settings Panel", meta=(ConfigRestartRequired=true))
	bool bUseSubsystemSettings = false;
	// Enables use of custom settings widget in Settings panel.
	// Will enable display of built-in subsystems that are configurable but not editable
	UPROPERTY(config, EditAnywhere, Category="Settings Panel")
	bool bUseCustomSettingsWidget = false;
	// Enables use of custom property filter in Settings panel.
	// Will display only properties that have Config flag instead of normal "anything editable" behavior
	UPROPERTY(config, EditAnywhere, Category="Settings Panel")
	bool bUseCustomPropertyFilter = false;

private:
	void NotifyPropertyChange(FName PropertyName);

	// Holds an event delegate that is executed when a setting has changed.
	static FSettingChangedEvent SettingChangedEvent;

};

template <typename TList, typename TMap>
void USubsystemBrowserSettings::LoadDataFromConfig(const TList& InConfigList, TMap& OutMap)
{
	for (const auto& Option : InConfigList)
	{
		OutMap.Add(Option.Name, Option.bValue);
	}
}

template <typename TList, typename TMap>
void USubsystemBrowserSettings::StoreDataToConfig(const TMap& InMap, TList& OutConfigList)
{
	for (const auto& Option : InMap)
	{
		if (auto Existing = OutConfigList.FindByKey(Option.Key))
		{
			Existing->bValue = Option.Value;
		}
		else
		{
			OutConfigList.Emplace(Option.Key, Option.Value);
		}
	}
}

template <typename TMap>
void USubsystemBrowserSettings::SetConfigFlag(TMap& InMap, FName Category, bool State)
{
	if (auto* Existing = InMap.FindByKey(Category))
	{
		Existing->bValue = State;
	}
	else
	{
		InMap.Emplace(FSubsystemBrowserConfigItem{ Category, State });
	}
}
