// Copyright 2022, Aquanox.

#include "Model/Column/SubsystemBrowserColumn_Name.h"

#include "UI/SubsystemTableItem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn_Name::FSubsystemDynamicColumn_Name()
{
	Name = TEXT("Name");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Name", "Name");
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Name", "Name");
	SortOrder = -1;
	PreferredWidthRatio = 0.6f;
}

TSharedPtr<SWidget> FSubsystemDynamicColumn_Name::GenerateColumnWidget(TSharedRef<const ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const
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

FText FSubsystemDynamicColumn_Name::ExtractText(TSharedRef<const ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("DisplayText"), Item->GetDisplayName());
	Args.Add(TEXT("Stale"), (Item->IsStale() ? LOCTEXT("SubsystemItem_Stale", " (Stale)") : FText::GetEmpty()));
	return FText::Format(LOCTEXT("SubsystemItem_Name", "{DisplayText}{Stale}"), Args);
}

FSlateColor FSubsystemDynamicColumn_Name::ExtractColor(TSharedRef<const ISubsystemTreeItem> Item) const
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
