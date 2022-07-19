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
	FFormatNamedArguments Args;
	Args.Add(TEXT("Config"), FText::FromString(Item->GetConfigNameString()));
	return FText::Format(LOCTEXT("SubsystemItem_Config", "{Config}"), Args);
}

void FSubsystemDynamicColumn_Config::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	FString Value = Item.GetConfigNameString();
	if (!Value.IsEmpty())
	{
		OutSearchStrings.Add(Value);
	}
}

#undef LOCTEXT_NAMESPACE