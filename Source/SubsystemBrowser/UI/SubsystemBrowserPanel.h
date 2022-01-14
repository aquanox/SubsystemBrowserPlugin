// Copyright 2022, Aquanox.

#pragma once

#include "SKismetInspector.h"
#include "SlateCore.h"
#include "SlateBasics.h"
#include "UI/SubsystemTreeWidget.h"
#include "UI/SubsystemTableItem.h"
#include "Model/SubsystemBrowserModel.h"

/**
 * Subsystem browser tab content widget
 */
class SSubsystemBrowserPanel : public SCompoundWidget
{
public:
	static const FName PanelIconName;

	SLATE_BEGIN_ARGS(SSubsystemBrowserPanel)
		:_InWorld(nullptr)
		{}
		SLATE_ARGUMENT(UWorld*, InWorld)
	SLATE_END_ARGS()

	SSubsystemBrowserPanel();
	~SSubsystemBrowserPanel();

	void Construct(const FArguments& InArgs);

	bool IsItemSelected(SubsystemTreeItemPtr Item);
	bool IsColumnVisible(FName ColumnName) const { return GetColumnDisplayStatus(ColumnName); }

protected:
	virtual void Tick( const FGeometry& AllotedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	void Populate();
	void EmptyTreeItems();

	void RefreshView() { bNeedsRefresh = true;}
	void RefreshDetails() { bNeedRefreshDetails = true; }
	void RefreshColumns() { bNeedsRefresh = true; bNeedListRebuild = true;  }
	void FullRefresh();

	// Search bar

	void TransformItemToString(const ISubsystemTreeItem& Level, TArray<FString>& OutSearchStrings) const;
	void SetFilterText(const FText& InFilterText);
	FText GetSearchBoxText() const;
	FText GetFilterStatusText() const;
	FSlateColor GetFilterStatusTextColor() const;

	// View options panel

	void BrowserSplitterFinishedResizing();

	FSlateColor GetViewOptionsButtonForegroundColor() const;
	TSharedRef<SWidget> GetViewOptionsButtonContent();

	void ToggleDisplayCategory(FSubsystemTreeItemID InCategory);
	bool GetCategoryDisplayStatus(FSubsystemTreeItemID InCategory);

	// Tree view

	TSharedRef<ITableRow> GenerateTreeRow(SubsystemTreeItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void GetChildrenForTree(SubsystemTreeItemPtr Item, TArray<SubsystemTreeItemPtr>& OutChildren);
	void OnExpansionChanged(SubsystemTreeItemPtr Item, bool bIsItemExpanded);
	void OnSelectionChanged(const SubsystemTreeItemPtr Item, ESelectInfo::Type SelectInfo);
	void OnTreeViewMouseButtonDoubleClick(SubsystemTreeItemPtr Item);

	EVisibility GetVisibilityForColumn(FName ColumnName) const;
	void ToggleDisplayColumn(FName ColumnName);
	bool GetColumnDisplayStatus(FName ColumnName) const;

	bool IsTableColoringEnabled() const;
	void ToggleTableColoring();

	bool ShouldShowHiddenProperties() const;
	void ToggleShowHiddenProperties();

	bool ShouldShowOnlyGame() const;
	void ToggleShouldShowOnlyGame();

	// Selection and Expansion

	TMap<FSubsystemTreeItemID, bool> GetParentsExpansionState() const;
	void SetParentsExpansionState(const TMap<FSubsystemTreeItemID, bool>& ExpansionInfo);

	SubsystemTreeItemPtr GetFirstSelectedItem() const;
	const FSubsystemTreeSubsystemItem* GetFirstSelectedSubsystem() const;
	FSubsystemTreeItemID GetFirstSelectedItemId() const;

	// World picker

	const FSlateBrush* GetWorldssMenuBrush() const;
	FText GetCurrentWorldText() const;
	FText GetWorldDescription(UWorld* World) const;
	void OnSelectWorld(TWeakObjectPtr<UWorld> InWorld);
	bool IsWorldChecked(TWeakObjectPtr<UWorld> InWorld);
	TSharedRef<SWidget> GetWorldsButtonContent();

	void HandlePIEStart(const bool bIsSimulating);
	void HandlePIEEnd(const bool bIsSimulating);

	// Details

	void SetSelectedObject(SubsystemTreeItemPtr Item);
	void ResetSelectedObject();

	bool IsPropertyReadOnly(const FPropertyAndParent& PropertyAndParent) const;
	bool IsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;
	bool IsPropertyEditingEnabled() const { return true; }

	// Item context menu

	TSharedPtr<SWidget> ConstructSubsystemContextMenu();
	void ContextMenu_Empty() const { }
	void ContextMenu_OpenSourceFile() const;
	void ContextMenu_CopySourceFilePath() const;
	void ContextMenu_CopyClassName() const;
	void ContextMenu_CopyPackageName() const;
	void ContextMenu_CopyScriptName() const;
	void ContextMenu_ConfigExport(bool bModifiedOnly) const;

	bool CanExportSelection() const;
	bool HasSelectedSubsystem() const;

	// Settings

	void OnSettingsChanged(FName InPropertyName);

private:
	TSharedPtr<FSubsystemModel> SubsystemModel;

	TSharedPtr<IDetailsView>	DetailsView;
	TSharedPtr<IDetailsView>	WorldDetailsView;
	TSharedPtr<SVerticalBox>	VerticalBox;
	TSharedPtr<SBorder>			VerticalBoxBorder;
	TSharedPtr<SSplitter>		BrowserSplitter;

	TSharedPtr<SComboButton>    ViewOptionsComboButton;

	TSharedPtr<SubsystemTextFilter> SearchBoxSubsystemFilter;
	TSharedPtr<SubsystemCategoryFilter> CategoryFilter;
	int32 FilteredSubsystemsCount = 0;

	TSharedPtr<SHeaderRow>			HeaderRowWidget;

	/** Root items for the tree widget */
	TArray<SubsystemTreeItemPtr> RootTreeItems;

	/** All items that are currently displayed in the tree widget */
	TMap<FSubsystemTreeItemID, SubsystemTreeItemPtr> TreeItemMap;

	TSharedPtr<SSubsystemsTreeWidget> TreeWidget;

	bool bIsReentrant = false;
	bool bFullRefresh = true;
	bool bNeedsRefresh = true; // needs initial update
	bool bNeedRefreshDetails = false;
	bool bUpdatingSelection = false;
	bool bLoadedExpansionSettings = false;
	bool bNeedListRebuild = true; // needs initial header update to upply config
};



