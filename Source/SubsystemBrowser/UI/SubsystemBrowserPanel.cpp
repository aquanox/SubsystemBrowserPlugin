// Copyright 2022, Aquanox.

#include "UI/SubsystemBrowserPanel.h"

#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserSettings.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Images/SImage.h"
#include "SlateOptMacros.h"
#include "SourceCodeNavigation.h"
#include "ToolMenus.h"
#include "Components/SlateWrapperTypes.h"
#include "Editor.h"
#include "Engine/MemberReference.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

#if ENGINE_MAJOR_VERSION < 5
const FName SSubsystemBrowserPanel::PanelIconName(TEXT("LevelEditor.GameSettings.Small"));
#else
const FName SSubsystemBrowserPanel::PanelIconName(TEXT("Icons.Settings"));
#endif

static void ShowBrowserInfoMessage(FText InText, SNotificationItem::ECompletionState InType)
{
	FNotificationInfo Info(InText);
	Info.ExpireDuration = 5.0f;

	if (TSharedPtr<SNotificationItem> InfoItem = FSlateNotificationManager::Get().AddNotification(Info))
	{
		InfoItem->SetCompletionState(InType);
	}
}

SSubsystemBrowserPanel::SSubsystemBrowserPanel()
{
}

SSubsystemBrowserPanel::~SSubsystemBrowserPanel()
{
	FEditorDelegates::PostPIEStarted.RemoveAll(this);
	FEditorDelegates::PrePIEEnded.RemoveAll(this);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSubsystemBrowserPanel::Construct(const FArguments& InArgs)
{
	// Automatically switch to pie world and back
	FEditorDelegates::PostPIEStarted.AddSP(this, &SSubsystemBrowserPanel::HandlePIEStart);
	FEditorDelegates::PrePIEEnded.AddSP(this, &SSubsystemBrowserPanel::HandlePIEEnd);

	// Automatically handle settings change
	USubsystemBrowserSettings::OnSettingChanged().AddSP(this, &SSubsystemBrowserPanel::OnSettingsChanged);

	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	SubsystemModel = MakeShared<FSubsystemModel>();
	SubsystemModel->SetCurrentWorld(InArgs._InWorld);

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
	HeaderRowWidget =
		SNew( SHeaderRow )

		/** Subsystem name column */
		+ SHeaderRow::Column(SubsystemColumns::ColumnID_Name)
		  .FillWidth(0.60f)
		  .SortMode(EColumnSortMode::None)
		  .HeaderContent()
		[
			SNew(SBox)
			.MinDesiredHeight(24)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SubsystemBrowser_Column_Name", "Name"))
				]
			]
		]

		/** Subsystem package column */
		+ SHeaderRow::Column( SubsystemColumns::ColumnID_Package )
		  .FillWidth( 0.25f )
		  .SortMode(EColumnSortMode::None)
		  .HeaderContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SubsystemBrowser_Column_Package", "Module"))
			]
		]

		/** Subsystem config class column */
		+ SHeaderRow::Column( SubsystemColumns::ColumnID_ConfigClass )
		  .FillWidth( 0.15f )
		  .SortMode(EColumnSortMode::None)
		  .HeaderContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SubsystemBrowser_Column_ConfigClass", "Config"))
			]
		];

	// Build the details viewer
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.ViewIdentifier = TEXT("SubsystemBrowserView");
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show; // always show defaults
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.bShowAnimatedPropertiesOption = false;
	DetailsViewArgs.bShowKeyablePropertiesOption = false;
	DetailsViewArgs.bAllowFavoriteSystem = false; // no favorites here
	// show All properties. possibly apply custom property filter or custom checkbox.
	// but there is no way to change its value via IDetailsView interface
	// so show all and filter visibility by IsPropertyVisible
	DetailsViewArgs.bForceHiddenPropertyVisibility = Settings->ShouldShowHiddenProperties();

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = EditModule.CreateDetailView( DetailsViewArgs );
	check(DetailsView.IsValid());
	//DetailsView->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateSP(this, &SSubsystemBrowserPanel::IsPropertyReadOnly));
	//DetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SSubsystemBrowserPanel::IsPropertyVisible));

	// Context menu

	FOnContextMenuOpening ContextMenuEvent = FOnContextMenuOpening::CreateSP(this, &SSubsystemBrowserPanel::ConstructSubsystemContextMenu);

	// Build the actual subsystem browser view panel
	ChildSlot
	[
		SNew(SVerticalBox)

		// Subsystems First Toolbar (World Selection)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
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
					.ComboButtonStyle(FEditorStyle::Get(), "ToolbarComboButton")
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
							.Image(this, &SSubsystemBrowserPanel::GetWorldssMenuBrush)
						]

						// Text
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(1,0,2,0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
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
			.BorderImage(FEditorStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
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
			]
		]

		// Subsystems Content
		+SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(0,4,0,0)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				SAssignNew(BrowserSplitter, SSplitter)
				.MinimumSlotHeight(140.0f)
				.Orientation(Orient_Vertical)
				.Style(FEditorStyle::Get(), "SplitterDark")
				.PhysicalSplitterHandleSize(2.0f)
				.OnSplitterFinishedResizing(this, &SSubsystemBrowserPanel::BrowserSplitterFinishedResizing)
				+ SSplitter::Slot().Value(Settings->GetSeparatorLocation())
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(TreeWidget, SSubsystemsTreeWidget, SubsystemModel, SharedThis(this))
						.TreeItemsSource(&RootTreeItems)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SSubsystemBrowserPanel::GenerateTreeRow)
						.OnGetChildren(this, &SSubsystemBrowserPanel::GetChildrenForTree)
						.OnSelectionChanged(this, &SSubsystemBrowserPanel::OnSelectionChanged)
						.OnExpansionChanged(this, &SSubsystemBrowserPanel::OnExpansionChanged)
						.OnMouseButtonDoubleClick(this, &SSubsystemBrowserPanel::OnTreeViewMouseButtonDoubleClick)
						.OnContextMenuOpening(ContextMenuEvent)
						.HighlightParentNodesForSelection(true)
						.ClearSelectionOnClick(true)
						.HeaderRow(HeaderRowWidget.ToSharedRef())
					]

					// Separator
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 1)
					[
						SNew(SSeparator)
						.Visibility(EVisibility::Visible)
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
							.ButtonStyle( FEditorStyle::Get(), "ToggleButton" )
							.OnGetMenuContent( this, &SSubsystemBrowserPanel::GetViewOptionsButtonContent )
							.ButtonContent()
							[
								SNew(SHorizontalBox)

								+SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SImage).Image( FEditorStyle::GetBrush("GenericViewButton") )
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
				]
				+ SSplitter::Slot()
				[
					SNew( SVerticalBox )
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 4, 0, 0)
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
	SCompoundWidget::Tick(AllotedGeometry, InCurrentTime, InDeltaTime);

	if (bNeedsRefresh)
	{
		if (!bIsReentrant)
		{
			Populate();
		}
	}

	if (bNeedRefreshDetails)
	{
		if (DetailsView.IsValid())
		{
			DetailsView->ForceRefresh();
		}
		bNeedRefreshDetails = false;
	}
}

