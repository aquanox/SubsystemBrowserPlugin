// Copyright 2022, Aquanox.

#include "UI/SubsystemBrowserPanel.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserStyle.h"
#include "SubsystemBrowserUtils.h"
#include "Components/SlateWrapperTypes.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Input/SComboButton.h"
#include "SlateOptMacros.h"
#include "ToolMenus.h"
#include "Editor.h"
#include "Engine/MemberReference.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "UI/SubsystemDetailsCustomizations.h"
#include "HAL/PlatformApplicationMisc.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

SSubsystemBrowserPanel::SSubsystemBrowserPanel()
{
}

SSubsystemBrowserPanel::~SSubsystemBrowserPanel()
{
	FEditorDelegates::PostPIEStarted.RemoveAll(this);
	FEditorDelegates::PrePIEEnded.RemoveAll(this);

	GEngine->OnWorldAdded().RemoveAll(this);
	GEngine->OnWorldDestroyed().RemoveAll(this);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSubsystemBrowserPanel::Construct(const FArguments& InArgs)
{
	// Automatically switch to pie world and back
	FEditorDelegates::PostPIEStarted.AddSP(this, &SSubsystemBrowserPanel::HandlePIEStart);
	FEditorDelegates::PrePIEEnded.AddSP(this, &SSubsystemBrowserPanel::HandlePIEEnd);

	// World load/unload events
	GEngine->OnWorldAdded().AddSP(this, &SSubsystemBrowserPanel::HandleWorldChange);
	GEngine->OnWorldDestroyed().AddSP(this, &SSubsystemBrowserPanel::HandleWorldChange);

	// Update initial settings to apply custom category registrations
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	Settings->SyncCategorySettings();
	Settings->SyncColumnSettings();

	// Automatically handle settings change
	USubsystemBrowserSettings::OnSettingChanged().AddSP(this, &SSubsystemBrowserPanel::OnSettingsChanged);
	FModuleManager::Get().OnModulesChanged().AddSP(this, &SSubsystemBrowserPanel::OnModulesChanged);

	SubsystemModel = MakeShared<FSubsystemModel>();
	SubsystemModel->SetCurrentWorld(InArgs._InWorld);
	SubsystemModel->OnDataChanged.AddSP(this, &SSubsystemBrowserPanel::OnSubsystemDataChanged);

	// Generate search box
	SearchBoxSubsystemFilter = MakeShared<SubsystemTextFilter>(
		SubsystemTextFilter::FItemToStringArray::CreateSP(this, &SSubsystemBrowserPanel::TransformItemToString)
	);
	SearchBoxSubsystemFilter->OnChanged().AddSP(this, &SSubsystemBrowserPanel::FullRefresh);

	// Generate category selector
	CategoryFilter = MakeShared<SubsystemCategoryFilter>();
	CategoryFilter->OnChanged().AddSP(this, &SSubsystemBrowserPanel::FullRefresh);

	// Assign filters to model

	SubsystemModel->CategoryFilter = CategoryFilter;
	SubsystemModel->SubsystemTextFilter = SearchBoxSubsystemFilter;

	// Generate tree view header.
	HeaderRowWidget = SNew(SSubsystemsHeaderRow, SubsystemModel, SharedThis(this));

	// Build the details viewer
	DetailsView = CreateDetails();
	check(DetailsView.IsValid());

	// Build the actual subsystem browser view panel
	ChildSlot
	[
		SNew(SVerticalBox)

		// Subsystems First Toolbar (World Selection)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FStyleHelper::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				SNew(SHorizontalBox)

				// Toolbar Button
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				[
					// Levels menu
					SNew( SComboButton )
					.ComboButtonStyle(FStyleHelper::GetWidgetStylePtr<FComboButtonStyle>("ToolbarComboButton"))
					.ForegroundColor(FLinearColor::White)
					.ContentPadding(0)
					.OnGetMenuContent(this, &SSubsystemBrowserPanel::GetWorldsButtonContent)
					.ButtonContent()
					[
						SNew(SHorizontalBox)

						// Icon
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(this, &SSubsystemBrowserPanel::GetWorldsMenuBrush)
						]

						// Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(1,2,1,2)
						[
							SNew(STextBlock)
							.TextStyle(FStyleHelper::GetWidgetStylePtr<FTextBlockStyle>("ContentBrowser.TopBar.Font"))
							.Text(this, &SSubsystemBrowserPanel::GetCurrentWorldText)
						]
					]
				]
				
			]
		]

		// Subsystems Second Toolbar (Search Panel)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FStyleHelper::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				SNew(SHorizontalBox)

				// Filter box
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SSearchBox)
						.ToolTipText(LOCTEXT("FilterSearchToolTip", "Type here to search Subsystems"))
						.HintText(LOCTEXT("FilterSearchHint", "Search Subsystems"))
						.OnTextChanged(this, &SSubsystemBrowserPanel::SetFilterText)
				]

				//Refresh Button
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.Padding(4, 0, 0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					.ButtonStyle(FStyleHelper::GetWidgetStylePtr<FButtonStyle>("SimpleButton"))
					.ContentPadding(FMargin(1, 0))
					.OnClicked(this, &SSubsystemBrowserPanel::RequestRefresh)
					.ToolTipText(LOCTEXT("SubsystemListRefresh", "Refresh displayed subsystems"))
					[
						SNew(SImage)
						.Image(FStyleHelper::GetBrush("Icons.Refresh"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				]
			]
		]

		// Subsystems Content
		+SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(0,4,0,0)
		[
			SNew(SBorder)
			.BorderImage(FStyleHelper::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				SAssignNew(BrowserSplitter, SSplitter)
				.MinimumSlotHeight(100.f)
				.Orientation(Settings->GetSeparatorOrientation() == ESubsystemBrowserSplitterOrientation::Horizontal ? Orient_Horizontal : Orient_Vertical)
#if UE_VERSION_OLDER_THAN(5, 0, 0)
				.Style(FStyleHelper::GetWidgetStylePtr<FSplitterStyle>("Splitter"))
#else
				.Style(FStyleHelper::GetWidgetStylePtr<FSplitterStyle>("SplitterDark"))
#endif
				.PhysicalSplitterHandleSize(4.0f)
				.HitDetectionSplitterHandleSize(6.0f)
				.OnSplitterFinishedResizing(this, &SSubsystemBrowserPanel::BrowserSplitterFinishedResizing)
				+ SSplitter::Slot()
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
				.MinSize(120.f)
#endif
				.Value(Settings->GetSeparatorLocation())
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(0, 0, 0, 2)
					[
						SAssignNew(TreeWidget, SSubsystemsTreeWidget, SubsystemModel, SharedThis(this))
							.TreeItemsSource(&RootTreeItems)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SSubsystemBrowserPanel::GenerateTreeRow)
							.OnGetChildren(this, &SSubsystemBrowserPanel::GetChildrenForTree)
							.OnSelectionChanged(this, &SSubsystemBrowserPanel::OnSelectionChanged)
							.OnExpansionChanged(this, &SSubsystemBrowserPanel::OnExpansionChanged)
							.OnMouseButtonDoubleClick(this, &SSubsystemBrowserPanel::OnTreeViewMouseButtonDoubleClick)
							.OnContextMenuOpening(this, &SSubsystemBrowserPanel::ConstructSubsystemContextMenu)
							.HighlightParentNodesForSelection(true)
							.ClearSelectionOnClick(true)
							.HeaderRow(HeaderRowWidget.ToSharedRef())
					]

					// View options
					+SVerticalBox::Slot()
					.Padding(0, 0, 0, 2)
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						.Visibility(EVisibility::Visible)

						// Asset count
						+SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						.Padding(8, 0)
						[
							SNew( STextBlock )
							.Text( this, &SSubsystemBrowserPanel::GetFilterStatusText )
							.ColorAndOpacity( this, &SSubsystemBrowserPanel::GetFilterStatusTextColor )
						]

						// View mode combo button
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew( ViewOptionsComboButton, SComboButton )
							.ContentPadding(0)
							.ForegroundColor( this, &SSubsystemBrowserPanel::GetViewOptionsButtonForegroundColor )
							.ButtonStyle( FStyleHelper::GetWidgetStylePtr<FButtonStyle>("ToggleButton") )
							.OnGetMenuContent( this, &SSubsystemBrowserPanel::GetViewOptionsButtonContent )
							.ButtonContent()
							[
								SNew(SHorizontalBox)

								+SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SImage).Image( FStyleHelper::GetBrush("GenericViewButton") )
								]

								+SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(2, 0, 0, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock).Text( LOCTEXT("ViewButton", "View Options") )
								]
							]
						]
					]

					// Separator
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 1)
					[
						SNew(SSeparator)
						.Visibility(EVisibility::Visible)
					]
				]
				+ SSplitter::Slot()
				[
					SAssignNew( DetailsViewBox, SVerticalBox )
					+SVerticalBox::Slot()
					.Padding(0, 4, 0, 2)
					[
						DetailsView.ToSharedRef()
					]
				]
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSubsystemBrowserPanel::Tick(const FGeometry& AllotedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::Tick);

	SCompoundWidget::Tick(AllotedGeometry, InCurrentTime, InDeltaTime);

	if (bNeedsRefresh)
	{
		if (!bIsReentrant)
		{
			Populate();
		}
	}

	if (bSortDirty)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::RequestTreeRefresh);

		// SortItems(RootTreeItems);
		for (const auto& Pair : TreeItemMap)
		{
			Pair.Value->bChildrenRequireSort = true;
		}

		TreeWidget->RequestTreeRefresh();

		bSortDirty = false;
	}

	if (bNeedsColumnRefresh)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::RefreshColumns);

		bNeedsColumnRefresh = false;
		HeaderRowWidget->RefreshColumns();
	}

	if (bNeedRefreshDetails || PendingSelectionObject.IsSet())
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::RefreshDetails);

		if (DetailsView.IsValid())
		{
			if (PendingSelectionObject.IsSet())
			{
				DetailsView->SetObject(PendingSelectionObject.GetValue().Get(), true);
			}
			else
			{
				DetailsView->ForceRefresh();
			}
		}

		PendingSelectionObject.Reset();
		bNeedRefreshDetails = false;
	}

	if (bNeedsExpansionSettingsSave)
	{
		USubsystemBrowserSettings::Get()->SetTreeExpansionStates(GetParentsExpansionState());
		bNeedsExpansionSettingsSave = false;
	}
}

