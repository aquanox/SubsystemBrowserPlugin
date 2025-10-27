// Copyright 2022, Aquanox.

#pragma once

#include "CoreFwd.h"
#include "SlateFwd.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "UI/SubsystemTreeWidget.h"
#include "UI/SubsystemTableItem.h"
#include "UI/SubsystemTableHeader.h"
#include "Model/SubsystemBrowserModel.h"

class SComboButton;
struct FPropertyAndParent;
class IDetailsView;
class ITableRow;

/**
 * Subsystem browser tab content widget
 */
class SSubsystemBrowserPanel : public SCompoundWidget
{
	friend class SSubsystemsHeaderRow;
public:
	SLATE_BEGIN_ARGS(SSubsystemBrowserPanel)
		:_InWorld(nullptr)
		{}
		SLATE_ARGUMENT(UWorld*, InWorld)
	SLATE_END_ARGS()

	SSubsystemBrowserPanel();
	virtual ~SSubsystemBrowserPanel();

	void Construct(const FArguments& InArgs);

	bool IsItemSelected(SubsystemTreeItemPtr Item);

protected:
	virtual void Tick( const FGeometry& AllotedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	void Populate();
	void EmptyTreeItems();

	void RefreshView() { bNeedsRefresh = true; }
	void RefreshDetails() { bNeedRefreshDetails = true; }
	void RefreshColumns() { bNeedsRefresh = true; bNeedListRebuild = true; bNeedsColumnRefresh = true; }
	void RequestSort() { bSortDirty = true; }
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
	void BuildColumnPickerContent(FMenuBuilder& MenuBuilder);
	void BuildCategoryPickerContent(FMenuBuilder& MenuBuilder);

	void ToggleDisplayCategory(FSubsystemTreeItemID InCategory);
	bool GetCategoryDisplayStatus(FSubsystemTreeItemID InCategory);

	// Tree view

	void SetupColumns(SHeaderRow& HeaderRow);
	TSharedRef<ITableRow> GenerateTreeRow(SubsystemTreeItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
	void GetChildrenForTree(SubsystemTreeItemPtr Item, TArray<SubsystemTreeItemPtr>& OutChildren);
	void OnExpansionChanged(SubsystemTreeItemPtr Item, bool bIsItemExpanded);
	void OnSelectionChanged(const SubsystemTreeItemPtr Item, ESelectInfo::Type SelectInfo);
	void OnTreeViewMouseButtonDoubleClick(SubsystemTreeItemPtr Item);

	EColumnSortMode::Type GetColumnSortMode(FName ColumnId) const;
	void OnColumnSortModeChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnId, const EColumnSortMode::Type InSortMode);
	void SortItems(TArray<SubsystemTreeItemPtr>& Items) const;

	void ToggleDisplayColumn(FName ColumnName);
	void ToggleTableColoring();
	void ToggleForceHiddenPropertyVisibility();
	void ToggleShowSubobjects();
	void ToggleShouldShowOnlyGame();
	void ToggleShouldShowOnlyPlugins();
	void ToggleShouldShowOnlyViewable();

	void ShowPluginSettingsTab() const;
	void ShowSubsystemSettingsTab() const;

	FReply RequestRefresh();

	// Selection and Expansion

	TMap<FSubsystemTreeItemID, bool> GetParentsExpansionState() const;
	void SetParentsExpansionState(const TMap<FSubsystemTreeItemID, bool>& ExpansionInfo);
	void ResetParentsExpansionState();

	SubsystemTreeItemPtr GetFirstSelectedItem() const;
	const FSubsystemTreeSubsystemItem* GetFirstSelectedSubsystem() const;
	FSubsystemTreeItemID GetFirstSelectedItemId() const;

	// World picker

	const FSlateBrush* GetWorldsMenuBrush() const;
	FText GetCurrentWorldText() const;
	void OnSelectWorld(TWeakObjectPtr<UWorld> InWorld);
	bool IsWorldChecked(TWeakObjectPtr<UWorld> InWorld);
	TSharedRef<SWidget> GetWorldsButtonContent();

	void HandlePIEStart(const bool bIsSimulating);
	void HandlePIEEnd(const bool bIsSimulating);
	void HandleWorldChange(UWorld* InWorld);

	// Details

	TSharedRef<IDetailsView> CreateDetails();
	void RecreateDetails();
	void SetSelectedObject(SubsystemTreeItemPtr Item);
	void ResetSelectedObject();

	bool IsDetailsPropertyReadOnly(const FPropertyAndParent& InProperty) const;
	bool IsDetailsPropertyVisible(const FPropertyAndParent& InProperty) const;

	// Item context menu

	TSharedPtr<SWidget> ConstructSubsystemContextMenu();
	bool HasSelectedSubsystem() const;

	// Settings

	void OnSettingsChanged(FName InPropertyName);

	// Data tracking

	void OnModulesChanged(FName ModuleThatChanged, EModuleChangeReason ReasonForChange);
	void OnSubsystemDataChanged(TSharedRef<ISubsystemTreeItem> Item);

private:
	TSharedPtr<FSubsystemModel> SubsystemModel;

	TSharedPtr<IDetailsView>	DetailsView;
	TSharedPtr<SVerticalBox>	DetailsViewBox;
	TSharedPtr<SVerticalBox>	VerticalBox;
	TSharedPtr<SBorder>			VerticalBoxBorder;
	TSharedPtr<SSplitter>		BrowserSplitter;

	TSharedPtr<SComboButton>    ViewOptionsComboButton;

	TSharedPtr<SubsystemTextFilter> SearchBoxSubsystemFilter;
	TSharedPtr<SubsystemCategoryFilter> CategoryFilter;
	int32 FilteredSubsystemsCount = 0;

	TSharedPtr<SSubsystemsHeaderRow> HeaderRowWidget;

	TArray<SubsystemColumnPtr> DynamicColumnSlots;

	/** Root items for the tree widget */
	TArray<SubsystemTreeItemPtr> RootTreeItems;

	/** All items that are currently displayed in the tree widget */
	TMap<FSubsystemTreeItemID, SubsystemTreeItemPtr> TreeItemMap;

	TSharedPtr<SSubsystemsTreeWidget> TreeWidget;

	TOptional<TWeakObjectPtr<UObject>> PendingSelectionObject;

	bool bIsReentrant = false;
	bool bFullRefresh = true;
	bool bNeedsRefresh = true; // needs initial update
	bool bNeedRefreshDetails = false;
	bool bUpdatingSelection = false;
	bool bLoadedExpansionSettings = false;
	bool bNeedsExpansionSettingsSave = false;
	bool bNeedListRebuild = true; // needs initial header update to upply config
	bool bNeedsColumnRefresh = false; // refresh header widgets?

	// column to sort by
	FName SortByColumn;
	// sorting order
	EColumnSortMode::Type SortMode = EColumnSortMode::None;
	//
	bool bSortDirty = false;
};