void SSubsystemBrowserPanel::Populate()
{
	TGuardValue<bool> ReentrantGuard(bIsReentrant, true);

	TMap<FSubsystemTreeItemID, bool> ExpansionStateInfo = GetParentsExpansionState();
	if (!bLoadedExpansionSettings)
	{// load settings only on first populate
		USubsystemBrowserSettings::Get()->LoadTreeExpansionStates(ExpansionStateInfo);
		bLoadedExpansionSettings = true;
	}

	const FSubsystemTreeItemID SelectedItem = GetFirstSelectedItemId();

	if (bFullRefresh)
	{
		FilteredSubsystemsCount = 0;
		EmptyTreeItems();
		ResetSelectedObject();

		SubsystemModel->GetFilteredCategories(RootTreeItems);
		for (SubsystemTreeItemPtr Category : RootTreeItems)
		{
			TreeItemMap.Add(Category->GetID(), Category);

			SubsystemModel->GetFilteredSubsystems(Category, Category->Children);
			for (SubsystemTreeItemPtr  Child : Category->GetChildren())
			{
				TreeItemMap.Add(Child->GetID(), Child);

				FilteredSubsystemsCount ++;
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
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
		for (const SHeaderRow::FColumn& Column : TreeWidget->GetHeaderRow()->GetColumns())
		{
			const_cast<SHeaderRow::FColumn&>(Column).bIsVisible = IsColumnVisible(Column.ColumnId);
		}

		TreeWidget->GetHeaderRow()->RefreshColumns();
#endif

		TreeWidget->RebuildList();

		bNeedListRebuild = false;
	}

	TreeWidget->RequestTreeRefresh();

	bNeedsRefresh = false;
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
}

void SSubsystemBrowserPanel::TransformItemToString(const ISubsystemTreeItem&  Item, TArray<FString>& OutSearchStrings) const
{
	if (Item.GetAsSubsystemDescriptor())
	{
		OutSearchStrings.Add(Item.GetDisplayNameString());
		if (IsColumnVisible(SubsystemColumns::ColumnID_Package))
		{
			OutSearchStrings.Add(Item.GetShortPackageString());
		}
		if (IsColumnVisible(SubsystemColumns::ColumnID_ConfigClass))
		{
			OutSearchStrings.Add(Item.GetConfigClassNameString());
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
			return FText::Format( LOCTEXT("ShowSubsystemsCounterFmt", "No matching subsystems out of {0} total"), FText::AsNumber( SubsystemTotalCount ) );
		}
		else
		{   // got something to display
			return FText::Format( LOCTEXT("ShowingOnlySomeActorsFmt", "Showing {0} of {1} subsystems"), FText::AsNumber( FilteredSubsystemsCount ), FText::AsNumber( SubsystemTotalCount ) );
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
	float NewValue = BrowserSplitter->SlotAt(0).SizeValue.Get();
	USubsystemBrowserSettings::Get()->SetSeparatorLocation(NewValue);
}

FSlateColor SSubsystemBrowserPanel::GetViewOptionsButtonForegroundColor() const
{
	static const FName InvertedForegroundName("InvertedForeground");
	static const FName DefaultForegroundName("DefaultForeground");

	return ViewOptionsComboButton->IsHovered() ? FEditorStyle::GetSlateColor(InvertedForegroundName) : FEditorStyle::GetSlateColor(DefaultForegroundName);
}

TSharedRef<SWidget> SSubsystemBrowserPanel::GetViewOptionsButtonContent()
{
	FMenuBuilder MenuBuilder(true, nullptr);

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewColumns", "Columns"));
	{
		for (const FSubsystemModel::FSubsystemColumn& Column : SubsystemModel->GetOptionalColumns())
		{
			MenuBuilder.AddMenuEntry(
				Column.Label,
				LOCTEXT("ToggleDisplayColumn_Tooltip", "Toggles display of subsystem browser columns."),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleDisplayColumn, Column.Id),
					FCanExecuteAction(),
					FIsActionChecked::CreateSP(this, &SSubsystemBrowserPanel::GetColumnDisplayStatus, Column.Id)
				),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}
	}
	MenuBuilder.EndSection();
#endif

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewCategoryGroup", "Categories"));
	{
		for (const SubsystemTreeItemPtr & Category : SubsystemModel->GetAllCategories())
		{
			check(Category->GetAsCategoryDescriptor());
			FSubsystemTreeItemID CategoryID = Category->GetID();

			MenuBuilder.AddMenuEntry(Category->GetAsCategoryDescriptor()->Label,
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
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("ViewOptionsGroup", "Options"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleColoring", "Table Coloring"),
			LOCTEXT("ToggleColoring_Tooltip", "Toggles coloring in subsystem browser tree."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleTableColoring),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SSubsystemBrowserPanel::IsTableColoringEnabled)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleGameOnly", "Only Game Modules"),
			LOCTEXT("ToggleGameOnly_Tooltip", "Show only subsystems that are within Game Modules."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleShouldShowOnlyGame),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SSubsystemBrowserPanel::ShouldShowOnlyGame)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		//MenuBuilder.AddMenuEntry(
		//	LOCTEXT("ToggleTickableOnly", "Only Tickable"),
		//	LOCTEXT("ToggleTickableOnly_Tooltip", "Show only subsystems that are tickable."),
		//	FSlateIcon(),
		//	FUIAction(
		//		FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleShouldShowOnlyTickable),
		//		FCanExecuteAction(),
		//		FIsActionChecked::CreateSP(this, &SSubsystemBrowserPanel::ShouldShowOnlyTikable)
		//	),
		//	NAME_None,
		//	EUserInterfaceActionType::ToggleButton
		//);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ToggleHiddenProps", "Show Hidden Properties"),
			LOCTEXT("ToggleHiddenProps_Tooltip", "Enforces display of all hidden object properties in details panel."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ToggleShowHiddenProperties),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &SSubsystemBrowserPanel::ShouldShowHiddenProperties)
			),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
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
}

void SSubsystemBrowserPanel::OnExpansionChanged(SubsystemTreeItemPtr Item, bool bIsItemExpanded)
{
	Item->bExpanded = bIsItemExpanded;

	if (FSubsystemTreeCategoryItem* Folder = Item->GetAsCategoryDescriptor())
	{
		for (SubsystemTreeItemPtr Child : Folder->GetChildren())
		{
			Child->bExpanded = bIsItemExpanded;
		}
	}

	// Save expansion states
	USubsystemBrowserSettings::Get()->SetTreeExpansionStates(GetParentsExpansionState());

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

EVisibility SSubsystemBrowserPanel::GetVisibilityForColumn(FName ColumnName) const
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	return Settings->GetTableColumnState(ColumnName) ? EVisibility::Visible : EVisibility::Collapsed;
}

void SSubsystemBrowserPanel::ToggleDisplayColumn(FName ColumnName)
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();

	bool OldState = Settings->GetTableColumnState(ColumnName);
	Settings->SetTableColumnState(ColumnName, !OldState);

	bNeedListRebuild = true;
	bNeedsRefresh = true;
}

bool SSubsystemBrowserPanel::GetColumnDisplayStatus(FName ColumnName) const
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	return Settings->GetTableColumnState(ColumnName);
}

bool SSubsystemBrowserPanel::IsTableColoringEnabled() const
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	return Settings->IsColoringEnabled();
}

