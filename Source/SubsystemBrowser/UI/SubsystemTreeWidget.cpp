// Copyright 2022, Aquanox.

#include "UI/SubsystemTreeWidget.h"
#include "Model/SubsystemBrowserModel.h"
#include "UI/SubsystemBrowserPanel.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

void SSubsystemsTreeWidget::Construct(const FArguments& InArgs, const TSharedPtr<FSubsystemModel>& InModel, const TSharedPtr<SSubsystemBrowserPanel>& InBrowser)
{
	STreeView<SubsystemTreeItemPtr>::Construct(InArgs);

	Model = InModel;
	Browser = InBrowser;
}

#undef LOCTEXT_NAMESPACE
