// Copyright 2022, Aquanox.

#pragma once

#include "Widgets/Views/SHeaderRow.h"

class FSubsystemModel;
class SSubsystemBrowserPanel;

class SSubsystemsHeaderRow : public SHeaderRow
{
public:
	void Construct(const FArguments& InArgs, const TSharedPtr<FSubsystemModel>& InModel, const TSharedPtr<SSubsystemBrowserPanel>& InBrowser);
	void RebuildColumns();

private:
	TSharedPtr<FSubsystemModel> Model;
	TWeakPtr<SSubsystemBrowserPanel> Browser;
};
