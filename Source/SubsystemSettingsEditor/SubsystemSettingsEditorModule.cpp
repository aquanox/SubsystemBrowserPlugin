// Copyright 2022, Aquanox.

#include "SubsystemSettingsEditorModule.h"
#include "SubsystemBrowserFlags.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserStyle.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

IMPLEMENT_MODULE(FSubsystemSettingsEditorModule, SubsystemSettingsEditor);

DEFINE_LOG_CATEGORY(LogSubsystemSettingsEditor);

void FSubsystemSettingsEditorModule::StartupModule()
{
	if (GIsEditor
		&& !IsRunningCommandlet()
		&& USubsystemBrowserSettings::Get()->ShouldUseSubsystemSettings())
	{
		bEnabled = true;

		SettingsManager.Register();

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSubsystemSettingsEditorModule::RegisterMenus));
	}
}

void FSubsystemSettingsEditorModule::ShutdownModule()
{
	if (GIsEditor && bEnabled)
	{
		SettingsManager.Unregister();

		bEnabled = false;
	}
}

void FSubsystemSettingsEditorModule::RegisterMenus()
{
	struct Local
	{
		static void OpenSettings()
		{
			ISettingsModule& Module = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));
			Module.ShowViewer(TEXT("Subsystem"), TEXT(""), TEXT(""));
		}
	};

	UToolMenus* const ToolMenus = UToolMenus::Get();

	{ // Edit -> Configuration menu
		UToolMenu* Menu = ToolMenus->ExtendMenu(TEXT("MainFrame.MainMenu.Edit"));
		FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("Configuration"));

		Section.AddMenuEntry(
			TEXT("SubsystemSettings"),
			LOCTEXT("SubsystemSettingsMenuLabel", "Subsystem Settings"),
			LOCTEXT("SubsystemSettingsMenuToolTip", "Change subsystem settings"),
			FStyleHelper::GetSlateIcon(FStyleHelper::PanelIconName),
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
			FStyleHelper::GetSlateIcon(FStyleHelper::PanelIconName),
			FUIAction(FExecuteAction::CreateStatic(&Local::OpenSettings))
		);
	}
#endif
}


#undef LOCTEXT_NAMESPACE
