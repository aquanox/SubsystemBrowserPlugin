// Copyright 2022, Aquanox.

#pragma once

#include "SlateCore.h"
#include "Model/SubsystemDescriptor.h"

class FSubsystemModel;
class SSubsystemBrowserPanel;

class SSubsystemsTreeWidget : public STreeView<SubsystemTreeItemPtr>
{
public:
	void Construct(const FArguments& InArgs, const TSharedPtr<FSubsystemModel>& InModel, const TSharedPtr<SSubsystemBrowserPanel>& InBrowser);

private:
	TSharedPtr<FSubsystemModel> Model;
	TWeakPtr<SSubsystemBrowserPanel> Browser;
};
