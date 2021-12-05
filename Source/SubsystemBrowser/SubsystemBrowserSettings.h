#pragma once

#include "CoreMinimal.h"
#include "SubsystemBrowserSettings.generated.h"

USTRUCT()
struct FSubsystemBrowserConfigItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName Name;
	UPROPERTY(EditAnywhere)
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

	DECLARE_EVENT_OneParam(USubsystemBrowserSettings, FSettingChangedEvent, FName /*PropertyName*/);
	static FSettingChangedEvent& OnSettingChanged() { return SettingChangedEvent; }

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void LoadCategoryStates(TMap<FName, bool>& States);
	void SetCategoryStates(const TMap<FName, bool>& States);
	void SetCategoryState(FName Category, bool State);

	bool GetTableColumnState(FName Column) const;
	void SetTableColumnState(FName Column, bool State);

	void LoadTreeExpansionStates(TMap<FName, bool>& States);
	void SetTreeExpansionStates(const TMap<FName, bool>& States);

	float GetSeparatorLocation() const { return HorizontalSeparatorLocation; }
	void SetSeparatorLocation(float NewValue);

	bool IsColoringEnabled() const { return bEnableColoring; }
	void SetColoringEnabled(bool bNewValue);

	bool ShouldShowHiddenProperties() const { return bShowHiddenProperties; }
	void SetShowHiddenProperties(bool bNewValue);

	bool ShouldShowOnlyGame() const { return bShowOnlyGameModules; }
	void SetShouldShowOnlyGame(bool bNewValue);

protected:

	UPROPERTY(config, EditAnywhere, meta=(ConfigAffectsView, EditFixedSize, EditFixedOrder, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> CategoryVisibilityState;

	UPROPERTY(config, /*EditAnywhere,*/ meta=(ConfigAffectsView, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> TreeExpansionState;

	UPROPERTY(config, EditAnywhere, meta=(ConfigAffectsColumns, EditFixedSize, EditFixedOrder, TitleProperty="Name"))
	TArray<FSubsystemBrowserConfigItem> TableColumnVisibilityState;

	UPROPERTY(config, EditAnywhere)
	float HorizontalSeparatorLocation = 0.33f;

	UPROPERTY(config, EditAnywhere, meta=(ConfigAffectsView))
	bool bEnableColoring = false;

	UPROPERTY(config, EditAnywhere, meta=(ConfigAffectsDetails))
	bool bShowHiddenProperties = true;

	UPROPERTY(config, EditAnywhere, meta=(ConfigAffectsView))
	bool bShowOnlyGameModules = false;

private:
	void NotifyPropertyChange(FName PropertyName);

	// Holds an event delegate that is executed when a setting has changed.
	static FSettingChangedEvent SettingChangedEvent;

};