void SSubsystemBrowserPanel::ToggleTableColoring()
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	bool bOldValue = Settings->IsColoringEnabled();
	Settings->SetColoringEnabled(!bOldValue);

	RefreshView();
}

bool SSubsystemBrowserPanel::ShouldShowHiddenProperties() const
{
	const USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	return Settings->ShouldShowHiddenProperties();
}

void SSubsystemBrowserPanel::ToggleShowHiddenProperties()
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	bool bOldValue = Settings->ShouldShowHiddenProperties();
	Settings->SetShowHiddenProperties(!bOldValue);

	RefreshView();

	ShowBrowserInfoMessage(LOCTEXT("PropertyToggleWarning", "Changes will be applied after panel restart"), SNotificationItem::CS_Pending);
}

bool SSubsystemBrowserPanel::ShouldShowOnlyGame() const
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	return Settings->ShouldShowOnlyGame();
}

void SSubsystemBrowserPanel::ToggleShouldShowOnlyGame()
{
	USubsystemBrowserSettings* Settings = USubsystemBrowserSettings::Get();
	bool bOldValue = Settings->ShouldShowOnlyGame();
	Settings->SetShouldShowOnlyGame(!bOldValue);

	FullRefresh();
}

void SSubsystemBrowserPanel::OnSelectionChanged(const SubsystemTreeItemPtr Item, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	if (bUpdatingSelection)
		return;

	bUpdatingSelection = true;

	const TArray<SubsystemTreeItemPtr>& SelectedItems = TreeWidget->GetSelectedItems();

	SetSelectedObject(SelectedItems.Num() ? SelectedItems[0] : nullptr);

	bUpdatingSelection = false;
}

