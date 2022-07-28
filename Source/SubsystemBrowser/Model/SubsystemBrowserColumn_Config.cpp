// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn_Config.h"
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
		FFormatNamedArguments Args;
		Args.Add(TEXT("Config"), Item->IsConfigExportable() ? FText::GetEmpty() : FText::FromName(SubsystemItem->ConfigName));
		return FText::Format(LOCTEXT("SubsystemItem_Config", "{Config}"), Args);
	}

	return FText::GetEmpty();
}

void FSubsystemDynamicColumn_Config::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	if (Item.IsConfigExportable())
	{
		OutSearchStrings.Add(Item.GetAsSubsystemDescriptor()->ConfigName.ToString());
	}
}

#undef LOCTEXT_NAMESPACE