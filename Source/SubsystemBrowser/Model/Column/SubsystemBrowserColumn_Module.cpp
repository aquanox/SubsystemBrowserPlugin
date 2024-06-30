// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Module.h"

#include "SubsystemBrowserSettings.h"
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
		return FText::FromString(SubsystemItem->ModuleName);
	}

	return FText::GetEmpty();
}

FSlateColor FSubsystemDynamicColumn_Module::ExtractColor(TSharedRef<const ISubsystemTreeItem> Item) const
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	if (Settings->IsColoringEnabled() && !Item->IsStale() && !Item->IsSelected())
	{
		if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
		{
			return USubsystemBrowserSettings::Get()->GetModuleColor(SubsystemItem->IsGameModule());
		}
	}
	return Super::ExtractColor(Item);
}

void FSubsystemDynamicColumn_Module::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item.GetAsSubsystemDescriptor())
	{
		OutSearchStrings.Add(SubsystemItem->ModuleName);
	}
}

#undef LOCTEXT_NAMESPACE
