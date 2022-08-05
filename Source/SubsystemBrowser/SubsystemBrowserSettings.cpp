// Copyright 2022, Aquanox.

#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserModule.h"
#include "Model/SubsystemBrowserModel.h"

USubsystemBrowserSettings::FSettingChangedEvent USubsystemBrowserSettings::SettingChangedEvent;

USubsystemBrowserSettings::USubsystemBrowserSettings()
{
}

void USubsystemBrowserSettings::OnSettingsSelected()
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Browser settings being selected"));

	SyncCategorySettings();
	SyncColumnSettings();
}

bool USubsystemBrowserSettings::OnSettingsModified()
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Browser settings being modified"));
	return true;
}

bool USubsystemBrowserSettings::OnSettingsReset()
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Browser settings being reset"));
	return true;
}

// Sync category settings with categories we have in module
void USubsystemBrowserSettings::SyncCategorySettings()
{
	TMap<FName, bool> CurrentSettings;
	LoadDataFromConfig(CategoryVisibilityState, CurrentSettings);
	// clear from possible stale/removed categories
	CategoryVisibilityState.Empty();

	for (const auto& Category : FSubsystemBrowserModule::Get().GetCategories())
	{
		if (!CurrentSettings.Contains(Category->Name))
		{
			CurrentSettings.Emplace(Category->Name, Category->IsVisibleByDefault());
		}
	}

	StoreDataToConfig(CurrentSettings, CategoryVisibilityState);
}

void USubsystemBrowserSettings::LoadCategoryStates(TMap<FName, bool>& States)
{
	SyncCategorySettings();
	LoadDataFromConfig(CategoryVisibilityState, States);
}

void USubsystemBrowserSettings::SetCategoryStates(const TMap<FName, bool>& States)
{
	StoreDataToConfig(States, CategoryVisibilityState);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, CategoryVisibilityState));
}

void USubsystemBrowserSettings::SetCategoryState(FName Category, bool State)
{
	SetConfigFlag(CategoryVisibilityState, Category, State);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, CategoryVisibilityState));
}

void USubsystemBrowserSettings::LoadTreeExpansionStates(TMap<FName, bool>& States)
{
	LoadDataFromConfig(TreeExpansionState, States);
}

void USubsystemBrowserSettings::SetTreeExpansionStates(TMap<FName, bool> const& States)
{
	StoreDataToConfig(States, TreeExpansionState);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, TreeExpansionState));
}

void USubsystemBrowserSettings::SetSeparatorLocation(float NewValue)
{
	HorizontalSeparatorLocation = NewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, HorizontalSeparatorLocation));
}

void USubsystemBrowserSettings::SetColoringEnabled(bool bNewValue)
{
	bEnableColoring = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bEnableColoring));
}

void USubsystemBrowserSettings::SetShowHiddenProperties(bool bNewValue)
{
	bShowHiddenProperties = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bShowHiddenProperties));
}

void USubsystemBrowserSettings::SetShouldShowOnlyGame(bool bNewValue)
{
	bShowOnlyGameModules = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bShowOnlyGameModules));
}

void USubsystemBrowserSettings::SetShouldShowOnlyPlugins(bool bNewValue)
{
	bShowOnlyPluginModules = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bShowOnlyPluginModules));
}

void USubsystemBrowserSettings::SyncColumnSettings()
{
	TMap<FName, bool> CurrentSettings;
	LoadDataFromConfig(TableColumnVisibilityState, CurrentSettings);
	// clear from possible stale/removed categories
	TableColumnVisibilityState.Empty();

	for (const auto& DynamicColumn : FSubsystemBrowserModule::Get().GetDynamicColumns())
	{
		if (!CurrentSettings.Contains(DynamicColumn->Name))
		{
			CurrentSettings.Emplace(DynamicColumn->Name, DynamicColumn->IsVisibleByDefault());
		}
	}

	StoreDataToConfig(CurrentSettings, TableColumnVisibilityState);
}

bool USubsystemBrowserSettings::GetTableColumnState(FName Column) const
{
	const FSubsystemBrowserConfigItem* bFoundState = TableColumnVisibilityState.FindByKey(Column);
	return bFoundState ? bFoundState->bValue : true;
}

void USubsystemBrowserSettings::SetTableColumnState(FName Column, bool State)
{
	SetConfigFlag(TableColumnVisibilityState, Column, State);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, TableColumnVisibilityState));
}

void USubsystemBrowserSettings::NotifyPropertyChange(FName PropertyName)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Property %s changed and called save"), *PropertyName.ToString());
	SaveConfig();

	SettingChangedEvent.Broadcast(PropertyName);
}

void USubsystemBrowserSettings::PostLoad()
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Browser settings being loaded"));

	Super::PostLoad();
}

void USubsystemBrowserSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Take the class member property name instead of struct member
	FName PropertyName = (PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : PropertyChangedEvent.GetPropertyName());

	NotifyPropertyChange(PropertyName);
}
