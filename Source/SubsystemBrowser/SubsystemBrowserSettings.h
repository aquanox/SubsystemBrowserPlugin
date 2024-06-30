// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
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

// Dynamic delegate for owner name requesting
DECLARE_DYNAMIC_DELEGATE_RetVal(FString, FSubsystemBrowserGetOwnerName);

struct FSubsystemBrowserUserMeta
{
	// Subsystem name color in list
	static const FName MD_SBColor;
	// Extra tooltip text when hovering 
	static const FName MD_SBTooltip;
	// Owner name provider function/property name (will be called on subsystem)
	static const FName MD_SBOwnerName;
};

struct FSubsystemBrowserConfigMeta
{
	static const FName MD_ConfigAffectsView;
	static const FName MD_ConfigAffectsColumns;
	static const FName MD_ConfigAffectsDetails;
};

/**
 * Class that holds settings for subsystem browser plugin.
 *
 * It is possible to register it within ISettingsModule to see in Editor Settings.
 */
UCLASS(config=EditorPerProjectUserSettings, meta=(DisplayName="Subsystem Browser Settings"))
class USubsystemBrowserSettings : public UObject
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

	DECLARE_EVENT_OneParam(USubsystemBrowserSettings, FSettingChangedEvent, FName /*PropertyName*/);
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

	void LoadTreeExpansionStates(TMap<FName, bool>& States);
	void SetTreeExpansionStates(const TMap<FName, bool>& States);

	float GetSeparatorLocation() const { return HorizontalSeparatorLocation; }
	void SetSeparatorLocation(float NewValue);

	bool IsColoringEnabled() const { return bEnableColoring; }
	void SetColoringEnabled(bool bNewValue);
	void ToggleColoringEnabled() { SetColoringEnabled(!bEnableColoring); }

	FSlateColor GetSelectedColor() const;
	FSlateColor GetStaleColor() const;
	FSlateColor GetModuleColor(bool bGameModule);

	bool ShouldShowHiddenProperties() const { return bShowHiddenProperties; }
	void SetShowHiddenProperties(bool bNewValue);
	void ToggleShouldShowHiddenProperties() { SetShowHiddenProperties(!bShowHiddenProperties); }

	bool ShouldEditAnyProperties() const { return bEditAnyProperties; }

	bool ShouldShowOnlyGame() const { return bShowOnlyGameModules; }
	void SetShouldShowOnlyGame(bool bNewValue);
	void ToggleShouldShowOnlyGame() { SetShouldShowOnlyGame(!bShowOnlyGameModules); }

	bool ShouldShowOnlyPlugins() const { return bShowOnlyPluginModules; }
	void SetShouldShowOnlyPlugins(bool bNewValue);
	void ToggleShouldShowOnlyPlugins() { SetShouldShowOnlyPlugins(!bShowOnlyPluginModules); }

	int32 GetMaxColumnTogglesToShow() const { return MaxColumnTogglesToShow; }
	int32 GetMaxCategoryTogglesToShow() const { return MaxCategoryTogglesToShow; }

private:

	template<typename TList, typename TMap>
	void LoadDataFromConfig(const TList& InConfigList, TMap& OutMap);

	template<typename TList, typename TMap>
	void StoreDataToConfig(const TMap& InMap, TList& OutConfigList);

	template<typename TMap>
	void SetConfigFlag(TMap& InMap, FName Category, bool State);

protected:

	// Should show subsystems only from Game Modules?
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsView))
	bool bShowOnlyGameModules = false;

	// Should show subsystems only from Plugins?
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsView))
	bool bShowOnlyPluginModules = false;

	// Should show hidden properties in Details View?
	// Enforces display of all hidden object properties in details panel.
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsDetails))
	bool bShowHiddenProperties = false;

	// Should force editing of all properties in Details View?
	// Enforces editing of all visible object properties in details panel.
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsDetails))
	bool bEditAnyProperties = false;

	// Maximum number of column toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(config, EditAnywhere, Category=Visuals)
	int32 MaxColumnTogglesToShow = 4;

	// Maximum number of category toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(config, EditAnywhere, Category=Visuals)
	int32 MaxCategoryTogglesToShow = 6;

	// Should color some data in table?
	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(ConfigAffectsView))
	bool bEnableColoring = false;

	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(InlineEditConditionToggle))
	bool bEnableStaleColor = false;
	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(EditCondition="bEnableStaleColor"))
	FLinearColor StaleStateColor = FLinearColor(0.4, 0.4, 1.0);
	
	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(InlineEditConditionToggle))
	bool bEnableSelectedColor = false;
	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(EditCondition="bEnableSelectedColor"))
	FLinearColor SelectedStateColor = FLinearColor(0.828, 0.364, 0.003, 1.0);

	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(InlineEditConditionToggle))
	bool bEnableColoringGameModule = false;
	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(EditCondition="bEnableColoringGameModule"))
	FLinearColor GameModuleColor = FLinearColor(0.4, 0.4, 1.0, 1.0);

	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(InlineEditConditionToggle))
	bool bEnableColoringEngineModule = false;
	UPROPERTY(config, EditAnywhere, Category=Visuals, meta=(EditCondition="bEnableColoringEngineModule"))
	FLinearColor EngineModuleColor = FLinearColor(0.75, 0.75, 0.75, 1.0);

	// 
	UPROPERTY(config, EditAnywhere, Category=State, meta=(ConfigAffectsView, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> CategoryVisibilityState;

	// 
	UPROPERTY(config, EditAnywhere, Category=State, meta=(ConfigAffectsView, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> TreeExpansionState;

	//
	UPROPERTY(config, EditAnywhere, Category=State, meta=(ConfigAffectsColumns, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> TableColumnVisibilityState;

	//
	UPROPERTY(config, EditAnywhere, Category=State)
	float HorizontalSeparatorLocation = 0.33f;
	
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
