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

	for (const SubsystemColumnPtr& Column : Model->GetSelectedTableColumns())
	{
		AddColumn(Column->GenerateHeaderColumnWidget());
	}

	RefreshColumns();
}

#undef LOCTEXT_NAMESPACE