const FSlateBrush* SSubsystemBrowserPanel::GetWorldssMenuBrush() const
{
	return FEditorStyle::GetBrush("WorldBrowser.LevelsMenuBrush");
}

FText SSubsystemBrowserPanel::GetCurrentWorldText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("World"), GetWorldDescription(SubsystemModel->GetCurrentWorld().Get()));
	return FText::Format(LOCTEXT("WorldsSelectButton", "World: {World}"), Args);
}

FText SSubsystemBrowserPanel::GetWorldDescription(UWorld* World) const
{
	FText Description;
	if(World)
	{
		FText PostFix;
		const FWorldContext* WorldContext = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if(Context.World() == World)
			{
				WorldContext = &Context;
				break;
			}
		}

		if (World->WorldType == EWorldType::PIE)
		{
			switch(World->GetNetMode())
			{
				case NM_Client:
					if (WorldContext)
					{
						PostFix = FText::Format(LOCTEXT("ClientPostfixFormat", "(Client {0})"), FText::AsNumber(WorldContext->PIEInstance - 1));
					}
					else
					{
						PostFix = LOCTEXT("ClientPostfix", "(Client)");
					}
					break;
				case NM_DedicatedServer:
				case NM_ListenServer:
					PostFix = LOCTEXT("ServerPostfix", "(Server)");
					break;
				case NM_Standalone:
					PostFix = LOCTEXT("PlayInEditorPostfix", "(Play In Editor)");
					break;
			}
		}
		else if(World->WorldType == EWorldType::Editor)
		{
			PostFix = LOCTEXT("EditorPostfix", "(Editor)");
		}

		Description = FText::Format(LOCTEXT("WorldFormat", "{0} {1}"), FText::FromString(World->GetFName().GetPlainNameString()), PostFix);
	}

	return Description;
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

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World && (World->WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Editor))
		{
			MenuBuilder.AddMenuEntry(
				GetWorldDescription(World),
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
	UE_LOG(LogSubsystemBrowser, Log, TEXT("On PIE Start"));

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
	UE_LOG(LogSubsystemBrowser, Log, TEXT("On PIE End"));

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

void SSubsystemBrowserPanel::SetSelectedObject(SubsystemTreeItemPtr Item)
{
	UObject* InObject = Item.IsValid() ? Item->GetObjectForDetails() : nullptr;
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Selected object %s"), *GetNameSafe(InObject));

	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(InObject);
		RefreshDetails();
	}
}

void SSubsystemBrowserPanel::ResetSelectedObject()
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Reset selected object"));

	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(nullptr);
		RefreshDetails();
	}
}

