// Copyright 2022, Aquanox.

#include "SubsystemBrowserModule.h"

#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserStyle.h"
#include "Model/Column/SubsystemBrowserColumn_Name.h"
#include "Model/Column/SubsystemBrowserColumn_Config.h"
#include "Model/Column/SubsystemBrowserColumn_Module.h"
#include "Model/Column/SubsystemBrowserColumn_Plugin.h"
#include "Model/Category/SubsystemBrowserCategory_Editor.h"
#include "Model/Category/SubsystemBrowserCategory_Engine.h"
#include "Model/Category/SubsystemBrowserCategory_GameInstance.h"
#include "Model/Category/SubsystemBrowserCategory_Player.h"
#include "Model/Category/SubsystemBrowserCategory_World.h"
#include "UI/SubsystemBrowserPanel.h"
#include "ISettingsModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

IMPLEMENT_MODULE(FSubsystemBrowserModule, SubsystemBrowser);

DEFINE_LOG_CATEGORY(LogSubsystemBrowser);

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const FName FSubsystemBrowserModule::SubsystemBrowserTabName = TEXT("SubsystemBrowserTab");
const FName FSubsystemBrowserModule::SubsystemBrowserContextMenuName = TEXT("SubsystemBrowser.ContextMenu");

FSubsystemBrowserModule::FOnGenerateTooltip FSubsystemBrowserModule::OnGenerateTooltip;
FSubsystemBrowserModule::FOnGenerateMenu FSubsystemBrowserModule::OnGenerateContextMenu;

void FSubsystemBrowserModule::StartupModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		FSubsystemBrowserStyle::Register();

#if !SUBSYSTEM_BROWSER_NOMAD_MODE

		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		LevelEditorModule.OnTabManagerChanged().AddLambda([Module = this]()
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
			if (LevelEditorTabManager.IsValid())
			{
				LevelEditorTabManager->RegisterTabSpawner(SubsystemBrowserTabName, FOnSpawnTab::CreateRaw(Module, &FSubsystemBrowserModule::HandleSpawnBrowserTab))
					.SetDisplayName(LOCTEXT("SubsystemBrowserTitle", "Subsystems"))
					.SetTooltipText(LOCTEXT("SubsystemBrowserTooltip", "Open the Subsystem Browser tab."))
					.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
					.SetIcon(FStyleHelper::GetSlateIcon(FSubsystemBrowserStyle::PanelIconName));
			}
		});

#else

		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SubsystemBrowserTabName, FOnSpawnTab::CreateRaw(this, &FSubsystemBrowserModule::HandleSpawnBrowserTab))
				.SetDisplayName(LOCTEXT("SubsystemBrowserTitle", "Subsystem Browser"))
				.SetTooltipText(LOCTEXT("SubsystemBrowserTooltip", "Open the Subsystem Browser tab."))
#if UE_VERSION_OLDER_THAN(5, 0, 0)
				.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
#else
				.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
#endif
				.SetIcon(FStyleHelper::GetSlateIcon(FSubsystemBrowserStyle::PanelIconName));

#endif

		SettingsManager.Register();

		// Register default columns and categories on startup
		RegisterDefaultDynamicColumns();
		RegisterDefaultCategories();

		//
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSubsystemBrowserModule::RegisterMenus));
	}
}

void FSubsystemBrowserModule::RegisterMenus()
{
	UToolMenus* const ToolMenus = UToolMenus::Get();

	struct Local
	{
		static void OpenSettings()
		{
			Get().GetSettingsManager().SummonSubsystemSettingsTab();
		}
	};

	ToolMenus->RegisterMenu(SubsystemBrowserContextMenuName);

	{ // Edit -> Configuration menu
		UToolMenu* Menu = ToolMenus->ExtendMenu(TEXT("MainFrame.MainMenu.Edit"));
		FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("Configuration"));

		Section.AddMenuEntry(
			TEXT("SubsystemSettings"),
			LOCTEXT("SubsystemSettingsMenuLabel", "Subsystem Settings"),
			LOCTEXT("SubsystemSettingsMenuToolTip", "Change subsystem settings"),
			FStyleHelper::GetSlateIcon(FSubsystemBrowserStyle::PanelIconName),
			FUIAction(FExecuteAction::CreateStatic(&Local::OpenSettings))
		);
	}

#if !UE_VERSION_OLDER_THAN(5,0,0)
	{ // Project Quick Settings on right side in UE 5
		UToolMenu* Menu = ToolMenus->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.LevelToolbarQuickSettings"));
		FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("ProjectSettingsSection"));

		Section.AddMenuEntry(
			TEXT("SubsystemSettings"),
			LOCTEXT("SubsystemSettingsMenuLabel", "Subsystem Settings"),
			LOCTEXT("SubsystemSettingsMenuToolTip", "Change subsystem settings"),
			FStyleHelper::GetSlateIcon(FSubsystemBrowserStyle::PanelIconName),
			FUIAction(FExecuteAction::CreateStatic(&Local::OpenSettings))
		);
	}