void SSubsystemBrowserPanel::Populate()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::Populate);

	TGuardValue<bool> ReentrantGuard(bIsReentrant, true);

	TMap<FSubsystemTreeItemID, bool> ExpansionStateInfo = GetParentsExpansionState();

	const FSubsystemTreeItemID SelectedItem = GetFirstSelectedItemId();

	if (bFullRefresh)
	{
		FilteredSubsystemsCount = 0;
		EmptyTreeItems();
		ResetSelectedObject();
		
		SubsystemModel->GetFilteredCategories(RootTreeItems);
		for (SubsystemTreeItemPtr CategoryItem : RootTreeItems)
		{
			if (!bLoadedExpansionSettings || !ExpansionStateInfo.Num())
			{
				bool bExpanded = USubsystemBrowserSettings::Get()->GetTreeExpansionState(CategoryItem->GetID());

				CategoryItem->bExpanded = bExpanded;
				ExpansionStateInfo.Add(CategoryItem->GetID(), bExpanded);
			}

			TreeItemMap.Add(CategoryItem->GetID(), CategoryItem);

			SubsystemModel->GetFilteredSubsystems(CategoryItem, CategoryItem->Children);
			for (SubsystemTreeItemPtr  SubsystemItem : CategoryItem->GetChildren())
			{
				TreeItemMap.Add(SubsystemItem->GetID(), SubsystemItem);

				FilteredSubsystemsCount ++;
				
				if (USubsystemBrowserSettings::Get()->ShouldShowSubobjbects())
				{
					SubsystemModel->GetSubsystemSubobjects(SubsystemItem, SubsystemItem->Children);
				}
			}
		}

		bFullRefresh = false;
	}

	SetParentsExpansionState(ExpansionStateInfo);

	if (SubsystemTreeItemPtr LastSelected = TreeItemMap.FindRef(SelectedItem))
	{
		SetSelectedObject(LastSelected);
	}

	if (bNeedListRebuild)
	{
		HeaderRowWidget->RebuildColumns();
		TreeWidget->RebuildList();

		bNeedListRebuild = false;
	}

	TreeWidget->RequestTreeRefresh();

	bNeedsRefresh = false;

	bLoadedExpansionSettings = true;
}

