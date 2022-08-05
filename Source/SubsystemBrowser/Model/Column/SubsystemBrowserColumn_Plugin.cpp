// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Plugin.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Plugin::FSubsystemDynamicColumn_Plugin()
{
	Name = TEXT("Plugin");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Plugin", "Plugin");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Plugin", "Plugin");
	PreferredWidthRatio = 0.25f;
}

FText FSubsystemDynamicColumn_Plugin::ExtractText(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
	{
		return FText::FromString(SubsystemItem->PluginDisplayName);
	}

	return FText::GetEmpty();
}

FText FSubsystemDynamicColumn_Plugin::ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const
{
	return ExtractText(Item);
}

void FSubsystemDynamicColumn_Plugin::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item.GetAsSubsystemDescriptor())
	{
		OutSearchStrings.Add(SubsystemItem->PluginDisplayName);
	}
}

#undef LOCTEXT_NAMESPACE
