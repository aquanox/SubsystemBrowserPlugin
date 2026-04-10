#pragma once

#include "Framework/Commands/Commands.h"
#include "LevelEditor.h"
#include "LevelEditorActions.h"

class FUICommandInfo;

class FSubsystemBrowserActions : public TCommands<FSubsystemBrowserActions>
{
public:
	FSubsystemBrowserActions();

	virtual void RegisterCommands() override;

	static const FSubsystemBrowserActions& Get();

	void MapDefaultActions() const;
	void UnmapDefaultActions() const;

	// Global
	TSharedPtr<FUICommandInfo> QuickOpenSubsystemBrowser;
	TSharedPtr<FUICommandInfo> QuickOpenSubsystemSettings;
	TSharedPtr<FUICommandInfo> QuickOpenPluginSettings;

	// Subsystem Browser Panel
	// TSharedPtr<FUICommandInfo> WorldSwitchDefault;
	// TSharedPtr<FUICommandInfo> WorldSwitchNext;
	// TSharedPtr<FUICommandInfo> WorldSwitchPrev;

	// Subsystem browser Details Panel
	// TSharedPtr<FUICommandInfo> QuickToggleHiddenPropertiesDisplay;
	// TSharedPtr<FUICommandInfo> QuickToggleSubobjectsDisplay;
};