void SSubsystemBrowserPanel::EmptyTreeItems()
{
	for (auto& Pair : TreeItemMap)
	{
		Pair.Value->RemoveAllChildren();
	}

	RootTreeItems.Empty();
	TreeItemMap.Reset();
}

void SSubsystemBrowserPanel::FullRefresh()
{
	bFullRefresh = true;

	RefreshView();
	RefreshDetails();
	RequestSort();
}

void SSubsystemBrowserPanel::TransformItemToString(const ISubsystemTreeItem&  Item, TArray<FString>& OutSearchStrings) const
{
	if (Item.GetAsSubsystemDescriptor())
	{
		for (auto& Column : SubsystemModel->GetSelectedTableColumns())
		{
			Column->PopulateSearchStrings(Item, OutSearchStrings);
		}
	}
}

void SSubsystemBrowserPanel::SetFilterText(const FText& InFilterText)
{
	SearchBoxSubsystemFilter->SetRawFilterText(InFilterText);
}

FText SSubsystemBrowserPanel::GetSearchBoxText() const
{
	return SearchBoxSubsystemFilter->GetRawFilterText();
}

FText SSubsystemBrowserPanel::GetFilterStatusText() const
{
	const int32 SubsystemTotalCount = SubsystemModel->GetNumSubsystemsFromVisibleCategories();

	if (!SubsystemModel->IsSubsystemFilterActive())
	{
		return FText::Format( LOCTEXT("ShowSubsystemsCounterFmt", "{0} subsystems"), FText::AsNumber( FilteredSubsystemsCount ) );
	}
	else
	{
		if ( FilteredSubsystemsCount == 0)
		{   // all subsystems were filtered out
			return FText::Format( LOCTEXT("ShowNoSubsystemsCounterFmt", "No matching subsystems out of {0} total"), FText::AsNumber( SubsystemTotalCount ) );
		}
		else
		{   // got something to display
			return FText::Format( LOCTEXT("ShowingOnlySomeSubsystemsFmt", "Showing {0} of {1} subsystems"), FText::AsNumber( FilteredSubsystemsCount ), FText::AsNumber( SubsystemTotalCount ) );
		}
	}
}

FSlateColor SSubsystemBrowserPanel::GetFilterStatusTextColor() const
{
	if (!SubsystemModel->IsSubsystemFilterActive())
	{
		// White = no text filter
		return FLinearColor(1.0f, 1.0f, 1.0f);
	}
	else if (FilteredSubsystemsCount == 0)
	{
		// Red = no matching actors
		return FLinearColor(1.0f, 0.4f, 0.4f);
	}
	else
	{
		// Green = found at least one match!
		return FLinearColor(0.4f, 1.0f, 0.4f);
	}
}

