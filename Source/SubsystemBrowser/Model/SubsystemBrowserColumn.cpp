// Copyright 2022, Aquanox.

#include "SubsystemBrowserModule.h"
#include "Model/SubsystemBrowserColumn.h"

#include "SubsystemBrowserDescriptor.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

#if ENABLE_SUBSYSTEM_BROWSER_CUSTOM_COLUMNS

const TArray<SubsystemColumnPtr>& FSubsystemBrowserModule::GetCustomDynamicColumns() const
{
	return CustomDynamicColumns;
}

void FSubsystemBrowserModule::RegisterDynamicColumn(TSharedRef<FSubsystemDynamicColumn> InColumn)
{
	CustomDynamicColumns.Add(InColumn);
}

#endif

bool SubsystemColumns::IsDefaultColumn(FName InName)
{
	return InName == SubsystemColumns::ColumnID_Marker
		|| InName == SubsystemColumns::ColumnID_Name
		|| InName == SubsystemColumns::ColumnID_Module
		|| InName == SubsystemColumns::ColumnID_Config
		|| IsDynamicColumn(InName);
}

bool SubsystemColumns::IsDynamicColumn(FName InName)
{
	return InName == SubsystemColumns::ColumnID_DynamicSlot0
		|| InName == SubsystemColumns::ColumnID_DynamicSlot1
		|| InName == SubsystemColumns::ColumnID_DynamicSlot2
		|| InName == SubsystemColumns::ColumnID_DynamicSlot3
		|| InName == SubsystemColumns::ColumnID_DynamicSlot4;
}

int32 SubsystemColumns::GetDynamicColumnIndex(FName InName)
{
	if (InName == SubsystemColumns::ColumnID_DynamicSlot0) { return 0; }
	if (InName == SubsystemColumns::ColumnID_DynamicSlot1) { return 1; }
	if (InName == SubsystemColumns::ColumnID_DynamicSlot2) { return 2; }
	if (InName == SubsystemColumns::ColumnID_DynamicSlot3) { return 3; }
	if (InName == SubsystemColumns::ColumnID_DynamicSlot4) { return 4; }

	checkNoEntry();
	return -1;
}

FSlateColor FSubsystemDynamicColumn::GetColumnColor(ISubsystemTreeItem& InItem, bool bSelected) const
{
	return FSlateColor::UseForeground();
}

TSharedPtr<SWidget> FSubsystemDynamicColumn::GetColumnWidget(ISubsystemTreeItem& InItem) const
{
	return SNullWidget::NullWidget;
}

FSubsystemDynamicColumn_Module::FSubsystemDynamicColumn_Module()
{
	Name = SubsystemColumns::ColumnID_Module;
	Label = LOCTEXT("SubsystemBrowser_Column_Package", "Module");
}

FText FSubsystemDynamicColumn_Module::GetColumnText(ISubsystemTreeItem& InItem) const
{
	return FText::FromString(InItem.GetPackageString());
}

FSlateColor FSubsystemDynamicColumn_Module::GetColumnColor(ISubsystemTreeItem& InItem, bool bSelected) const
{
	if (InItem.IsGameModule() && !bSelected)
	{
		return FLinearColor(0.4f, 0.4f, 1.0f);
	}
	return FSlateColor::UseForeground();
}

FSubsystemDynamicColumn_Config::FSubsystemDynamicColumn_Config()
{
	Name = SubsystemColumns::ColumnID_Config;
	Label = LOCTEXT("SubsystemBrowser_Column_ConfigClass", "Config");
}

FText FSubsystemDynamicColumn_Config::GetColumnText(ISubsystemTreeItem& InItem) const
{
	return FText::FromString(InItem.GetConfigNameString());
}

#undef LOCTEXT_NAMESPACE
