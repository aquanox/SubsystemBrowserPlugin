// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserModule.h"
#include "Model/SubsystemBrowserDescriptor.h"
#include "Model/SubsystemBrowserColumn_Name.h"
#include "Model/SubsystemBrowserColumn_Config.h"
#include "Model/SubsystemBrowserColumn_Module.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

void FSubsystemBrowserModule::RegisterDefaultDynamicColumns()
{
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Module>());
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Config>());
}

FSubsystemDynamicColumn::FSubsystemDynamicColumn()
{
}

SHeaderRow::FColumn::FArguments FSubsystemDynamicColumn::GenerateHeaderColumnWidget() const
{
	return SHeaderRow::Column( Name )
			  .SortMode(EColumnSortMode::None)
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
	return !InName.IsNone() && InName != SubsystemColumns::ColumnID_Name;
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
			.Font(this, &FSubsystemDynamicTextColumn::ExtractFont, TableRow)
			.ColorAndOpacity(this, &FSubsystemDynamicTextColumn::ExtractColor, TableRow)
			.Text(this, &FSubsystemDynamicTextColumn::ExtractText, Item)
			.ToolTipText(this, &FSubsystemDynamicTextColumn::ExtractTooltipText, Item)
			.HighlightText(TableRow->HighlightText)
		];
}

FSlateColor FSubsystemDynamicTextColumn::ExtractColor(TSharedRef<SSubsystemTableItem> TableRow) const
{
	return TableRow->GetDefaultColorAndOpacity();
}

FSlateFontInfo FSubsystemDynamicTextColumn::ExtractFont(TSharedRef<SSubsystemTableItem> TableRow) const
{
	return TableRow->GetDefaultFont();
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