bool SSubsystemBrowserPanel::IsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent) const
{
	return false;
}

bool SSubsystemBrowserPanel::IsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	return true;
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
		if (Pair.Value->GetChildren().Num() > 0)
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

TSharedPtr<SWidget> SSubsystemBrowserPanel::ConstructSubsystemContextMenu()
{
	TSharedRef<SWidget> MenuWidget = SNullWidget::NullWidget;

	const FName MenuName = TEXT("SubsystemBrowser.ContextMenu");

	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus->IsMenuRegistered(MenuName))
	{
		ToolMenus->RegisterMenu(MenuName);
	}

	FToolMenuContext Context(nullptr, TSharedPtr<FExtender>());
	UToolMenu* Menu = ToolMenus->GenerateMenu(MenuName, Context);

	SubsystemTreeItemPtr Selected = GetFirstSelectedItem();
	if (Selected.IsValid())
	{
		Selected->GenerateContextMenu(Menu, *this);
	}

	{
		FToolMenuSection& Section = Menu->AddSection("SubsystemContextActions", LOCTEXT("SubsystemContextActions", "Common"));
		Section.AddMenuEntry("OpenSourceFile",
			LOCTEXT("OpenSourceFile", "Open Source File"),
			FText::GetEmpty(),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "SystemWideCommands.FindInContentBrowser"),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_OpenSourceFile),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::HasSelectedSubsystem)
			)
		);
	}

	{
		FToolMenuSection& Section = Menu->AddSection("SubsystemReferenceActions", LOCTEXT("SubsystemReferenceActions", "References"));
		Section.AddMenuEntry("CopyClassName",
			LOCTEXT("CopyClassName", "Copy Class Name"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_CopyClassName),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::HasSelectedSubsystem)
			)
		);
		Section.AddMenuEntry("CopyPackageName",
			LOCTEXT("CopyPackageName", "Copy Module Name"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_CopyPackageName),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::HasSelectedSubsystem)
			)
		);
		Section.AddMenuEntry("CopyScriptName",
			LOCTEXT("CopyScriptName", "Copy Script Name"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_CopyScriptName),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::HasSelectedSubsystem)
			)
		);
		Section.AddMenuEntry("CopyFilePath",
			LOCTEXT("CopyFilePath", "Copy File Path"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_CopySourceFilePath),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::HasSelectedSubsystem)
			)
		);
	}

	{
		FToolMenuSection& Section = Menu->AddSection("SubsystemConfigActions", LOCTEXT("SubsystemConfigActions", "Config"));
		Section.AddMenuEntry("ExportModified",
			LOCTEXT("ExportModified", "Export Modified Properties"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_ConfigExport, true),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::CanExportSelection)
			)
		);
		Section.AddMenuEntry("ExportAll",
			LOCTEXT("ExportAll", "Export All Properties"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::ContextMenu_ConfigExport, false),
				FCanExecuteAction::CreateSP(this, &SSubsystemBrowserPanel::CanExportSelection)
			)
		);
	}

	MenuWidget = ToolMenus->GenerateWidget(Menu);

	return MenuWidget;
}

