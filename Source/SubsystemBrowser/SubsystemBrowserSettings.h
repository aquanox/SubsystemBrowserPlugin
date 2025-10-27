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
DECLARE_DELEGATE(FSubsystemBrowserQuickAction);
DECLARE_DELEGATE_OneParam(FSubsystemBrowserQuickActionWorldContext, const UObject*);

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
	// 
	// If Parameter is empty or "auto" - Automatically gather marked subobjbects for display
	static const FName MD_SBGetSubobjects;
	// Important subobject display marker meta
	static const FName MD_SBSubobject;

	// Subsystem Browser Panel - Quick Actions
	static const FName MD_SBQuickAction;
	
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

USTRUCT()
struct FSubsystemBrowserNamedColorEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Color)
	FName Name;
	UPROPERTY(EditAnywhere, Category=Color)
	FLinearColor Color = FLinearColor::White;

	bool operator==(const FName& InKey) const { return Name == InKey; }
};

USTRUCT()
struct FSubsystemIgnoreListEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Entry)
	FString FilterString;
	UPROPERTY(EditAnywhere, Category=Entry)
	bool bMatchSubstring = false;

	bool operator==(const FSubsystemIgnoreListEntry& Other) const
	{
		return FilterString == Other.FilterString;
	}
};

/**
 * Class that holds settings for subsystem browser plugin.
 *
 * It is possible to register it within ISettingsModule to see in Editor Settings.
 */
UCLASS(Config=EditorPerProjectUserSettings, meta=(DisplayName="Subsystem Browser Settings"))
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
	// Reset all settings to default values
	// UFUNCTION(CallInEditor, DisplayName="Reset to Defaults", Category="Actions")
	void SetDefaults();

	DECLARE_MULTICAST_DELEGATE_OneParam(FSettingChangedEvent, FName /* InPropertyName */);
	static FSettingChangedEvent& OnSettingChanged() { return SettingChangedEvent; }

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

	bool ShouldUseCustomPropertyFilteringInBrowser() const { return bUseCustomPropertyFilterInBrowser; }
	
	bool ShouldForceHiddenPropertyVisibility() const { return bForceHiddenPropertyVisibility; }
	void SetForceHiddenPropertyVisibility(bool bNewValue);
	void ToggleForceHiddenPropertyVisibility() { SetForceHiddenPropertyVisibility(!bForceHiddenPropertyVisibility); }

	bool ShouldShowAnyProperties() const { return bShowAnyProperties; }
	bool ShouldEditAnyProperties() const { return bEditAnyProperties; }

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
	int32 GetMaxQuickActionsToShow() const { return MaxQuickActionsToShow; }

	bool ShouldShowDetailsTooltips() const { return bShowDetailedTooltips; }

	bool ShouldUseSubsystemSettings() const { return bUseSubsystemSettings; }
	bool ShouldUseCustomSettingsWidget() const { return bUseCustomSettingsWidget; }
	bool ShouldUseCustomPropertyFilterInSettings() const { return bUseCustomPropertyFilterInSettings; }

	bool ShouldUseNomadMode() const { return bUseNomadMode; }

	bool TryFindNamedColor(const FName& InName, FLinearColor& OutColor) const;

	bool ShouldDisplayAllWorlds() const { return bShowAllWorlds; }
	bool ShouldDisplayConfigExportActions() const { return bConfigExportActions; }

	bool HasIgnoredSubsystems() const { return IgnoredSubsystems.Num() > 0; }
	bool IsSubsystemIgnored(FString InClass) const;
	void AddToIgnoreList(FString InClass, bool bMatchSubstring);

