// Copyright 2022, Aquanox.

#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserModule.h"
#include "Model/SubsystemBrowserModel.h"

USubsystemBrowserSettings::FSettingChangedEvent USubsystemBrowserSettings::SettingChangedEvent;

namespace Helpers
{
	template<typename TList, typename TMap>
	void LoadDataFromConfig(const TList& InConfigList, TMap& OutMap)
	{
		for (const auto& Option : InConfigList)
		{
			OutMap.Add(Option.Name, Option.bValue);
		}
	}

	template<typename TList, typename TMap>
	void StoreDataToConfig(const TMap& InMap, TList& OutConfigList)
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

	template<typename TMap>
	void SetConfigFlag(TMap& InMap, FName Category, bool State)
	{
		if (auto Existing = InMap.FindByKey(Category))
		{
			Existing->bValue = State;
		}
		else
		{
			InMap.Emplace(FSubsystemBrowserConfigItem { Category, State });
		}
	}
}


USubsystemBrowserSettings::USubsystemBrowserSettings()
{
	CategoryVisibilityState.Emplace(SubsystemCategories::CategoryEngine, true);
	CategoryVisibilityState.Emplace(SubsystemCategories::CategoryEditor, true);
	CategoryVisibilityState.Emplace(SubsystemCategories::CategoryGameInstance, true);
	CategoryVisibilityState.Emplace(SubsystemCategories::CategoryPlayer, true);
	CategoryVisibilityState.Emplace(SubsystemCategories::CategoryWorld, true);

	TableColumnVisibilityState.Emplace(SubsystemColumns::ColumnID_Package, true);
	TableColumnVisibilityState.Emplace(SubsystemColumns::ColumnID_ConfigClass, true);
}

void USubsystemBrowserSettings::LoadCategoryStates(TMap<FName, bool>& States)
{
	Helpers::LoadDataFromConfig(CategoryVisibilityState, States);
}

void USubsystemBrowserSettings::SetCategoryStates(const TMap<FName, bool>& States)
{
	Helpers::StoreDataToConfig(States, CategoryVisibilityState);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, CategoryVisibilityState));
}

void USubsystemBrowserSettings::SetCategoryState(FName Category, bool State)
{
	Helpers::SetConfigFlag(CategoryVisibilityState, Category, State);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, CategoryVisibilityState));
}

void USubsystemBrowserSettings::LoadTreeExpansionStates(TMap<FName, bool>& States)
{
	Helpers::LoadDataFromConfig(TreeExpansionState, States);
}

void USubsystemBrowserSettings::SetTreeExpansionStates(TMap<FName, bool> const& States)
{
	Helpers::StoreDataToConfig(States, TreeExpansionState);
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

bool USubsystemBrowserSettings::GetTableColumnState(FName Column) const
{
	const FSubsystemBrowserConfigItem* bFoundState = TableColumnVisibilityState.FindByKey(Column);
	return bFoundState ? bFoundState->bValue : true;
}

void USubsystemBrowserSettings::SetTableColumnState(FName Column, bool State)
{
	Helpers::SetConfigFlag(TableColumnVisibilityState, Column, State);
	NotifyPropertyChange(GET_MEMBER_NAME_CHECKED(ThisClass, TableColumnVisibilityState));
}

void USubsystemBrowserSettings::NotifyPropertyChange(FName PropertyName)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Property %s changed and called save"), *PropertyName.ToString());
	SaveConfig();

	SettingChangedEvent.Broadcast(PropertyName);
}

void USubsystemBrowserSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	// Take the class member property name instead of struct member
	FName PropertyName = (PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : PropertyChangedEvent.GetPropertyName());

	NotifyPropertyChange(PropertyName);
}
