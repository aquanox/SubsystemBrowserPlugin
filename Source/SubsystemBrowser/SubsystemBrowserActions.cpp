#include "SubsystemBrowserActions.h"

#include "Interfaces/IMainFrameModule.h"
#include "Framework/Commands/UICommandList.h"
#include "Modules/ModuleManager.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserStyle.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemBrowserActions::FSubsystemBrowserActions()
	: TCommands<FSubsystemBrowserActions>
	(
		TEXT("SubsystemBrowser"),
		LOCTEXT("SubsystemBrowserCommandTitle", "Subsystem Browser"),
		NAME_None,
		FStyleHelper::GetStyleSetName()
	)
{
}

void FSubsystemBrowserActions::RegisterCommands()
{
	UI_COMMAND( QuickOpenSubsystemBrowser, "Open Subsystem Browser", "Open and Focus Subsystem Browser Panel.", EUserInterfaceActionType::Button, FInputChord() );
	UI_COMMAND( QuickOpenSubsystemSettings, "Open Subsystem Settings", "Open and Focus Subsystem Settings Panel.", EUserInterfaceActionType::Button, FInputChord() );
	UI_COMMAND( QuickOpenPluginSettings, "Open Plugin Settings", "Open and Focus Plugin Settings Panel.", EUserInterfaceActionType::Button, FInputChord() );
}

const FSubsystemBrowserActions& FSubsystemBrowserActions::Get()
{
	return TCommands<FSubsystemBrowserActions>::Get();
}

namespace SubsystemBrowserActions
{
	static void Action_QuickOpenSubsystemBrowser()
	{
		FSubsystemBrowserModule::Get().SummonSubsystemTab();
	}

	static void Action_QuickOpenSubsystemSettings()
	{
		FSubsystemBrowserModule::Get().SummonSubsystemSettingsTab();
	}

	static void Action_QuickOpenPluginSettings()
	{
		FSubsystemBrowserModule::Get().SummonPluginSettingsTab();
	}
}

void FSubsystemBrowserActions::MapDefaultActions() const
{
	IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

	// Register editor global shortcuts in main frame
	TSharedRef<FUICommandList> MainFrameActions = MainFrame.GetMainFrameCommandBindings();
	MainFrameActions->MapAction(QuickOpenSubsystemBrowser,
		FUIAction(FExecuteAction::CreateStatic(&SubsystemBrowserActions::Action_QuickOpenSubsystemBrowser)));
	MainFrameActions->MapAction(QuickOpenSubsystemSettings,
		FUIAction(FExecuteAction::CreateStatic(&SubsystemBrowserActions::Action_QuickOpenSubsystemSettings)));
	MainFrameActions->MapAction(QuickOpenPluginSettings,
		FUIAction(FExecuteAction::CreateStatic(&SubsystemBrowserActions::Action_QuickOpenPluginSettings)));

	// Register panel-specific shortcuts in level editor
	FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedRef<FUICommandList> LevelEditorActions = LevelEditor.GetGlobalLevelEditorActions();

	// TBD
}

void FSubsystemBrowserActions::UnmapDefaultActions() const
{
	IMainFrameModule* MainFrame = FModuleManager::GetModulePtr<IMainFrameModule>(TEXT("MainFrame"));
	if (MainFrame)
	{
		TSharedRef<FUICommandList> CmdList = MainFrame->GetMainFrameCommandBindings();

		CmdList->UnmapAction(QuickOpenSubsystemBrowser);
		CmdList->UnmapAction(QuickOpenSubsystemSettings);
		CmdList->UnmapAction(QuickOpenPluginSettings);
	}

	FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
	if (LevelEditor)
	{
		TSharedRef<FUICommandList> CmdList = LevelEditor->GetGlobalLevelEditorActions();

		// TBD
	}
}

#undef LOCTEXT_NAMESPACE