protected:
	
	// Should spawn Subsystem Browser Panel as a standalone (nomad) tab insteaf of Level Editor Panel tab?
	// Editor Restart is required to apply value change. Default is False. 
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigRestartRequired=true))
	bool bUseNomadMode = false;

	// Should show subsystems only from Game Modules?
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowOnlyGameModules = false;

	// Should show subsystems only from Plugins?
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowOnlyPluginModules = false;

	// Should hide categories with no subsystems?
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bHideEmptyCategories = false;

	// Should show subsystems that have at least one viewable property or callable function?
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowOnlyWithViewableElements = false;
	
	// Should display subsystem important subobjects?
	// List of subobjects controlled by metadata specifiers
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView))
	bool bShowSubobjects = false;

	// Display additional information in subsystem panel tooltips
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel")
	bool bShowDetailedTooltips = false;

	// Enables selection of all known worlds (instead of only WorldType::PIE and WorldType::Editor)
	// Useful when need to see subsystems in Editor Preview worlds (Blueprint Viewport)
	// WARNING: Advanced users only
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel")
	bool bShowAllWorlds = false;

	// Enables legacy config export actions that were used before Settings panel was introduced
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel")
	bool bConfigExportActions = false;

	// Matching objects will be automatically filtered out
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel", meta=(ConfigAffectsView, TitleProperty="FilterString"))
	TArray<FSubsystemIgnoreListEntry> IgnoredSubsystems;
	
	// Enables hidden property display (Without Edit flag) on details view
	// WARNING: Advanced users only
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Details", meta=(ConfigAffectsDetails))
	bool bForceHiddenPropertyVisibility = false;

	// Enables custom property filtering.
	// Hides Delegates and explicitly hidden properties from Details. Additional options below.
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Details", meta=(ConfigAffectsDetails))
	bool bUseCustomPropertyFilterInBrowser = true;

	// Enforces all object hidden properties (without Edit property specifier) to be visible
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Details", meta=(ConfigAffectsDetails, EditConditionHides, EditCondition="bUseCustomPropertyFilterInBrowser"))
	bool bShowAnyProperties = false;
	
	// Enforces all object properties in details panel to be editable.
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Details", meta=(ConfigAffectsDetails, EditConditionHides, EditCondition="bUseCustomPropertyFilterInBrowser"))
	bool bEditAnyProperties = false;

	// Maximum number of column toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance")
	int32 MaxColumnTogglesToShow = 4;

	// Maximum number of category toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance")
	int32 MaxCategoryTogglesToShow = 6;

	// Maximum number of quick actions to show
	// Specify 0 to always fold
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance")
	int32 MaxQuickActionsToShow = 4;

	// Should color some data in table?
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(ConfigAffectsView))
	bool bEnableColoring = false;

	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableStaleColor = false;
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableStaleColor"))
	FLinearColor StaleStateColor = FLinearColor(0.75, 0.75, 0.75, 1.0);

	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableSelectedColor = false;
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableSelectedColor"))
	FLinearColor SelectedStateColor = FLinearColor(0.828, 0.364, 0.003, 1.0);

	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableColoringGameModule = false;
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableColoringGameModule"))
	FLinearColor GameModuleColor = FLinearColor(0.4, 0.4, 1.0, 1.0);

	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(InlineEditConditionToggle))
	bool bEnableColoringEngineModule = false;
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(EditCondition="bEnableColoringEngineModule"))
	FLinearColor EngineModuleColor = FLinearColor(0.75, 0.75, 0.75, 1.0);

	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(TitleProperty="Name"))
	TArray<FSubsystemBrowserNamedColorEntry> NamedColors;

	// Control TreeView|DetailsView splitter mode
	// Respawning panel or restarting editor is required to apply change. 
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance", meta=(ConfigRestartRequired=true))
	ESubsystemBrowserSplitterOrientation SeparatorOrientation = ESubsystemBrowserSplitterOrientation::Auto;
	
	// Location of separator between subsystem tree and details
	UPROPERTY(Config, EditAnywhere, Category="Browser Panel Appearance")
	float SeparatorLocation = 0.33f;
	
	// Category visibility state.
	UPROPERTY(Config)
	TArray<FSubsystemBrowserConfigItem> CategoryVisibilityState;

	// Tree expansion state
	UPROPERTY(Config)
	TArray<FSubsystemBrowserConfigItem> TreeExpansionState;

	// Table column state
	UPROPERTY(Config)
	TArray<FSubsystemBrowserConfigItem> TableColumnVisibilityState;

	// Enables subsystem settings panel.
	// Requires editor restart to apply value change.
	UPROPERTY(Config, EditAnywhere, Category="Settings Panel", meta=(ConfigRestartRequired=true))
	bool bUseSubsystemSettings = false;
	// Enables use of custom settings widget in Settings panel.
	// Will enable display of built-in subsystems that are configurable but not editable
	UPROPERTY(Config, EditAnywhere, Category="Settings Panel", meta=(ConfigAffectsSettings))
	bool bUseCustomSettingsWidget = false;
	// Enables use of custom property filter in Settings panel.
	UPROPERTY(Config, EditAnywhere, Category="Settings Panel", meta=(ConfigRestartRequired=true))
	bool bUseCustomPropertyFilterInSettings = true;

private:
	bool bReloadingConfig = false;

	// Notify system that a property was externally changed
	void NotifyPropertyChange(FName PropertyName);

	// Holds an event delegate that is executed when a setting has changed.
	static FSettingChangedEvent SettingChangedEvent;

};