void SSubsystemBrowserPanel::BrowserSplitterFinishedResizing()
{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	float NewValue = BrowserSplitter->SlotAt(0).SizeValue.Get();
#else
	float NewValue = BrowserSplitter->SlotAt(0).GetSizeValue();
#endif
	USubsystemBrowserSettings::Get()->SetSeparatorLocation(NewValue);
}

FSlateColor SSubsystemBrowserPanel::GetViewOptionsButtonForegroundColor() const
{
	static const FName InvertedForegroundName("InvertedForeground");
	static const FName DefaultForegroundName("DefaultForeground");

	return ViewOptionsComboButton->IsHovered()
		? FStyleHelper::GetSlateColor(InvertedForegroundName)
		: FStyleHelper::GetSlateColor(DefaultForegroundName);
}

TSharedRef<SWidget> SSubsystemBrowserPanel::GetViewOptionsButtonContent()
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewColumnsGroup", "Columns"));
	{
		if (SubsystemModel->GetNumDynamicColumns() > Settings->GetMaxColumnTogglesToShow())
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("ChooseColumnSubMenu", "Choose Columns"),
				LOCTEXT("ChooseColumnSubMenu_ToolTip", "Choose columns to display in browser."),
				FNewMenuDelegate::CreateSP(this, &SSubsystemBrowserPanel::BuildColumnPickerContent)
			);
		}
		else
		{
			BuildColumnPickerContent(MenuBuilder);
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewCategoryGroup", "Categories"));
	{
		if (SubsystemModel->GetNumCategories() > Settings->GetMaxCategoryTogglesToShow())
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("ChooseCategorySubMenu", "Choose Category"),
				LOCTEXT("ChooseCategorySubMenu_ToolTip", "Choose categories to display in browser."),
				FNewMenuDelegate::CreateSP(this, &SSubsystemBrowserPanel::BuildCategoryPickerContent)
			);
		}
		else
		{
			BuildCategoryPickerContent(MenuBuilder);
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewFilteringGroup", "Display"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleGameOnly", "Only Game Modules"),
			LOCTEXT("ToggleGameOnly_Tooltip", "Show only subsystems that are within Game Modules."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleShouldShowOnlyGame),
				FCanExecuteAction(),
				FIsActionChecked::CreateUObject(Settings, &USubsystemBrowserSettings::ShouldShowOnlyGame)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleViewableOnly", "Only With Properties"),
			LOCTEXT("ToggleViewableOnly_Tooltip", "Show only subsystems that have viewable elements."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleShouldShowOnlyViewable),
				FCanExecuteAction(),
				FIsActionChecked::CreateUObject(Settings, &USubsystemBrowserSettings::ShouldShowOnlyViewable)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleSubobjects", "Show Subobjects"),
			LOCTEXT("ToggleSubobjects_Tooltip", "Enable display of notable subobjects in subsystem display as nested elements."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleShowSubobjects),
				FCanExecuteAction(),
				FIsActionChecked::CreateUObject(Settings, &USubsystemBrowserSettings::ShouldShowSubobjbects)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleHiddenProps", "Show Hidden Properties"),
			LOCTEXT("ToggleHiddenProps_Tooltip", "Enforces display of all hidden object properties in details panel."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleForceHiddenPropertyVisibility),
				FCanExecuteAction(),
				FIsActionChecked::CreateUObject(Settings, &USubsystemBrowserSettings::ShouldForceHiddenPropertyVisibility)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewOptionsGroup", "Options"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("OpenSubsystemSettingsPanel", "Subsystem Settings"),
			LOCTEXT("OpenSubsystemSettingsPanel_Tooltip", "Open subsystem settings panel."),
			FStyleHelper::GetSlateIcon(FStyleHelper::PanelIconName),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ShowSubsystemSettingsTab),
				FCanExecuteAction(),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateUObject(Settings, &USubsystemBrowserSettings::ShouldUseSubsystemSettings)
			),
			NAME_None,
			EUserInterfaceActionType::Button
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("OpenSettingsPanel", "Plugin Settings"),
			LOCTEXT("OpenSettingsPanel_Tooltip", "Open plugin settings panel."),
			FStyleHelper::GetSlateIcon("EditorPreferences.TabIcon"),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ShowPluginSettingsTab)
			),
			NAME_None,
			EUserInterfaceActionType::Button
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SSubsystemBrowserPanel::BuildColumnPickerContent(FMenuBuilder& MenuBuilder)
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	for (const SubsystemColumnPtr& Column : SubsystemModel->GetDynamicTableColumns())
	{
		MenuBuilder.AddMenuEntry(
			Column->ConfigLabel,
			LOCTEXT("ToggleDisplayColumn_Tooltip", "Toggles display of subsystem browser columns."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleDisplayColumn, Column->Name),
				FCanExecuteAction(),
				FIsActionChecked::CreateUObject(Settings, &USubsystemBrowserSettings::GetTableColumnState, Column->Name)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
}

void SSubsystemBrowserPanel::BuildCategoryPickerContent(FMenuBuilder& MenuBuilder)
{
	for (const SubsystemTreeItemPtr& Category : SubsystemModel->GetAllCategories())
	{
		check(Category->GetAsCategoryDescriptor());
		FSubsystemTreeItemID CategoryID = Category->GetID();

		MenuBuilder.AddMenuEntry(Category->GetDisplayName(),
			LOCTEXT("ToggleDisplayCategory_Tooltip", "Toggles display of subsystems for category."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleDisplayCategory, CategoryID),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SSubsystemBrowserPanel::GetCategoryDisplayStatus, CategoryID)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
}

void SSubsystemBrowserPanel::ToggleDisplayCategory(FSubsystemTreeItemID InCategory)
{
	if (CategoryFilter->IsCategoryVisible(InCategory))
	{
		CategoryFilter->HideCategory(InCategory);
	}
	else
	{
		CategoryFilter->ShowCategory(InCategory);
	}

	FullRefresh();
}

bool SSubsystemBrowserPanel::GetCategoryDisplayStatus(FSubsystemTreeItemID InCategory)
{
	return CategoryFilter->IsCategoryVisible(InCategory);
}

void SSubsystemBrowserPanel::SetupColumns(SHeaderRow& HeaderRow)
{
	HeaderRow.ClearColumns();

	for (const SubsystemColumnPtr& Column : SubsystemModel->GetSelectedTableColumns())
	{
		auto ColumnArgs = Column->GenerateHeaderColumnWidget();

		if (Column->SupportsSorting())
		{
			ColumnArgs
				.SortMode(this, &SSubsystemBrowserPanel::GetColumnSortMode, Column->Name)
				.OnSort(this, &SSubsystemBrowserPanel::OnColumnSortModeChanged);
		}
		else
		{
			ColumnArgs.SortMode(EColumnSortMode::None);
		}

		HeaderRow.AddColumn(ColumnArgs);
	}

	bNeedsColumnRefresh = true;
}


TSharedRef<ITableRow> SSubsystemBrowserPanel::GenerateTreeRow(SubsystemTreeItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	check(Item.IsValid());

	return SNew(SSubsystemTableItem, OwnerTable)
		.InModel(SubsystemModel)
		.InBrowser(SharedThis(this))
		.InItemModel(Item)
		.IsItemExpanded(Item->bExpanded)
		.HighlightText(this, &SSubsystemBrowserPanel::GetSearchBoxText)
		;
}

void SSubsystemBrowserPanel::GetChildrenForTree(SubsystemTreeItemPtr Item, TArray<SubsystemTreeItemPtr>& OutChildren)
{
	OutChildren = Item->GetChildren();

	if (OutChildren.Num() && Item->bChildrenRequireSort)
	{
		// Sort the children we returned
		SortItems(OutChildren);

		// Empty out the children and repopulate them in the correct order
		Item->Children.Empty();
		for (auto& Child : OutChildren)
		{
			Item->Children.Emplace(Child);
		}

		// They no longer need sorting
		Item->bChildrenRequireSort = false;
	}
}

void SSubsystemBrowserPanel::OnExpansionChanged(SubsystemTreeItemPtr Item, bool bIsItemExpanded)
{
	Item->bExpanded = bIsItemExpanded;

	if (Item->CanHaveChildren())
	{
		for (SubsystemTreeItemPtr Child : Item->GetChildren())
		{
			Child->bExpanded = bIsItemExpanded;
		}
	}

	// Save expansion states
	bNeedsExpansionSettingsSave = true;

	RefreshView();
}

void SSubsystemBrowserPanel::OnTreeViewMouseButtonDoubleClick(SubsystemTreeItemPtr Item)
{
	if (Item.IsValid() && Item->GetAsCategoryDescriptor())
	{
		Item->bExpanded = ! Item->bExpanded;
		TreeWidget->SetItemExpansion(Item, Item->bExpanded);
	}
}

void SSubsystemBrowserPanel::ToggleDisplayColumn(FName ColumnName)
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	bool OldState = Settings->GetTableColumnState(ColumnName);
	Settings->SetTableColumnState(ColumnName, !OldState);

	bNeedListRebuild = true;
	bNeedsRefresh = true;
}

void SSubsystemBrowserPanel::ToggleTableColoring()
{
	USubsystemBrowserSettings::Get()->ToggleColoringEnabled();

	RefreshView();
}

void SSubsystemBrowserPanel::ToggleForceHiddenPropertyVisibility()
{
	USubsystemBrowserSettings::Get()->ToggleForceHiddenPropertyVisibility();

	RefreshView();
	RecreateDetails();
}

void SSubsystemBrowserPanel::ToggleShowSubobjects()
{
	USubsystemBrowserSettings::Get()->ToggleShowSubobjbects();

	RefreshView();
}

void SSubsystemBrowserPanel::ToggleShouldShowOnlyGame()
{
	USubsystemBrowserSettings::Get()->ToggleShouldShowOnlyGame();

	FullRefresh();
}

void SSubsystemBrowserPanel::ToggleShouldShowOnlyPlugins()
{
	USubsystemBrowserSettings::Get()->ToggleShouldShowOnlyPlugins();

	FullRefresh();
}

void SSubsystemBrowserPanel::ToggleShouldShowOnlyViewable()
{
	USubsystemBrowserSettings::Get()->ToggleShouldShowOnlyViewable();

	FullRefresh();
}

void SSubsystemBrowserPanel::ShowPluginSettingsTab() const
{
	FSubsystemBrowserModule::Get().SummonPluginSettingsTab();
}

void SSubsystemBrowserPanel::ShowSubsystemSettingsTab() const
{
	FSubsystemBrowserModule::Get().SummonSubsystemSettingsTab();
}

FReply SSubsystemBrowserPanel::RequestRefresh()
{
	FullRefresh();
	return FReply::Handled();
}

void SSubsystemBrowserPanel::OnSelectionChanged(const SubsystemTreeItemPtr Item, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	if (!bUpdatingSelection)
	{
		TGuardValue<bool> ScopeGuard(bUpdatingSelection, true);

		const TArray<SubsystemTreeItemPtr>& SelectedItems = TreeWidget->GetSelectedItems();

		SetSelectedObject(SelectedItems.Num() ? SelectedItems[0] : nullptr);
	}
}

const FSlateBrush* SSubsystemBrowserPanel::GetWorldsMenuBrush() const
{
	return FStyleHelper::GetBrush("WorldBrowser.LevelsMenuBrush");
}

FText SSubsystemBrowserPanel::GetCurrentWorldText() const
{
	if (SubsystemModel->GetCurrentWorld().IsValid())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("World"), FSubsystemBrowserUtils::GetWorldDescription(SubsystemModel->GetCurrentWorld().Get()));
		return FText::Format(LOCTEXT("WorldsSelectButton", "World: {World}"), Args);
	}
	else
	{
		return LOCTEXT("WorldsSelectButton_Bad", "World: Invalid");
	}
}

