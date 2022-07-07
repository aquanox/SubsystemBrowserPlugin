// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserDescriptor.h"
#include "UI/SubsystemTableItem.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const TArray<SubsystemColumnPtr>& FSubsystemBrowserModule::GetDynamicColumns() const
{
	return DynamicColumns;
}

void FSubsystemBrowserModule::RegisterDefaultDynamicColumns()
{
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Module>());
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Config>());
}

void FSubsystemBrowserModule::RegisterDynamicColumn(TSharedRef<FSubsystemDynamicColumn> InColumn)
{
	DynamicColumns.Add(InColumn);

	// Sort columns by order
	DynamicColumns.StableSort(SubsystemColumnSorter());
}

TSharedPtr<SWidget> FSubsystemDynamicColumn::GenerateColumnWidget(TSharedRef<class SSubsystemTableItem> TableRow) const
{
	return SNullWidget::NullWidget;
}

FSubsystemDynamicColumn_Module::FSubsystemDynamicColumn_Module()
{
	Name = SubsystemColumns::ColumnID_Module;
	Label = LOCTEXT("SubsystemBrowser_Column_Package", "Module");
}

TSharedPtr<SWidget> FSubsystemDynamicColumn_Module::GenerateColumnWidget(TSharedRef<class SSubsystemTableItem> TableRow) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
	    .Padding(1, 0, 0, 0)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(TableRow, &SSubsystemTableItem::GetDefaultFont)
			.Text(this, &FSubsystemDynamicColumn_Module::ExtractModuleText, TableRow->Item)
			.ToolTipText(this, &FSubsystemDynamicColumn_Module::ExtractModuleTooltipText, TableRow->Item)
			.ColorAndOpacity(TableRow, &SSubsystemTableItem::GetDefaultColorAndOpacity)
			.HighlightText(TableRow->HighlightText)
		];
}

FText FSubsystemDynamicColumn_Module::ExtractModuleText(TSharedPtr<ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Package"), FText::FromString(Item->GetShortPackageString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Package", "{Package}"), Args);
}

FText FSubsystemDynamicColumn_Module::ExtractModuleTooltipText(TSharedPtr<ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Package"), FText::FromString(Item->GetPackageString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Package_Tooltip", "{Package}"), Args);
}

FSubsystemDynamicColumn_Config::FSubsystemDynamicColumn_Config()
{
	Name = SubsystemColumns::ColumnID_Config;
	Label = LOCTEXT("SubsystemBrowser_Column_ConfigClass", "Config");
}

TSharedPtr<SWidget> FSubsystemDynamicColumn_Config::GenerateColumnWidget(TSharedRef<SSubsystemTableItem> TableRow) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(1, 0, 0, 0)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(TableRow, &SSubsystemTableItem::GetDefaultFont)
			.Text(this, &FSubsystemDynamicColumn_Config::ExtractConfigText, TableRow->Item)
			.ToolTipText(this, &FSubsystemDynamicColumn_Config::ExtractConfigText, TableRow->Item)
			.ColorAndOpacity(TableRow, &SSubsystemTableItem::GetDefaultColorAndOpacity)
			.HighlightText(TableRow->HighlightText)
		];
}

FText FSubsystemDynamicColumn_Config::ExtractConfigText(TSharedPtr<ISubsystemTreeItem> Item) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("ConfigClass"), FText::FromString(Item->GetConfigNameString()));
	return FText::Format(LOCTEXT("SubsystemItemType_ConfigClass", "{ConfigClass}"), Args);
}

#undef LOCTEXT_NAMESPACE

#if ENABLE_SUBSYSTEM_BROWSER_EXAMPLES

// 1. Create a new struct inheriting FSubsystemDynamicColumn

struct FSubsystemDynamicColumn_Tick : public FSubsystemDynamicColumn
{
	FSubsystemDynamicColumn_Tick()
	{
		// Configure name and header title
		Name = TEXT("IsTickable");
		Label = INVTEXT("Ticks?");
	}

	virtual TSharedPtr<SWidget> GenerateColumnWidget(TSharedRef<SSubsystemTableItem> TableRow) const override
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