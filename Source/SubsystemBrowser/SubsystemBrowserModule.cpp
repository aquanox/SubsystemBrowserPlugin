// Copyright 2022, Aquanox.

#include "SubsystemBrowserModule.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "UI/SubsystemBrowserPanel.h"
#include "LevelEditor.h"
#include "SubsystemBrowserSettings.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

IMPLEMENT_MODULE(FSubsystemBrowserModule, SubsystemBrowser);

DEFINE_LOG_CATEGORY(LogSubsystemBrowser);

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

const FName FSubsystemBrowserModule::SubsystemBrowserTabName = TEXT("SubsystemBrowserTab");
constexpr bool RegisterConfigPanel = true;

void FSubsystemBrowserModule::StartupModule()
{
	if (GIsEditor && !IsRunningCommandlet())
	{
		ISettingsModule& SettingsModule = FModuleManager::GetModuleChecked<ISettingsModule>("Settings");
		if (RegisterConfigPanel)
		{
			USubsystemBrowserSettings* SettingsObject = USubsystemBrowserSettings::Get();

			SettingsSection = SettingsModule.RegisterSettings(
				TEXT("Editor"), TEXT("ContentEditors"), TEXT("Subsystem Browser"),
				LOCTEXT("SubsystemBrowserSettingsName", "Subsystem Browser"),
				LOCTEXT("SubsystemBrowserSettingsDescription", "Settings for Subsystem Browser Plugin"),
				SettingsObject
			);
			SettingsSection->OnSelect().BindUObject(SettingsObject, &USubsystemBrowserSettings::OnSettingsSelected);
			//SettingsSection->OnModified().BindUObject(SettingsObject, &USubsystemBrowserSettings::OnSettingsModified);
		}

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
					.SetIcon( FSlateIcon(FEditorStyle::GetStyleSetName(), SSubsystemBrowserPanel::PanelIconName) );
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
	}
}

TSharedRef<SDockTab> FSubsystemBrowserModule::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
#if ENGINE_MAJOR_VERSION < 5
		.Icon(FEditorStyle::GetBrush(SSubsystemBrowserPanel::PanelIconName))
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

TSharedRef<SWidget> FSubsystemBrowserModule::CreateSubsystemBrowser(const class FSpawnTabArgs& Args)
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	return SNew(SSubsystemBrowserPanel).InWorld(EditorWorld);
}

void FSubsystemBrowserModule::SummonSubsystemTab()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	auto LevelEditorInstance = LevelEditorModule.GetLevelEditorInstance().Pin();
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	FGlobalTabmanager::Get()->InvokeTab(SubsystemBrowserTabName);
#else
	FGlobalTabmanager::Get()->TryInvokeTab(SubsystemBrowserTabName);
#endif
}

#undef LOCTEXT_NAMESPACE