void SSubsystemBrowserPanel::OnSelectWorld(TWeakObjectPtr<UWorld> InWorld)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Selected world %s"), *GetNameSafe(InWorld.Get()));

	SubsystemModel->SetCurrentWorld(InWorld);
	FullRefresh();
}

bool SSubsystemBrowserPanel::IsWorldChecked(TWeakObjectPtr<UWorld> InWorld)
{
	return SubsystemModel->GetCurrentWorld() == InWorld;
}

TSharedRef<SWidget> SSubsystemBrowserPanel::GetWorldsButtonContent()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("Worlds", LOCTEXT("WorldsHeading", "Worlds"));

	auto IsAllowedWorldType = [](EWorldType::Type InType) -> bool
	{
		// TBD: make a setting to pick world instead?
		if (USubsystemBrowserSettings::Get()->ShouldDisplayAllWorlds())
			return true;
		return InType == EWorldType::PIE || InType == EWorldType::Editor;
	};

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World && IsAllowedWorldType(World->WorldType))
		{
			MenuBuilder.AddMenuEntry(
				FSubsystemBrowserUtils::GetWorldDescription(World),
				LOCTEXT("ChooseWorldToolTip", "Display subsystems for this world."),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP( this, &SSubsystemBrowserPanel::OnSelectWorld, MakeWeakObjectPtr(World) ),
					FCanExecuteAction(),
					FIsActionChecked::CreateSP( this, &SSubsystemBrowserPanel::IsWorldChecked, MakeWeakObjectPtr(World) )
				),
				NAME_None,
				EUserInterfaceActionType::RadioButton
			);
		}
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void SSubsystemBrowserPanel::HandlePIEStart(const bool bIsSimulating)
{
	UE_LOG(LogSubsystemBrowser, Verbose, TEXT("On PIE Start"));

	if (USubsystemBrowserSettings::Get()->ShouldDisplayAllWorlds())
	{
		PrePieSelectedWorld = SubsystemModel->GetCurrentWorld();
	}

	UWorld* PIEWorld = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World && World->WorldType == EWorldType::PIE)
		{
			PIEWorld = World;
			break;
		}
	}
	OnSelectWorld(PIEWorld);
}

