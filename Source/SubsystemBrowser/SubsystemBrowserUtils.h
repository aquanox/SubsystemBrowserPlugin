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

	struct FClassPropertyCounts
	{
		int32 NumTotal = 0;
		int32 NumVisible = 0;
		int32 NumConfig = 0;
	};

	/**
	 * Collect property display info for tooltip
	 */
	static FClassPropertyCounts GetClassPropertyCounts(UClass* InClass);

	/**
	 * @brief
	 * @param ClipboardText
	 */
	static void SetClipboardText(const FString& Attribute);

	static void ShowBrowserInfoMessage(FText InText, SNotificationItem::ECompletionState InType);

	/**
	 *
	 */
	static FString GenerateConfigExport(const struct FSubsystemTreeSubsystemItem* Item, bool bModifiedOnly);
};