static void SetClipboardText(const FString& ClipboardText)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Clipboard set to:\n%s"), *ClipboardText);

	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
}

void SSubsystemBrowserPanel::ContextMenu_OpenSourceFile() const
{
	const FSubsystemTreeSubsystemItem* SelectedSubsystem = GetFirstSelectedSubsystem();
	if (SelectedSubsystem)
	{
		UObject* ViewedObject = SelectedSubsystem->GetObjectForDetails();
		if (!ViewedObject || !FSourceCodeNavigation::CanNavigateToClass(ViewedObject->GetClass()))
		{
			ShowBrowserInfoMessage(LOCTEXT("OpenSourceFile_Failed", "Failed to open source file."), SNotificationItem::CS_Fail);
		}
		else
		{
			FSourceCodeNavigation::NavigateToClass(ViewedObject->GetClass());
		}
	}
}

void SSubsystemBrowserPanel::ContextMenu_CopySourceFilePath() const
{
	const FSubsystemTreeSubsystemItem* SelectedSubsystem = GetFirstSelectedSubsystem();
	if (SelectedSubsystem)
	{
		FString ClipboardText;
		for (const FString& Item : SelectedSubsystem->SourceFilePaths)
		{
			if (ClipboardText.Len() > 0)
			{
				ClipboardText += LINE_TERMINATOR;
			}

			ClipboardText += FPaths::ConvertRelativePathToFull(Item);
		}

		SetClipboardText(ClipboardText);
	}
}

void SSubsystemBrowserPanel::ContextMenu_CopyClassName() const
{
	const FSubsystemTreeSubsystemItem* SelectedSubsystem = GetFirstSelectedSubsystem();
	if (SelectedSubsystem)
	{
		FString ClipboardText;
		ClipboardText += TEXT("U");
		ClipboardText +=  SelectedSubsystem->ClassName.ToString();
		SetClipboardText(ClipboardText);
	}
}

void SSubsystemBrowserPanel::ContextMenu_CopyPackageName() const
{
	const FSubsystemTreeSubsystemItem* SelectedSubsystem = GetFirstSelectedSubsystem();
	if (SelectedSubsystem)
	{
		FString ClipboardText;
		ClipboardText +=  SelectedSubsystem->ShortPackage;
		SetClipboardText(ClipboardText);
	}
}

void SSubsystemBrowserPanel::ContextMenu_CopyScriptName() const
{
	const FSubsystemTreeSubsystemItem* SelectedSubsystem = GetFirstSelectedSubsystem();
	if (SelectedSubsystem)
	{
		FString ClipboardText;
		ClipboardText +=  SelectedSubsystem->Package;
		ClipboardText += TEXT(".");
		ClipboardText +=  SelectedSubsystem->ClassName.ToString();
		SetClipboardText(ClipboardText);
	}
}