void SSubsystemBrowserPanel::HandlePIEEnd(const bool bIsSimulating)
{
	UE_LOG(LogSubsystemBrowser, Verbose, TEXT("On PIE End"));

	if (USubsystemBrowserSettings::Get()->ShouldDisplayAllWorlds() && PrePieSelectedWorld.IsValid())
	{
		OnSelectWorld(PrePieSelectedWorld.Get());
		PrePieSelectedWorld.Reset();
		return;
	}

	UWorld* EditorWorld = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World->WorldType == EWorldType::Editor)
		{
			EditorWorld = World;
			break;
		}
	}

	OnSelectWorld(EditorWorld);
}

void SSubsystemBrowserPanel::HandleWorldChange(UWorld* InWorld)
{
	UE_LOG(LogSubsystemBrowser, Verbose, TEXT("On World Changed"));

	// Automatically switch to the newest editor world after opening level
	if (InWorld && InWorld->WorldType == EWorldType::Editor)
	{
		OnSelectWorld(InWorld);
	}
}

TSharedRef<IDetailsView> SSubsystemBrowserPanel::CreateDetails()
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.ViewIdentifier = TEXT("SubsystemBrowserDetailsPanel");
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bShowAnimatedPropertiesOption = false;
	DetailsViewArgs.bShowKeyablePropertiesOption = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bAllowFavoriteSystem = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowOptions = true;
	
	// show All properties. possibly apply custom property filter or custom checkbox.
	// but there is no way to change its value via IDetailsView interface
	// so show all and filter visibility by IsPropertyVisible
	DetailsViewArgs.bForceHiddenPropertyVisibility = Settings->ShouldForceHiddenPropertyVisibility();

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	TSharedRef<IDetailsView> DetailViewWidget = EditModule.CreateDetailView(DetailsViewArgs);

	if (Settings->ShouldUseCustomPropertyFilteringInBrowser())
	{
		DetailViewWidget->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SSubsystemBrowserPanel::IsDetailsPropertyVisible));
		DetailViewWidget->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateSP(this, &SSubsystemBrowserPanel::IsDetailsPropertyReadOnly));
	}

	FSubsystemBrowserModule::OnCustomizeDetailsView.Broadcast(DetailViewWidget, TEXT("SubsystemBrowserPanel"));

	return DetailViewWidget;
}

