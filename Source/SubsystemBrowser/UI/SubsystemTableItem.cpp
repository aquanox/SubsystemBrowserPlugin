// Copyright 2022, Aquanox.

#include "UI/SubsystemTableItem.h"
#include "Model/SubsystemBrowserModel.h"
#include "UI/SubsystemBrowserPanel.h"
#include "SlateOptMacros.h"
#include "EditorStyleSet.h"
#include "SubsystemBrowserSettings.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

void SSubsystemTableItem::Construct(const FArguments& InArgs, TSharedRef<STableViewBase> OwnerTableView)
{
	Model = InArgs._InModel;
	Item = InArgs._InItemModel;
	Browser = InArgs._InBrowser;
	IsItemExpanded = InArgs._IsItemExpanded;
	HighlightText = InArgs._HighlightText;

	FSuperRowType::FArguments Args = FSuperRowType::FArguments();
	SMultiColumnTableRow<SubsystemTreeItemPtr>::Construct(Args, OwnerTableView);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> SSubsystemTableItem::GenerateWidgetForColumn(const FName& ColumnID)
{
	if (!Browser->IsColumnVisible(ColumnID) || !Item.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	TSharedPtr< SWidget > TableRowContent = SNullWidget::NullWidget;

	if (ColumnID == SubsystemColumns::ColumnID_Name)
	{
		TableRowContent =
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SExpanderArrow, SharedThis(this))
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
		    .Padding(1, 0, 0, 0)
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				.HeightOverride(22)
				.WidthOverride(Item->CanHaveChildren() ? 16.f : 7.f)
				[
					SNew(SImage)
					.Image(this, &SSubsystemTableItem::GetItemIconBrush)
				]
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
		    .Padding(4, 3, 0, 3)
			.AutoWidth()
			[
				SNew(STextBlock)
					.Font(this, &SSubsystemTableItem::GetDisplayNameFont)
					.Text(this, &SSubsystemTableItem::GetDisplayNameText)
					.ToolTipText(this, &SSubsystemTableItem::GetDisplayNameTooltipText)
					.ColorAndOpacity(this, &SSubsystemTableItem::GetDisplayNameColorAndOpacity)
					.HighlightText(HighlightText)
			];
	}
	else if (ColumnID == SubsystemColumns::ColumnID_Package)
	{
		TableRowContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
		    .Padding(1, 0, 0, 0)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Font(this, &SSubsystemTableItem::GetDisplayNameFont)
				.Text(this, &SSubsystemTableItem::GetPackageText)
				.ToolTipText(this, &SSubsystemTableItem::GetPackageTooltipText)
				.ColorAndOpacity(this, &SSubsystemTableItem::GetDisplayNameColorAndOpacity)
				.HighlightText(HighlightText)
			];
	}
	else if (ColumnID == SubsystemColumns::ColumnID_ConfigClass)
	{
		TableRowContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(1, 0, 0, 0)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Font(this, &SSubsystemTableItem::GetDisplayNameFont)
				.Text(this, &SSubsystemTableItem::GetConfigClassText)
				.ToolTipText(this, &SSubsystemTableItem::GetConfigClassText)
				.ColorAndOpacity(this, &SSubsystemTableItem::GetDisplayNameColorAndOpacity)
				.HighlightText(HighlightText)
			];
	}

	return TableRowContent.ToSharedRef();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

const FSlateBrush* SSubsystemTableItem::GetItemIconBrush() const
{
#if ENGINE_MAJOR_VERSION < 5
	static const FName FolderOpenName(TEXT("WorldBrowser.FolderOpen"));
	static const FName FolderClosedName(TEXT("WorldBrowser.FolderClosed"));
#else
	static const FName FolderOpenName(TEXT("Icons.FolderOpen"));
	static const FName FolderClosedName(TEXT("Icons.FolderClosed"));
#endif

	if (Item.IsValid() && Item->CanHaveChildren())
	{
		if (Item->GetNumChildren() > 0 && Item->bExpanded)
		{
			return FEditorStyle::GetBrush(FolderOpenName);
		}

		return FEditorStyle::GetBrush(FolderClosedName);
	}

	return nullptr;
}

FSlateColor SSubsystemTableItem::GetDisplayNameColorAndOpacity() const
{
	if (USubsystemBrowserSettings::Get()->IsColoringEnabled() && !Browser->IsItemSelected(Item))
	{
		if (Item->IsGameModule())
		{
			return FLinearColor(0.4f, 0.4f, 1.0f);
		}
	}

	return FSlateColor::UseForeground();
}

FSlateFontInfo SSubsystemTableItem::GetDisplayNameFont() const
{
	// if (Item->GetAsSubsystemDescriptor() && Browser->IsItemSelected(Item))
	// {
		// return FEditorStyle::GetFontStyle("WorldBrowser.LabelFontBold");
	// }
	// else
	// {
		return FEditorStyle::GetFontStyle("WorldBrowser.LabelFont");
	// }
}

FText SSubsystemTableItem::GetDisplayNameText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("DisplayText"), FText::FromString(Item->GetDisplayNameString()));
	Args.Add(TEXT("Stale"), (Item->IsStale() ? LOCTEXT("SubsystemItem_Stale", " (Stale)") : FText::GetEmpty()));

	if (Item->GetOwnerNameString().IsEmpty())
	{
		return FText::Format(LOCTEXT("SubsystemItem_Name", "{DisplayText}{Stale}"), Args);
	}
	else
	{
		Args.Add(TEXT("OwnedBy"), FText::FromString(Item->GetOwnerNameString()));
		return FText::Format(LOCTEXT("SubsystemItem_NameOwned", "{DisplayText} ({OwnedBy}){Stale}"), Args);
	}
}

FText SSubsystemTableItem::GetDisplayNameTooltipText() const
{
	return GetDisplayNameText();
}

FText SSubsystemTableItem::GetClassText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Class"), FText::FromString(Item->GetClassNameString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Class", "{Class}"), Args);
}

FText SSubsystemTableItem::GetPackageText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Package"), FText::FromString(Item->GetShortPackageString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Package", "{Package}"), Args);
}

FText SSubsystemTableItem::GetPackageTooltipText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Package"), FText::FromString(Item->GetPackageString()));
	return FText::Format(LOCTEXT("SubsystemItemType_Package_Tooltip", "{Package}"), Args);
}

FText SSubsystemTableItem::GetConfigClassText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("ConfigClass"), FText::FromString(Item->GetConfigClassNameString()));
	return FText::Format(LOCTEXT("SubsystemItemType_ConfigClass", "{ConfigClass}"), Args);
}

#undef LOCTEXT_NAMESPACE
