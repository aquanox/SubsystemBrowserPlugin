// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserColumn.h"
#include "SubsystemBrowserFlags.h"
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
	if (InColumn->Name.IsNone()
		|| InColumn->Name == SubsystemColumns::ColumnID_Name)
	{
		UE_LOG(LogSubsystemBrowser, Error, TEXT("Invalid column being registered"));
		return;
	}

	for (auto& Column : DynamicColumns)
	{
		if (Column->Name == InColumn->Name)
		{
			UE_LOG(LogSubsystemBrowser, Error, TEXT("Duplicating column with name %s."), *Column->Name.ToString());
			return;
		}
	}

	DynamicColumns.Add(InColumn);

	// Sort columns by order
	DynamicColumns.StableSort(SubsystemColumnSorter());
}

FSubsystemDynamicColumn::FSubsystemDynamicColumn()
{
}

SHeaderRow::FColumn::FArguments FSubsystemDynamicColumn::GenerateHeaderColumnWidget() const
{
	return SHeaderRow::Column( Name )
			  .SortMode(EColumnSortMode::None)
			  .FillWidth( PreferredWidthRatio )
			  .HeaderContent()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(TableLabel)
				]
			];
}

FSubsystemDynamicColumn_Module::FSubsystemDynamicColumn_Module()
{
	Name = SubsystemColumns::ColumnID_Module;
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Module", "Module");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Module", "Module");
	PreferredWidthRatio = 0.25f;
}

TSharedPtr<SWidget> FSubsystemDynamicColumn_Module::GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
	    .Padding(1, 0, 0, 0)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(TableRow, &SSubsystemTableItem::GetDefaultFont)
			.Text(this, &FSubsystemDynamicColumn_Module::ExtractModuleText, TableRow)
			.ToolTipText(this, &FSubsystemDynamicColumn_Module::ExtractModuleTooltipText, TableRow)
			.ColorAndOpacity(TableRow, &SSubsystemTableItem::GetDefaultColorAndOpacity)
			.HighlightText(TableRow->HighlightText)
		];
}

void FSubsystemDynamicColumn_Module::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	OutSearchStrings.Add(Item.GetShortPackageString());
}

FText FSubsystemDynamicColumn_Module::ExtractModuleText(TSharedRef<SSubsystemTableItem> TableRow) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Module"), FText::FromString(TableRow->Item->GetShortPackageString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Module", "{Module}"), Args);
}

FText FSubsystemDynamicColumn_Module::ExtractModuleTooltipText(TSharedRef<SSubsystemTableItem> TableRow) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Module"), FText::FromString(TableRow->Item->GetPackageString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Module_Tooltip", "{Module}"), Args);
}

FSubsystemDynamicColumn_Config::FSubsystemDynamicColumn_Config()
{
	Name = SubsystemColumns::ColumnID_Config;
	TableLabel = LOCTEXT("SubsystemBrowser_Column_Config", "Config");
	ConfigLabel = LOCTEXT("SubsystemBrowser_Column_Config", "Config");
	PreferredWidthRatio = 0.15f;
}

TSharedPtr<SWidget> FSubsystemDynamicColumn_Config::GenerateColumnWidget(TSharedRef<ISubsystemTreeItem> Item, TSharedRef<SSubsystemTableItem> TableRow) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(1, 0, 0, 0)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(TableRow, &SSubsystemTableItem::GetDefaultFont)
			.Text(this, &FSubsystemDynamicColumn_Config::ExtractConfigText, TableRow)
			.ToolTipText(this, &FSubsystemDynamicColumn_Config::ExtractConfigText, TableRow)
			.ColorAndOpacity(TableRow, &SSubsystemTableItem::GetDefaultColorAndOpacity)
			.HighlightText(TableRow->HighlightText)
		];
}

void FSubsystemDynamicColumn_Config::PopulateSearchStrings(const ISubsystemTreeItem& Item, TArray<FString>& OutSearchStrings) const
{
	OutSearchStrings.Add(Item.GetConfigNameString());
}

FText FSubsystemDynamicColumn_Config::ExtractConfigText(TSharedRef<SSubsystemTableItem> TableRow) const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Config"), FText::FromString(TableRow->Item->GetConfigNameString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Config", "{Config}"), Args);
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