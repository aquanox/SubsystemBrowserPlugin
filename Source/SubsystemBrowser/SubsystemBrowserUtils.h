// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

/**
 * Collection of various helpers to use
 */
struct SUBSYSTEMBROWSER_API FSubsystemBrowserUtils
{
	/**
	 * Get info about subsystem "Owner"
	 */
	static FString GetDefaultSubsystemOwnerName(UObject* InObject);

	/**
	 * Finds the base directory for a given module.
	 */
	static FString GetModulePathForClass(UClass* InClass);

	/**
	 * Find fully qualified class module name
	 */
	static FString GetModuleNameForClass(UClass* InClass);

	/**
	 * Find plugin name that contains class
	 */
	static TSharedPtr<class IPlugin> GetPluginForClass(UClass* InClass);

	/**
	 * Test if class is belongs to a Game Module
	 */
	static bool IsGameModuleClass(UClass* InClass);

	/**
	 * Collect related source files belonging to specified class
	 */
	static void CollectSourceFiles(UClass* InClass, TArray<FString>& OutSourceFiles);

	struct FClassFieldStats
	{
		int32 NumProperties = 0;
		int32 NumEditable = 0;
		int32 NumVisible = 0;
		int32 NumConfig = 0;
		int32 NumCallable = 0;
	};

	/**
	 * Collect property display info for tooltip
	 */
	static FClassFieldStats GetClassFieldStats(UClass* InClass);

	/**
	 * Put text into clipboard
	 */
	static void SetClipboardText(const FString& ClipboardText);

	/**
	 * @brief
	 * @param InText
	 * @param InType
	 */
	static void ShowBrowserInfoMessage(FText InText, SNotificationItem::ECompletionState InType);

	/**
	 * @brief Generate config export string for specified item
	 * @param Item
	 * @param bModifiedOnly
	 * @return
	 */
	static FString GenerateConfigExport(const struct FSubsystemTreeSubsystemItem* Item, bool bModifiedOnly);

	/**
	 *
	 */
	static bool TryUpdateDefaultConfigFile(UObject* Object);

	/**
	 * Dump class flags to output
	 *
	 * Example: `SB.PrintClass /Script/SubsystemBrowser.SubsystemBrowserTestSubsystem`
	 */
	static void PrintClassDetails(const TArray< FString >& InArgs, UWorld* InWorld, FOutputDevice& InLog);

	/**
	 * Dump property flags to output
	 *
	 * Example: `SB.PrintProperty /Script/SubsystemBrowser.SubsystemBrowserTestSubsystem SingleDelegate`
	 */
	static void PrintPropertyDetails(const TArray< FString >& InArgs, UWorld* InWorld, FOutputDevice& InLog);
};
