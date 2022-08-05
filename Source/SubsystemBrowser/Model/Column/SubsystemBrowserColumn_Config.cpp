// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Config.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Config::FSubsystemDynamicColumn_Config()
{
	Name = TEXT("Config");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Config", "Config");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Config", "Config");
	PreferredWidthRatio = 0.15f;
}

FText FSubsystemDynamicColumn_Config::ExtractText(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
	{
		return Item->IsConfigExportable() ? FText::FromName(SubsystemItem->ConfigName) : FText::GetEmpty();
	}

	return FText::GetEmpty();
}

void FSubsystemDynamicColumn_Config::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item.GetAsSubsystemDescriptor())
	{
		if (SubsystemItem->IsConfigExportable())
		{
			OutSearchStrings.Add(SubsystemItem->ConfigName.ToString());
		}
	}
}

#undef LOCTEXT_NAMESPACE