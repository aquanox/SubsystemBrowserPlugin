// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn_Module.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Module::FSubsystemDynamicColumn_Module()
{
	Name = TEXT("Module");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Module", "Module");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Module", "Module");
	PreferredWidthRatio = 0.25f;
}

FText FSubsystemDynamicColumn_Module::ExtractText(TSharedRef<ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Module"), FText::FromString(Item->GetShortPackageString()));
	return FText::Format(LOCTEXT("SubsystemItem_Module", "{Module}"), Args);
}

FText FSubsystemDynamicColumn_Module::ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Module"), FText::FromString(Item->GetPackageString()));
	Args.Add(TEXT("Plugin"), FText::FromString(Item->GetPluginNameString()));

	if (!Item->IsPluginModule())
	{
		return FText::Format(LOCTEXT("SubsystemItem_Module_Tooltip", "Package: {Module}"), Args);
	}
	else
	{
		return FText::Format(LOCTEXT("SubsystemItem_ModuleInPlugin_Tooltip", "Package: {Module}\nPlugin: {Plugin}"), Args);
	}
}

void FSubsystemDynamicColumn_Module::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	OutSearchStrings.Add(Item.GetShortPackageString());
}

#undef LOCTEXT_NAMESPACE