#endif
}

void FSubsystemBrowserModule::ShutdownModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
#if !SUBSYSTEM_BROWSER_NOMAD_MODE
		if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
		{
			LevelEditorModule->GetLevelEditorTabManager()->UnregisterTabSpawner(SubsystemBrowserTabName);
		}
#else
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SubsystemBrowserTabName);
#endif

		SettingsManager.Unregister();

		FSubsystemBrowserStyle::UnRegister();
	}
}

TSharedRef<SDockTab> FSubsystemBrowserModule::HandleSpawnBrowserTab(const FSpawnTabArgs& Args)
{
	UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;

	return SNew(SDockTab)
		.Label(LOCTEXT("SubsystemBrowserTitle", "Subsystems"))
#if SUBSYSTEM_BROWSER_NOMAD_MODE
		.TabRole(ETabRole::NomadTab)
#endif
	[
		SNew(SBorder)
		.BorderImage( FStyleHelper::GetBrush(TEXT("ToolPanel.GroupBorder")) )
		[
			SNew(SSubsystemBrowserPanel).InWorld(EditorWorld)
		]
	];
}

void FSubsystemBrowserModule::SummonSubsystemTab()
{
#if UE_VERSION_OLDER_THAN(4, 26, 0)
	FGlobalTabmanager::Get()->InvokeTab(SubsystemBrowserTabName);
#else
	FGlobalTabmanager::Get()->TryInvokeTab(SubsystemBrowserTabName);
#endif
}

const TArray<SubsystemCategoryPtr>& FSubsystemBrowserModule::GetCategories() const
{
	return Categories;
}

void FSubsystemBrowserModule::RegisterDefaultCategories()
{
	RegisterCategory<FSubsystemCategory_Engine>();
	RegisterCategory<FSubsystemCategory_Editor>();
	RegisterCategory<FSubsystemCategory_GameInstance>();
	RegisterCategory<FSubsystemCategory_World>();
	// RegisterCategory<FSubsystemCategory_Game>();
	RegisterCategory<FSubsystemCategory_Player>();
}

const TArray<SubsystemColumnPtr>& FSubsystemBrowserModule::GetDynamicColumns() const
{
	return DynamicColumns;
}

void FSubsystemBrowserModule::AddPermanentColumns(TArray<SubsystemColumnPtr>& Columns)
{
	Columns.Add(MakeShared<FSubsystemDynamicColumn_Name>());
}

void FSubsystemBrowserModule::RegisterDefaultDynamicColumns()
{
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Module>());
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Config>());
	RegisterDynamicColumn(MakeShared<FSubsystemDynamicColumn_Plugin>());
}

void FSubsystemBrowserModule::RegisterCategory(TSharedRef<FSubsystemCategory> InCategory)
{
	if (InCategory->Name.IsNone())
	{
		UE_LOG(LogSubsystemBrowser, Error, TEXT("Invalid category being registered"));
		return;
	}

	for (const SubsystemCategoryPtr& Category : Categories)
	{
		if (Category->Name == InCategory->Name)
		{
			UE_LOG(LogSubsystemBrowser, Error, TEXT("Duplicating category with name %s."), *Category->Name.ToString());
			return;
		}
	}

	Categories.Add(InCategory);

	// Sort categories according to their order
	Categories.Sort([](const SubsystemCategoryPtr& A, const SubsystemCategoryPtr& B)
	{
		return A->GetSortOrder() < B->GetSortOrder();
	});

	SettingsManager.HandleCategoriesChanged();
}

void FSubsystemBrowserModule::RemoveCategory(FName CategoryName)
{
	for (auto It = Categories.CreateIterator(); It; ++It)
	{
		if ((*It)->Name == CategoryName)
		{
			It.RemoveCurrent();
		}
	}
}

void FSubsystemBrowserModule::RegisterDynamicColumn(TSharedRef<FSubsystemDynamicColumn> InColumn)
{
	if (!FSubsystemDynamicColumn::IsValidColumnName(InColumn->Name))
	{
		UE_LOG(LogSubsystemBrowser, Error, TEXT("Invalid column being registered"));
		return;
	}

	for (SubsystemColumnPtr& Column : DynamicColumns)
	{
		if (Column->Name == InColumn->Name)
		{
			UE_LOG(LogSubsystemBrowser, Error, TEXT("Duplicating column with name %s."), *Column->Name.ToString());
			return;
		}
	}

	DynamicColumns.Add(InColumn);

	// Sort columns by order
	DynamicColumns.StableSort(SubsystemColumnSorter());
}

#undef LOCTEXT_NAMESPACE
