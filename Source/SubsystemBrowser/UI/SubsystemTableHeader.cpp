// Copyright 2022, Aquanox.

#include "UI/SubsystemTableHeader.h"
#include "UI/SubsystemBrowserPanel.h"
#include "Model/SubsystemBrowserModel.h"
#include "Model/SubsystemBrowserColumn.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

void SSubsystemsHeaderRow::Construct(const FArguments& InArgs, const TSharedPtr<FSubsystemModel>& InModel, const TSharedPtr<SSubsystemBrowserPanel>& InBrowser)
{
	Model = InModel;
	Browser = InBrowser;

	SHeaderRow::Construct(InArgs);

	RebuildColumns();
}

void SSubsystemsHeaderRow::RebuildColumns()
{
	ClearColumns();

	// Subsystem name column
	AddColumn(
		SSubsystemsHeaderRow::Column(SubsystemColumns::ColumnID_Name)
		  .FillWidth(0.60f)
		  .SortMode(EColumnSortMode::None)
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
					.Text(LOCTEXT("SubsystemBrowser_Column_Name", "Name"))
				]
			]
		]);

	for (const auto& Column : Model->GetSelectedDynamicColumns())
	{
		AddColumn(Column->GenerateHeaderColumnWidget());
	}

	RefreshColumns();
}

#undef LOCTEXT_NAMESPACE
