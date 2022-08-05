// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"
#include "Model/SubsystemBrowserDescriptor.h"
#include "SubsystemBrowserSorting.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemDynamicColumn::FSubsystemDynamicColumn()
{
}

SHeaderRow::FColumn::FArguments FSubsystemDynamicColumn::GenerateHeaderColumnWidget() const
{
	return SHeaderRow::Column( Name )
			  .FillWidth(PreferredWidthRatio)
			  .HeaderContent()
			[
				SNew(SBox)
				.MinDesiredHeight(24)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(TableLabel)
					]
				]
			];
}

bool FSubsystemDynamicColumn::IsValidColumnName(FName InName)
{
	static const FName ColumnID_Name("Name");
	return !InName.IsNone() && InName != ColumnID_Name;
}

TSharedPtr<SWidget> FSubsystemDynamicTextColumn::GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(1, 0, 0, 0)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(this, &FSubsystemDynamicTextColumn::ExtractFont, Item)
			.ColorAndOpacity(this, &FSubsystemDynamicTextColumn::ExtractColorIfEnabled, Item)
			.Text(this, &FSubsystemDynamicTextColumn::ExtractText, Item)
			.ToolTipText(this, &FSubsystemDynamicTextColumn::ExtractTooltipText, Item)
			.HighlightText(TableRow->HighlightText)
		];
}

FText FSubsystemDynamicTextColumn::ExtractTooltipText(TSharedRef<ISubsystemTreeItem> Item) const
{
	return ExtractText(Item);
}

FSlateColor FSubsystemDynamicTextColumn::ExtractColor(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (Item->IsStale())
	{
		return FSlateColor::UseSubduedForeground();
	}
	if (Item->IsGameModule() && !Item->IsSelected())
	{
		return FLinearColor(0.4f, 0.4f, 1.0f);
	}

	return FSlateColor::UseForeground();
}

FSlateColor FSubsystemDynamicTextColumn::ExtractColorIfEnabled(TSharedRef<ISubsystemTreeItem> Item) const
{
	if (USubsystemBrowserSettings::Get()->IsColoringEnabled())
	{
		return ExtractColor(Item);
	}
	return Item->IsStale() ? FSlateColor::UseSubduedForeground() : FSlateColor::UseForeground();
}

FSlateFontInfo FSubsystemDynamicTextColumn::ExtractFont(TSharedRef<ISubsystemTreeItem> Item) const
{
	return FEditorStyle::GetFontStyle("WorldBrowser.LabelFont");
}

void FSubsystemDynamicTextColumn::SortItems(TArray<SubsystemTreeItemPtr>& RootItems, const EColumnSortMode::Type SortMode) const
{
	SubsystemBrowser::FSortHelper<SubsystemTreeItemPtr, FString>()
		.Primary([this](TSharedPtr<ISubsystemTreeItem> Item) { return ExtractText(Item.ToSharedRef()).ToString(); }, SortMode)
		.Sort(RootItems);
}

#undef LOCTEXT_NAMESPACE

#if ENABLE_SUBSYSTEM_BROWSER_EXAMPLES && SB_UE_VERSION_NEWER_OR_SAME(4, 27, 0)

// 1. Create a new struct inheriting FSubsystemDynamicColumn

struct FSubsystemDynamicColumn_Tick : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Tick()
	{
		// Configure name and header title
		Name = TEXT("IsTickable");
		TableLabel = INVTEXT("");
		ConfigLabel = INVTEXT("Tickable");
		PreferredWidthRatio = 0.05f;
	}

	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<struct ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const override
	{
		// Build a widget to represent value
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FEditorStyle::GetBrush(TEXT("GraphEditor.Conduit_16x")))
				.DesiredSizeOverride(FVector2d{16,16})
				.Visibility(this, &FSubsystemDynamicColumn_Tick::ExtractIsTickable, TableRow->Item)
			];
	}
private:
	EVisibility ExtractIsTickable(TSharedPtr<ISubsystemTreeItem> Item) const
	{
		auto* Subsystem = Item->GetAsSubsystemDescriptor();
		return Subsystem && Subsystem->Class.IsValid() && Subsystem->Class->IsChildOf(UTickableWorldSubsystem::StaticClass())
			? EVisibility::Visible : EVisibility::Hidden;
	}
};

// 2. Call this during module initialization
void RegisterCustomColumns()
{
	// Get a reference to Subsystem Browser module instance or load it
	FSubsystemBrowserModule& Module = FModuleManager::LoadModuleChecked<FSubsystemBrowserModule>(TEXT("SubsystemBrowser"));
	// Construct and register new column
	Module.RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Tick>());
}

#endif
