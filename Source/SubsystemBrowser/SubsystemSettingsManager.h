// Copyright 2022, Aquanox.

#pragma once

#include "ISettingsModule.h"
#include "ISettingsViewer.h"
#include "Layout/Visibility.h"
#include "Modules/ModuleManager.h"

class SWidget;
class SDockTab;
class FSpawnTabArgs;
class SSubsystemSettingsWidget;
class ISettingsEditorModel;
class ISettingsSection;

/** Holds auto-discovered subsystems with settings */
struct FDiscoveredSubsystemInfo
{
	FName ContainerName;
	FName CategoryName;
	FName SectionName;

	TSharedPtr<SSubsystemSettingsWidget> EditorWidget;
};

/**
 * Class managing all Subsystem Browser settings: primary plugin and discovered subsystems
 */
class FSubsystemSettingsManager : public ISettingsViewer
{
	static const FName SubsystemSettingsTabName;
public:

	/**
	 *
	 */
	void Register();

	/**
	 *
	 */
	void Unregister();

	/**
	 * Open editor settings tab with plugin settings pre-selected
	 */
	void SummonPluginSettingsTab();

	/**
	 * Open editor settings tab with plugin settings pre-selected
	 */
	void SummonSubsystemSettingsTab();

	/**
	 * Show a panel with subsystem settings
	 */
	virtual void ShowSettings(const FName& CategoryName, const FName& SectionName) override;

	/** Handles creating the subsystem settings tab. */
	TSharedRef<SDockTab> HandleSpawnSettingsTab(const FSpawnTabArgs& Args);

	/** */
	void HandleCategoriesChanged();
	/** Handle dynamic modules load/unload */
	void HandleModulesChanges(FName Name, EModuleChangeReason ModuleChangeReason);
	/** */
	void HandleSettingsChanged(FName Name);

	void UpdateDiscoveredSubsystems(bool bForce = false);

	void RegisterDiscoveredSubsystems(ISettingsModule& SettingsModule);
	void RegisterSubsystemSettings(ISettingsModule& SettingsModule, FName Category, UObject* Subsystem, bool bCustomUI);

	void UnregisterDiscoveredSubsystems(ISettingsModule& SettingsModule);

	TWeakPtr<ISettingsEditorModel> GetSettingsEditorModel() const { return SettingsEditorModelPtr; }
	TWeakPtr<SWidget> GetSettingsEditorWidget() const { return SettingsEditorPtr; }

private:
	// Saved instance of Settings section
	TSharedPtr<ISettingsSection> PluginSettingsSection;

	// Settings model
	TWeakPtr<ISettingsEditorModel> SettingsEditorModelPtr;
	// Settings editor
	TWeakPtr<SWidget> SettingsEditorPtr;

	//
	friend class SBTrackableDockTab;
	TWeakPtr<SWidget> TrackedSettingsWidget;

	// Tracked list of discovered settings
	TArray<FDiscoveredSubsystemInfo> DiscoveredSettings;

	// Flag to indicate settings need to be rediscovered upon next panel opening
	bool bNeedsRediscover = true;
};
