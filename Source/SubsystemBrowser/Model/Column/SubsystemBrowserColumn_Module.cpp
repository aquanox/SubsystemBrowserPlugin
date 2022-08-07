// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Module.h"

#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Module::FSubsystemDynamicColumn_Module()
{
	Name = TEXT("Module");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Module", "Module");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Module", "Module");
	PreferredWidthRatio = 0.25f;
}

FText FSubsystemDynamicColumn_Module::ExtractText(TSharedRef<const ISubsystemTreeItem> Item) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
	{
		return FText::FromString(SubsystemItem->ShortPackage);
	}

	return FText::GetEmpty();
}

void FSubsystemDynamicColumn_Module::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item.GetAsSubsystemDescriptor())
	{
		OutSearchStrings.Add(SubsystemItem->ShortPackage);
	}
}

#undef LOCTEXT_NAMESPACE