void SSubsystemBrowserPanel::RecreateDetails()
{
	TSharedPtr<IDetailsView> ExistingDetails = DetailsView;

	DetailsView = CreateDetails();

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	DetailsViewBox->ClearChildren();
	DetailsViewBox->AddSlot().Padding(0, 4, 0, 2) [ DetailsView.ToSharedRef() ];
#else
	DetailsViewBox->GetSlot(0) [ DetailsView.ToSharedRef() ];
#endif

	// Copy other props from existing details ?
	ExistingDetails.Reset();
}

void SSubsystemBrowserPanel::SetSelectedObject(SubsystemTreeItemPtr Item)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::SetSelectedObject);

	UObject* InObject = Item.IsValid() ? Item->GetObjectForDetails() : nullptr;
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Selected object %s"), *GetNameSafe(InObject));

	SubsystemModel->NotifySelected(Item);

	if (DetailsView.IsValid())
	{
		PendingSelectionObject = InObject;
		RefreshDetails();
	}
}

void SSubsystemBrowserPanel::ResetSelectedObject()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SSubsystemBrowserPanel::ResetSelectedObject);

	UE_LOG(LogSubsystemBrowser, Log, TEXT("Reset selected object"));

	SubsystemModel->NotifySelected(nullptr);

	if (DetailsView.IsValid())
	{
		PendingSelectionObject = nullptr;
		RefreshDetails();
	}
}

bool SSubsystemBrowserPanel::IsDetailsPropertyVisible(const FPropertyAndParent& InProperty) const
{
	static const FName NAME_Hidden(TEXT("Hidden"));
	
	const FProperty* Property = InProperty.ParentProperties.Num() > 0 ? InProperty.ParentProperties.Last() : &InProperty.Property;

	// always hide blueprint delegate properties
	if (Property->IsA(FDelegateProperty::StaticClass()) || Property->IsA(FMulticastDelegateProperty::StaticClass()))
	{
		return false;
	}

	if (Property->FindMetaData(FSubsystemBrowserUserMeta::MD_SBHidden) != nullptr || Property->FindMetaData(NAME_Hidden) != nullptr)
	{
		return false;
	}

	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	if (Settings->ShouldShowAnyProperties())
	{
		return true;
	}
	
	// by default any property with EDIT can be edited
	return Property->HasAnyPropertyFlags(CPF_Edit);
}

bool SSubsystemBrowserPanel::IsDetailsPropertyReadOnly(const FPropertyAndParent& InProperty) const
{
	const FProperty* Property = InProperty.ParentProperties.Num() > 0 ? InProperty.ParentProperties.Last() : &InProperty.Property;

	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	if (Settings->ShouldEditAnyProperties())
	{
		return false;
	}

	// by default any property with EditConst or DisableEditOnInstance is readonly (as SS is an instance)
	return Property->HasAnyPropertyFlags(CPF_EditConst|CPF_DisableEditOnInstance);
}

SubsystemTreeItemPtr SSubsystemBrowserPanel::GetFirstSelectedItem() const
{
	return TreeWidget->GetNumItemsSelected() ? TreeWidget->GetSelectedItems()[0] : SubsystemTreeItemPtr();
}

const FSubsystemTreeSubsystemItem* SSubsystemBrowserPanel::GetFirstSelectedSubsystem() const
{
	for(SubsystemTreeItemPtr Selected : TreeWidget->GetSelectedItems())
	{
		if (Selected->GetAsSubsystemDescriptor())
		{
			return Selected->GetAsSubsystemDescriptor();
		}
	}

	return nullptr;
}

FSubsystemTreeItemID SSubsystemBrowserPanel::GetFirstSelectedItemId() const
{
	return TreeWidget->GetNumItemsSelected() ? TreeWidget->GetSelectedItems()[0]->GetID() : FSubsystemTreeItemID();
}

bool SSubsystemBrowserPanel::IsItemSelected(SubsystemTreeItemPtr Item)
{
	return TreeWidget->IsItemSelected(Item);
}

