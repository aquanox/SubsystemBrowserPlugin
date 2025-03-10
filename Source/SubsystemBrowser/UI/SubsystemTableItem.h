// Copyright 2022, Aquanox.

#pragma once

#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h" // NoPCH
#include "Model/SubsystemBrowserDescriptor.h"

class FSubsystemModel;
class SSubsystemBrowserPanel;

class SSubsystemTableItem : public SMultiColumnTableRow<SubsystemTreeItemPtr>
{
	using Super = SMultiColumnTableRow<SubsystemTreeItemPtr>;
public:
	SLATE_BEGIN_ARGS( SSubsystemTableItem )
		: _IsItemExpanded( false )
	{}
		/** Data for the world */
		SLATE_ARGUMENT(TSharedPtr<FSubsystemModel>, InModel)

		/** Item model this widget represents */
		SLATE_ARGUMENT(SubsystemTreeItemPtr, InItemModel)

		/** The hierarchy that this item belongs to */
		SLATE_ARGUMENT(TSharedPtr<SSubsystemBrowserPanel>, InBrowser)

		/** True when this item has children and is expanded */
		SLATE_ATTRIBUTE(bool, IsItemExpanded)

		/** The string in the title to highlight */
		SLATE_ATTRIBUTE(FText, HighlightText)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedRef<STableViewBase> OwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override;

public:
	TSharedPtr<FSubsystemModel>			Model;
	SubsystemTreeItemPtr				Item;
	TSharedPtr<SSubsystemBrowserPanel>	Browser;

	TAttribute<FText>					HighlightText;
	TAttribute<bool>					IsItemExpanded;
};
