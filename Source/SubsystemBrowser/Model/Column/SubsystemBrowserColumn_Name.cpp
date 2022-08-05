// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Name.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Name::FSubsystemDynamicColumn_Name()
{
	Name = TEXT("Name");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Name", "Name");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Name", "Name");
	SortOrder = -1;
	PreferredWidthRatio = 0.6f;
}

TSharedPtr<SWidget> FSubsystemDynamicColumn_Name::GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const
{
	return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SExpanderArrow, TableRow)
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
		    .Padding(1, 0, 0, 0)
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				.HeightOverride(22)
				.WidthOverride(Item->CanHaveChildren() ? 16.f : 7.f)
				[
					SNew(SImage)
					.Image(TableRow, &SSubsystemTableItem::GetItemIconBrush)
				]
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(4, 3, 0, 3)
			.AutoWidth()
			[
				SNew(STextBlock)
					.Font(this, &FSubsystemDynamicColumn_Name::ExtractFont, Item)
					.ColorAndOpacity(this, &FSubsystemDynamicColumn_Name::ExtractColor, Item)
					.Text(this, &FSubsystemDynamicColumn_Name::ExtractText, Item)
					.ToolTipText(this, &FSubsystemDynamicColumn_Name::ExtractTooltipText, Item)
					.HighlightText(TableRow->HighlightText)
			];
}

FText FSubsystemDynamicColumn_Name::ExtractText(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("DisplayText"), Item->GetDisplayName());
		Args.Add(TEXT("Stale"), (Item->IsStale() ? LOCTEXT("SubsystemItem_Stale", " (Stale)") : FText::GetEmpty()));

		if (SubsystemItem->OwnerName.IsEmpty())
		{
			return FText::Format(LOCTEXT("SubsystemItem_Name", "{DisplayText}{Stale}"), Args);
		}
		else
		{
			Args.Add(TEXT("OwnedBy"), FText::FromString(SubsystemItem->OwnerName));
			return FText::Format(LOCTEXT("SubsystemItem_NameOwned", "{DisplayText} ({OwnedBy}){Stale}"), Args);
		}
	}
	else
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("DisplayText"), Item->GetDisplayName());
		return FText::Format(LOCTEXT("SubsystemItem_Name", "{DisplayText}"), Args);
	}
}

FText FSubsystemDynamicColumn_Name::ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (const FSubsystemTreeSubsystemItem* SubsystemItem = Item->GetAsSubsystemDescriptor())
	{
		auto ToText = [](const FString& InString)
		{
			return InString.IsEmpty() ? INVTEXT("None") : FText::FromString(InString);
		};

		FFormatNamedArguments Args;
		Args.Add(TEXT("Name"), Item->GetDisplayName());
		Args.Add(TEXT("OwnedBy"), ToText(SubsystemItem->OwnerName));
		Args.Add(TEXT("Module"), ToText(SubsystemItem->ShortPackage));
		Args.Add(TEXT("Plugin"), ToText(SubsystemItem->PluginName));

		return FText::Format(LOCTEXT("SubsystemItem_Name_Tooltip", "{Name}\nModule: {Module}\nPlugin: {Plugin}\nOwned by: {OwnedBy}"), Args);
	}
	else
	{
		return ExtractText(Item);
	}
}

FSlateColor FSubsystemDynamicColumn_Name::ExtractColor(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (Item->IsStale())
	{
		return FSlateColor::UseSubduedForeground();
	}
	return Super::ExtractColor(Item);
}

void FSubsystemDynamicColumn_Name::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	OutSearchStrings.Add(Item.GetDisplayName().ToString());
}


#undef LOCTEXT_NAMESPACE
