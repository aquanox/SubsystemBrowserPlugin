// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn_Name.h"
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
					.Font(this, &FSubsystemDynamicColumn_Name::ExtractFont, TableRow)
					.ColorAndOpacity(this, &FSubsystemDynamicColumn_Name::ExtractColor, TableRow)
					.Text(this, &FSubsystemDynamicColumn_Name::ExtractText, Item)
					.ToolTipText(this, &FSubsystemDynamicColumn_Name::ExtractTooltipText, Item)
					.HighlightText(TableRow->HighlightText)
			];
}

void FSubsystemDynamicColumn_Name::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	OutSearchStrings.Add(Item.GetDisplayNameString());
}

FText FSubsystemDynamicColumn_Name::ExtractText(TSharedRef<ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("DisplayText"), Item->GetDisplayName());
	Args.Add(TEXT("Stale"), (Item->IsStale() ? LOCTEXT("SubsystemItem_Stale", " (Stale)") : FText::GetEmpty()));
	Args.Add(TEXT("Module"), FText::FromString(Item->GetShortPackageString()));
	Args.Add(TEXT("Plugin"), FText::FromString(Item->GetPluginNameString()));

	FString OwnerNameString = Item->GetOwnerNameString();
	if (OwnerNameString.IsEmpty())
	{
		return FText::Format(LOCTEXT("SubsystemItem_Name", "{DisplayText}{Stale}"), Args);
	}
	else
	{
		Args.Add(TEXT("OwnedBy"), FText::FromString(OwnerNameString));
		return FText::Format(LOCTEXT("SubsystemItem_NameOwned", "{DisplayText} ({OwnedBy}){Stale}"), Args);
	}
}

FText FSubsystemDynamicColumn_Name::ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const
{
	return ExtractText(Item);
}


#undef LOCTEXT_NAMESPACE
