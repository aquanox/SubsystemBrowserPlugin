// Copyright 2022, Aquanox.

#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserModule.h"
#include "Model/SubsystemBrowserModel.h"

USubsystemBrowserSettings::FSettingChangedEvent USubsystemBrowserSettings::SettingChangedEvent;

const FName FSubsystemBrowserConfigMeta::MD_ConfigAffectsView(TEXT("ConfigAffectsView"));
const FName FSubsystemBrowserConfigMeta::MD_ConfigAffectsColumns(TEXT("ConfigAffectsColumns"));
const FName FSubsystemBrowserConfigMeta::MD_ConfigAffectsDetails(TEXT("ConfigAffectsDetails"));
const FName FSubsystemBrowserConfigMeta::MD_ConfigAffectsSettings(TEXT("ConfigAffectsSettings"));

const FName FSubsystemBrowserUserMeta::MD_SBColor(TEXT("SBColor"));
const FName FSubsystemBrowserUserMeta::MD_SBTooltip(TEXT("SBTooltip"));
const FName FSubsystemBrowserUserMeta::MD_SBOwnerName(TEXT("SBOwnerName"));
const FName FSubsystemBrowserUserMeta::MD_SBHidden(TEXT("SBHidden"));
const FName FSubsystemBrowserUserMeta::MD_SBGetSubobjects(TEXT("SBGetSubobjects"));
const FName FSubsystemBrowserUserMeta::MD_SBAutoGetSubobjects(TEXT("SBAutoGetSubobjects"));
const FName FSubsystemBrowserUserMeta::MD_SBQuickAction(TEXT("SBQuickAction"));

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

	CategoryVisibilityState.Empty();
	TreeExpansionState.Empty();
	TableColumnVisibilityState.Empty();

	bShowOnlyGameModules = false;
	bShowOnlyPluginModules = false;
	bShowAnyProperties = false;
	bEditAnyProperties = false;
	bShowOnlyWithViewableElements = false;

	MaxColumnTogglesToShow = 4;
	MaxCategoryTogglesToShow = 6;

	bEnableColoring = false;

	bEnableStaleColor = false;
	StaleStateColor = FLinearColor(0.75, 0.75, 0.75, 1.0);
	bEnableSelectedColor = false;
	SelectedStateColor = FLinearColor(0.828, 0.364, 0.003, 1.0);
	bEnableColoringGameModule = false;
	GameModuleColor = FLinearColor(0.4, 0.4, 1.0, 1.0);
	bEnableColoringEngineModule = false;
	EngineModuleColor = FLinearColor(0.75, 0.75, 0.75, 1.0);


	NotifyPropertyChange(NAME_All);

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
			CurrentSettings.Emplace(Category->Name, Category->IsVisibleByDefaultInBrowser());
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

bool USubsystemBrowserSettings::GetTreeExpansionState(FName Category) const
{
	const FSubsystemBrowserConfigItem* bFoundState = TreeExpansionState.FindByKey(Category);
	return bFoundState ? bFoundState->bValue : true;
}

void USubsystemBrowserSettings::LoadTreeExpansionStates(TMap<FName, bool>& States)
{
	LoadDataFromConfig(TreeExpansionState, States);
}

void USubsystemBrowserSettings::SetTreeExpansionStates(TMap<FName, bool> const& States)
{
	StoreDataToConfig(States, TreeExpansionState);
	//NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, TreeExpansionState));
}

ESubsystemBrowserSplitterOrientation USubsystemBrowserSettings::GetSeparatorOrientation() const
{
	if (SeparatorOrientation == ESubsystemBrowserSplitterOrientation::Auto)
	{
		return bUseNomadMode ? ESubsystemBrowserSplitterOrientation::Horizontal : ESubsystemBrowserSplitterOrientation::Vertical;
	}
	return SeparatorOrientation;
}

void USubsystemBrowserSettings::SetSeparatorLocation(float NewValue)
{
	SeparatorLocation = NewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, SeparatorLocation));
}

void USubsystemBrowserSettings::SetColoringEnabled(bool bNewValue)
{
	bEnableColoring = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bEnableColoring));
}

FSlateColor USubsystemBrowserSettings::GetSelectedColor() const
{
	if (bEnableColoring && bEnableSelectedColor)
	{
		return SelectedStateColor;
	}
	return FSlateColor::UseForeground();
}

FSlateColor USubsystemBrowserSettings::GetStaleColor() const
{
	if (bEnableColoring && bEnableStaleColor)
	{
		return StaleStateColor;
	}
	return FSlateColor::UseSubduedForeground();
}

FSlateColor USubsystemBrowserSettings::GetModuleColor(bool bGameModule)
{
	if (bEnableColoring)
	{
		if (bGameModule && bEnableColoringGameModule)
			return GameModuleColor;
		if (!bGameModule && bEnableColoringEngineModule)
			return EngineModuleColor;
	}
	return FSlateColor::UseForeground();
}

void USubsystemBrowserSettings::SetShowSubobjects(bool bNewValue)
{
	bShowSubobjects = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bShowSubobjects));
}

void USubsystemBrowserSettings::SetForceHiddenPropertyVisibility(bool bNewValue)
{
	bForceHiddenPropertyVisibility = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bForceHiddenPropertyVisibility));
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

void USubsystemBrowserSettings::SetShouldShowOnlyViewable(bool bNewValue)
{
	bShowOnlyWithViewableElements = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bShowOnlyWithViewableElements));
}

void USubsystemBrowserSettings::SetShouldHideEmptyCategories(bool bNewValue)
{
	bHideEmptyCategories = bNewValue;
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, bHideEmptyCategories));
}

bool USubsystemBrowserSettings::TryFindNamedColor(const FName& InName, FLinearColor& OutColor) const
{
	if (const FSubsystemBrowserNamedColorEntry* Entry = NamedColors.FindByKey(InName))
	{
		OutColor = Entry->Color;
		return true;
	}
	return false;
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
