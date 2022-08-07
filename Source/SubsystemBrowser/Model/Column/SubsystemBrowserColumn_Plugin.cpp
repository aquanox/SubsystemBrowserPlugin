// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Plugin.h"

#include "UI/SubsystemTableItem.h"
#include "UI/SubsystemTableItemTooltip.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Plugin::FSubsystemDynamicColumn_Plugin()
{
	Name = TEXT("Plugin");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Plugin", "Plugin");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Plugin", "Plugin");
	PreferredWidthRatio = 0.25f;
}

FText FSubsystemDynamicColumn_Plugin::ExtractText(TSharedRef<const ISubsystemTreeItem> Item) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
	{
		return FText::FromString(SubsystemItem->PluginDisplayName);
	}

	return FText::GetEmpty();
}

void FSubsystemDynamicColumn_Plugin::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item.GetAsSubsystemDescriptor())
	{
		OutSearchStrings.Add(SubsystemItem->PluginDisplayName);
	}
}

#undef LOCTEXT_NAMESPACE