TMap<FSubsystemTreeItemID, bool> SSubsystemBrowserPanel::GetParentsExpansionState() const
{
	TMap<FSubsystemTreeItemID, bool> ExpansionStates;

	for (const auto& Pair : TreeItemMap)
	{
		if (Pair.Value->GetNumChildren() > 0)
		{
			ExpansionStates.Add(Pair.Key, Pair.Value->bExpanded);
		}
	}

	return ExpansionStates;
}

void SSubsystemBrowserPanel::SetParentsExpansionState(const TMap<FSubsystemTreeItemID, bool>& ExpansionInfo)
{
	for (const auto& Pair : TreeItemMap)
	{
		auto& Item = Pair.Value;
		if (Item->GetNumChildren() > 0)
		{
			const bool* bExpandedPtr = ExpansionInfo.Find(Pair.Key);
			bool bExpanded = bExpandedPtr != nullptr ? *bExpandedPtr : Item->bExpanded;

			TreeWidget->SetItemExpansion(Item, bExpanded);
		}
	}
}

void SSubsystemBrowserPanel::ResetParentsExpansionState()
{
	for (const auto& Pair : TreeItemMap)
	{
		if (Pair.Value->GetNumChildren() > 0)
		{
			Pair.Value->bExpanded = true;
		}
	}

	bNeedsRefresh = true;
}

TSharedPtr<SWidget> SSubsystemBrowserPanel::ConstructSubsystemContextMenu()
{
	UToolMenus* ToolMenus = UToolMenus::Get();

	if (!ToolMenus->IsMenuRegistered(FSubsystemBrowserModule::SubsystemBrowserContextMenuName))
	{
		ToolMenus->RegisterMenu(FSubsystemBrowserModule::SubsystemBrowserContextMenuName);
		UE_LOG(LogSubsystemBrowser, Warning, TEXT("For some reason context menu did not register"));
	}

	FToolMenuContext Context;
	UToolMenu* Menu = ToolMenus->GenerateMenu(FSubsystemBrowserModule::SubsystemBrowserContextMenuName, Context);

	SubsystemTreeItemPtr Selected = GetFirstSelectedItem();
	if (Selected.IsValid())
	{
		// Common stuff
		Selected->GenerateContextMenu(Menu);

		// Apply customizations
		FSubsystemBrowserModule::OnGenerateContextMenu.Broadcast(Selected.ToSharedRef(), Menu);
	}

	return ToolMenus->GenerateWidget(Menu);
}

bool SSubsystemBrowserPanel::HasSelectedSubsystem() const
{
	SubsystemTreeItemPtr SelectedSubsystem = GetFirstSelectedItem();
	return SelectedSubsystem.IsValid()  && SelectedSubsystem->GetAsSubsystemDescriptor();
}

void SSubsystemBrowserPanel::OnSettingsChanged(FName InPropertyName)
{
	if (InPropertyName == NAME_All)
	{
		bFullRefresh = true;
		RefreshView();
		RefreshColumns();
		RecreateDetails();
		ResetParentsExpansionState();
		return;
	}

	if (FProperty* Property = USubsystemBrowserSettings::StaticClass()->FindPropertyByName(InPropertyName))
	{
		if (Property->HasMetaData(FSubsystemBrowserConfigMeta::MD_ConfigAffectsView))
		{
			bFullRefresh = true;
			RefreshView();
		}
		if (Property->HasMetaData(FSubsystemBrowserConfigMeta::MD_ConfigAffectsColumns))
		{
			RefreshColumns();
		}
		if (Property->HasMetaData(FSubsystemBrowserConfigMeta::MD_ConfigAffectsDetails))
		{
			RecreateDetails();
		}
	}
}

void SSubsystemBrowserPanel::OnModulesChanged(FName ModuleThatChanged, EModuleChangeReason ReasonForChange)
{
	RefreshView();
}

void SSubsystemBrowserPanel::OnSubsystemDataChanged(TSharedRef<ISubsystemTreeItem> Item)
{
	RefreshView();
}

EColumnSortMode::Type SSubsystemBrowserPanel::GetColumnSortMode(FName ColumnId) const
{
	if (SortByColumn == ColumnId)
	{
		auto Column = SubsystemModel->FindTableColumn(ColumnId);
		if (Column.IsValid() && Column->SupportsSorting())
		{
			return SortMode;
		}
	}
	return EColumnSortMode::None;
}

void SSubsystemBrowserPanel::OnColumnSortModeChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnId,
	const EColumnSortMode::Type InSortMode)
{
	auto Column = SubsystemModel->FindTableColumn(ColumnId);
	if (!Column.IsValid() || !Column->SupportsSorting())
	{
		return;
	}

	SortByColumn = ColumnId;
	SortMode = InSortMode;

	bSortDirty = true;
}

void SSubsystemBrowserPanel::SortItems(TArray<SubsystemTreeItemPtr>& Items) const
{
	auto Column = SubsystemModel->FindTableColumn(SortByColumn);
	if (Column.IsValid() && Column->SupportsSorting())
	{
		Column->SortItems(Items, SortMode);
	}
}

#undef LOCTEXT_NAMESPACE