PRAGMA_DISABLE_OPTIMIZATION
void SSubsystemBrowserPanel::ContextMenu_ConfigExport(bool bModifiedOnly) const
{
	const FSubsystemTreeSubsystemItem* SelectedSubsystem = GetFirstSelectedSubsystem();
	if (SelectedSubsystem)
	{
		FString ClipboardText;

		ClipboardText += FString::Printf(TEXT("; Should be in Default%s.ini"), *SelectedSubsystem->GetConfigClassNameString());
		ClipboardText += LINE_TERMINATOR;
		ClipboardText += FString::Printf(TEXT("[%s.%s]"), *SelectedSubsystem->Package, *SelectedSubsystem->ClassName.ToString());
		ClipboardText += LINE_TERMINATOR;

		UObject* const Subsystem  = SelectedSubsystem->Subsystem.Get();
		UClass* const Class = SelectedSubsystem->Class.Get();
		UObject* const SubsystemDefaults  = Class ? Class->GetDefaultObject() : nullptr;

		if (Subsystem && SubsystemDefaults && Class)
		{
			TArray<FProperty*> ModifiedProperties;

			for (TFieldIterator<FProperty> It(Class); It; ++It)
			{
				FProperty* Property = *It;
				if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_Deprecated | CPF_SkipSerialization))
					continue;

				if( Property->HasAnyPropertyFlags(CPF_Config) )
				{
					for( int32 Idx=0; Idx<Property->ArrayDim; Idx++ )
					{
						uint8* DataPtr      = Property->ContainerPtrToValuePtr           <uint8>((uint8*)Subsystem, Idx);
						uint8* DefaultValue = Property->ContainerPtrToValuePtrForDefaults<uint8>(Class, (uint8*)SubsystemDefaults, Idx);
						if (bModifiedOnly == false || !Property->Identical( DataPtr, DefaultValue, PPF_DeepCompareInstances))
						{
							ModifiedProperties.Add(Property);
							break;
						}
					}
				}
			}

			for (FProperty* Property : ModifiedProperties)
			{
				const TCHAR* Prefix = TEXT("");

				if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
				{
					ClipboardText += FString::Printf(TEXT("!%s=ClearArray"), *Property->GetName());
					ClipboardText += LINE_TERMINATOR;
					Prefix = TEXT("+");

					FScriptArrayHelper ArrayHelper(ArrayProperty, Subsystem);
					if (!ArrayHelper.Num())
					{
						continue;
					}
				}

				for( int32 Idx=0; Idx< Property->ArrayDim; Idx++ )
				{
					uint8* DataPtr      = Property->ContainerPtrToValuePtr           <uint8>(Subsystem, Idx);
					uint8* DefaultValue = Property->ContainerPtrToValuePtrForDefaults<uint8>(Class, SubsystemDefaults, Idx);

					FString ExportValue;
					Property->ExportTextItem(ExportValue, DataPtr, DefaultValue, nullptr, 0);

					if (ExportValue.IsEmpty())
					{
						ClipboardText += FString::Printf(TEXT("%s="), *Property->GetName());
					}
					else
					{
						ClipboardText += FString::Printf(TEXT("%s%s=%s"), Prefix, *Property->GetName(), *ExportValue);
					}
					ClipboardText += LINE_TERMINATOR;
				}
			}
		}

		SetClipboardText(ClipboardText);
	}
}
PRAGMA_ENABLE_OPTIMIZATION

bool SSubsystemBrowserPanel::CanExportSelection() const
{
	auto SelectedSubsystem = GetFirstSelectedItem();
	return SelectedSubsystem.IsValid()  && SelectedSubsystem->IsConfigExportable();
}

bool SSubsystemBrowserPanel::HasSelectedSubsystem() const
{
	auto SelectedSubsystem = GetFirstSelectedItem();
	return SelectedSubsystem.IsValid()  && SelectedSubsystem->GetAsSubsystemDescriptor();
}

void SSubsystemBrowserPanel::OnSettingsChanged(FName InPropertyName)
{
	static const FName MD_ConfigAffecsView(TEXT("ConfigAffectsView"));
	static const FName MD_ConfigAffecsColumns(TEXT("ConfigAffectsColumns"));
	static const FName MD_ConfigAffecsDetails(TEXT("ConfigAffectsDetails"));

	if (FProperty* Property = USubsystemBrowserSettings::StaticClass()->FindPropertyByName(InPropertyName))
	{
		if (Property->HasMetaData(MD_ConfigAffecsView))
		{
			bFullRefresh = true;
			RefreshView();
		}
		if (Property->HasMetaData(MD_ConfigAffecsColumns))
		{
			RefreshColumns();
		}
		if (Property->HasMetaData(MD_ConfigAffecsDetails))
		{
			RefreshDetails();
		}
	}
}


#undef LOCTEXT_NAMESPACE
