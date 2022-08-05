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

	bool ShouldShowHiddenProperties() const { return bShowHiddenProperties; }
	void SetShowHiddenProperties(bool bNewValue);
	void ToggleShouldShowHiddenProperties() { SetShowHiddenProperties(!bShowHiddenProperties); }

	bool ShouldShowOnlyGame() const { return bShowOnlyGameModules; }
	void SetShouldShowOnlyGame(bool bNewValue);
	void ToggleShouldShowOnlyGame() { SetShouldShowOnlyGame(!bShowOnlyGameModules); }

	bool ShouldShowOnlyPlugins() const { return bShowOnlyPluginModules; }
	void SetShouldShowOnlyPlugins(bool bNewValue);
	void ToggleShouldShowOnlyPlugins() { SetShouldShowOnlyPlugins(!bShowOnlyPluginModules); }

private:

	template<typename TList, typename TMap>
	void LoadDataFromConfig(const TList& InConfigList, TMap& OutMap);

	template<typename TList, typename TMap>
	void StoreDataToConfig(const TMap& InMap, TList& OutConfigList);

	template<typename TMap>
	void SetConfigFlag(TMap& InMap, FName Category, bool State);

protected:

	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsView, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> CategoryVisibilityState;

	UPROPERTY(config, /*EditAnywhere, Category=General,*/ meta=(ConfigAffectsView, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> TreeExpansionState;

	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsColumns, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> TableColumnVisibilityState;

	UPROPERTY(config, EditAnywhere, Category=General)
	float HorizontalSeparatorLocation = 0.33f;

	// Should color some data in table?
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsView))
	bool bEnableColoring = false;

	// Should show hidden properties in Details View?
	// Enforces display of all hidden object properties in details panel.
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsDetails))
	bool bShowHiddenProperties = true;

	// Should show subsystems only from Game Modules?
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsView))
	bool bShowOnlyGameModules = false;

	// Should show subsystems only from Plugins?
	UPROPERTY(config, EditAnywhere, Category=General, meta=(ConfigAffectsView))
	bool bShowOnlyPluginModules = false;

public:
	// Maximum number of column toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(config, EditAnywhere, Category=General)
	int32 MaxColumnTogglesToShow = 4;

	// Maximum number of category toggles to show in menu before folding into submenu
	// Specify 0 to always fold
	UPROPERTY(config, EditAnywhere, Category=General)
	int32 MaxCategoryTogglesToShow = 6;

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
