// Copyright 2022, Aquanox.

#include "UI/SubsystemTableItem.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserStyle.h"
#include "Model/SubsystemBrowserModel.h"
#include "UI/SubsystemBrowserPanel.h"
#include "UI/SubsystemTableItemTooltip.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

void SSubsystemTableItem::Construct(const FArguments& InArgs, TSharedRef<STableViewBase> OwnerTableView)
{
	Model = InArgs._InModel;
	Item = InArgs._InItemModel;
	Browser = InArgs._InBrowser;
	IsItemExpanded = InArgs._IsItemExpanded;
	HighlightText = InArgs._HighlightText;

	SetToolTip(SNew(SSubsystemTableItemTooltip).SubsystemTableItem(SharedThis(this)));

	FSuperRowType::FArguments Args = FSuperRowType::FArguments();
	Super::Construct(Args, OwnerTableView);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> SSubsystemTableItem::GenerateWidgetForColumn(const FName& ColumnID)
{
	if (!Item.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	TSharedPtr<SWidget> TableRowContent = SNullWidget::NullWidget;

	SubsystemColumnPtr Column = Model->FindTableColumn(ColumnID);
	if (Column.IsValid() && Model->ShouldShowColumn(Column) )
	{
		TableRowContent = Column->GenerateColumnWidget(Item.ToSharedRef(), SharedThis(this));
	}

	return TableRowContent.ToSharedRef();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

const FSlateBrush* SSubsystemTableItem::GetItemIconBrush() const
{
	if (Item.IsValid() && Item->CanHaveChildren())
	{
		if (Item->GetNumChildren() > 0 && Item->bExpanded)
		{
			return FStyleHelper::GetBrush(FSubsystemBrowserStyle::FolderOpenName);
		}
		return FStyleHelper::GetBrush(FSubsystemBrowserStyle::FolderClosedName);
	}
	return nullptr;
}

bool SSubsystemTableItem::IsColoringEnabled() const
{
	return USubsystemBrowserSettings::Get()->IsColoringEnabled();
}

#undef LOCTEXT_NAMESPACE
