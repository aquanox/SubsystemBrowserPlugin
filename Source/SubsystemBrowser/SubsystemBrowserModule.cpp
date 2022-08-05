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
#include "ISettingsSection.h"
#include "Widgets/Docking/SDockTab.h"
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

IMPLEMENT_MODULE(FSubsystemBrowserModule, SubsystemBrowser);

DEFINE_LOG_CATEGORY(LogSubsystemBrowser);

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const FName FSubsystemBrowserModule::SubsystemBrowserTabName = TEXT("SubsystemBrowserTab");

#if SB_UE_VERSION_NEWER_OR_SAME(5, 0, 0)
static const FName PanelIconName(TEXT("Icons.Settings"));
#else
static const FName PanelIconName(TEXT("LevelEditor.GameSettings.Small"));
#endif

void FSubsystemBrowserModule::StartupModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		FSubsystemBrowserStyle::Register();

		USubsystemBrowserSettings* SettingsObject = USubsystemBrowserSettings::Get();

		ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>("Settings");
		SettingsSection = SettingsModule.RegisterSettings(
			TEXT("Editor"), TEXT("Plugins"), TEXT("SubsystemBrowser"),
			LOCTEXT("SubsystemBrowserSettingsName", "Subsystem Browser"),
			LOCTEXT("SubsystemBrowserSettingsDescription", "Settings for Subsystem Browser Plugin"),
			SettingsObject
		);
		SettingsSection->OnSelect().BindUObject(SettingsObject, &USubsystemBrowserSettings::OnSettingsSelected);
		//SettingsSection->OnResetDefaults().BindUObject(SettingsObject, &USubsystemBrowserSettings::OnSettingsReset);

		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		LevelEditorModule.OnTabManagerChanged().AddLambda([ &LevelEditorModule ]()
		{
			TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
			if (LevelEditorTabManager.IsValid())
			{
				LevelEditorTabManager->RegisterTabSpawner(SubsystemBrowserTabName, FOnSpawnTab::CreateStatic(&FSubsystemBrowserModule::HandleTabManagerSpawnTab))
					.SetDisplayName(LOCTEXT("SubsystemBrowserTitle", "Subsystems"))
					.SetTooltipText(LOCTEXT("SubsystemBrowserTooltip", "Open the Subsystem Browser tab."))
					.SetGroup( WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory() )
					.SetIcon( FSlateIcon(FEditorStyle::GetStyleSetName(), PanelIconName) );
			}
		});

		// Register default columns and categories on startup
		RegisterDefaultDynamicColumns();
		RegisterDefaultCategories();
	}
}

void FSubsystemBrowserModule::ShutdownModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
		if (LevelEditorModule)
		{
			LevelEditorModule->GetLevelEditorTabManager()->UnregisterTabSpawner(SubsystemBrowserTabName);
		}

		FSubsystemBrowserStyle::UnRegister();
	}
}

TSharedRef<SDockTab> FSubsystemBrowserModule::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		.Icon(FEditorStyle::GetBrush(PanelIconName))
#endif
		.Label(LOCTEXT("SubsystemBrowserTitle", "Subsystems"))
	[
		SNew(SBorder)
		//.Padding(4)
		.BorderImage( FEditorStyle::GetBrush("ToolPanel.GroupBorder") )
		[
			CreateSubsystemBrowser(Args)
		]
	];
}

TSharedRef<SWidget> FSubsystemBrowserModule::CreateSubsystemBrowser(const FSpawnTabArgs& Args)
{
	UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	return SNew(SSubsystemBrowserPanel).InWorld(EditorWorld);
}

void FSubsystemBrowserModule::SummonSubsystemTab()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<ILevelEditor> LevelEditorInstance = LevelEditorModule.GetLevelEditorInstance().Pin();
#if UE_VERSION_OLDER_THAN(4, 26, 0)
	FGlobalTabmanager::Get()->InvokeTab(SubsystemBrowserTabName);
#else
	FGlobalTabmanager::Get()->TryInvokeTab(SubsystemBrowserTabName);
#endif
}

void FSubsystemBrowserModule::SummonPluginSettingsTab()
{
	ISettingsModule& Module = FModuleManager::GetModuleChecked<ISettingsModule>("Settings");
	Module.ShowViewer(TEXT("Editor"), TEXT("Plugins"), TEXT("SubsystemBrowser"));
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
