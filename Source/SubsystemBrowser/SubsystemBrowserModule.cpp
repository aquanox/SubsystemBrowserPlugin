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
#include "Model/Category/SubsystemBrowserCategory_AudioEngine.h"
#include "UI/SubsystemBrowserPanel.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Widgets/Docking/SDockTab.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

IMPLEMENT_MODULE(FSubsystemBrowserModule, SubsystemBrowser);

DEFINE_LOG_CATEGORY(LogSubsystemBrowser);

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const FName FSubsystemBrowserModule::SubsystemBrowserTabName(TEXT("SubsystemBrowserTab"));
const FName FSubsystemBrowserModule::SubsystemBrowserNomadTabName(TEXT("SubsystemBrowserNomadTab"));
const FName FSubsystemBrowserModule::SubsystemBrowserContextMenuName(TEXT("SubsystemBrowser.ContextMenu"));

FSubsystemBrowserModule::FOnGenerateTooltip FSubsystemBrowserModule::OnGenerateTooltip;
FSubsystemBrowserModule::FOnGenerateMenu FSubsystemBrowserModule::OnGenerateContextMenu;

void FSubsystemBrowserModule::StartupModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		FSubsystemBrowserStyle::Register();

		bNomadModeActive = USubsystemBrowserSettings::Get()->ShouldUseNomadMode();

		if (!bNomadModeActive)
		{ // register as a normal panel within level editor 
			FLevelEditorModule& LevelEd = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			LevelEd.OnTabManagerChanged().AddLambda([Module = this]()
			{
				FLevelEditorModule& LevelEdInner = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
				TSharedPtr<FTabManager> LevelEditorTabManager = LevelEdInner.GetLevelEditorTabManager();
				if (LevelEditorTabManager.IsValid())
				{
					LevelEditorTabManager->RegisterTabSpawner(SubsystemBrowserTabName, FOnSpawnTab::CreateRaw(Module, &FSubsystemBrowserModule::HandleSpawnBrowserTab))
						.SetDisplayName(LOCTEXT("SubsystemBrowserTabTitle", "Subsystems"))
						.SetTooltipText(LOCTEXT("SubsystemBrowserTabTooltip", "Open the Subsystem Browser tab."))
						.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
						.SetIcon(FStyleHelper::GetSlateIcon(FStyleHelper::PanelIconName));
				}
			});
		}
		else
		{ // register as a nomad tab within global tab manager
			FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SubsystemBrowserNomadTabName, FOnSpawnTab::CreateRaw(this, &FSubsystemBrowserModule::HandleSpawnBrowserTab))
					.SetDisplayName(LOCTEXT("SubsystemBrowserNomadTabTitle", "Subsystem Browser"))
					.SetTooltipText(LOCTEXT("SubsystemBrowserNomadTabTooltip", "Open the Subsystem Browser tab."))
	#if UE_VERSION_OLDER_THAN(5, 0, 0)
					.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
	#else
					.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
	#endif
					.SetIcon(FStyleHelper::GetSlateIcon(FStyleHelper::PanelIconName));
		}

		// Register default columns and categories on startup
		RegisterDefaultDynamicColumns();
		RegisterDefaultCategories();

		// Register plugin settings
		RegisterSettings();

		//
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSubsystemBrowserModule::RegisterMenus));
	}
}

void FSubsystemBrowserModule::RegisterSettings()
{
	ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));

	// Setup plugin settings panel in Editor Settings
	USubsystemBrowserSettings* SettingsObject = USubsystemBrowserSettings::Get();
	PluginSettingsSection = SettingsModule.RegisterSettings(
		TEXT("Editor"), TEXT("Plugins"), TEXT("SubsystemBrowser"),
		LOCTEXT("SubsystemBrowserSettingsName", "Subsystem Browser"),
		LOCTEXT("SubsystemBrowserSettingsDescription", "Settings for Subsystem Browser Plugin"),
		SettingsObject
	);
	PluginSettingsSection->OnSelect().BindUObject(SettingsObject, &USubsystemBrowserSettings::OnSettingsSelected);
	PluginSettingsSection->OnResetDefaults().BindUObject(SettingsObject, &USubsystemBrowserSettings::OnSettingsReset);
}

void FSubsystemBrowserModule::RegisterMenus()
{
	UToolMenus* const ToolMenus = UToolMenus::Get();
	if (ToolMenus)
	{
		ToolMenus->RegisterMenu(SubsystemBrowserContextMenuName);
	}
}

void FSubsystemBrowserModule::ShutdownModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		PluginSettingsSection.Reset();

		if (!bNomadModeActive)
		{
			if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
			{
				LevelEditorModule->GetLevelEditorTabManager()->UnregisterTabSpawner(SubsystemBrowserTabName);
			}
		}
		else
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SubsystemBrowserNomadTabName);
		}
		
		FSubsystemBrowserStyle::UnRegister();
	}
}

TSharedRef<SDockTab> FSubsystemBrowserModule::HandleSpawnBrowserTab(const FSpawnTabArgs& Args)
{
	UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;

	return SNew(SDockTab)
		.Label(LOCTEXT("SubsystemBrowserTitle", "Subsystems"))
		.TabRole(bNomadModeActive ? ETabRole::NomadTab : ETabRole::PanelTab)
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
	const FName& TabName = bNomadModeActive ? SubsystemBrowserNomadTabName : SubsystemBrowserTabName;
	
#if UE_VERSION_OLDER_THAN(4, 26, 0)
	FGlobalTabmanager::Get()->InvokeTab(TabName);
#else
	FGlobalTabmanager::Get()->TryInvokeTab(TabName);
#endif
}

void FSubsystemBrowserModule::SummonPluginSettingsTab()
{
	ISettingsModule& Module = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));
    Module.ShowViewer(TEXT("Editor"), TEXT("Plugins"), TEXT("SubsystemBrowser"));
}

void FSubsystemBrowserModule::SummonSubsystemSettingsTab()
{
	if (USubsystemBrowserSettings::Get()->ShouldUseSubsystemSettings())
	{
		ISettingsModule& Module = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));
		Module.ShowViewer(TEXT("Subsystem"), TEXT(""), TEXT(""));
	}
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

#if UE_VERSION_NEWER_THAN(5, 1, 0)
	RegisterCategory<FSubsystemCategory_AudioEngine>();
#endif
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
