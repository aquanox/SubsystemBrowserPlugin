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
	 * Get "Smart metadata" property.
	 *
	 * Performs a lookup for possible function/property match, if none found uses raw string value.
	 *
	 * Functions should be a UFUNCTION() returning FString or FText
	 * Properties should be a UPROPERTY() of FString or FText type
	 *
 	 */
	static TOptional<FString> GetSmartMetaValue(UObject* InObject, const FName& InName, bool bHierarchical = false, bool bWarn = false);

	/**
	 * Get info about subsystem "Owner"
	 */
	static FString GetSubsystemOwnerName(UObject* InObject);

	/**
	 * Finds the base directory for a given module.
	 */
	static FString GetModulePathForClass(UClass* InClass);

	/**
	 * Find fully qualified class module name
	 */
	static bool GetModuleDetailsForClass(UClass* InClass, FString& OutName, bool& OutGameFlag);

	/**
	 * Find plugin name that contains class
	 */
	static bool GetPluginDetailsForClass(UClass* InClass, FString& OutName, FString& OutFriendlyName);

	/**
	 * Collect related source files belonging to specified class
	 */
	static void CollectSourceFiles(UClass* InClass, TArray<FString>& OutSourceFiles);

	struct FClassFieldStats
	{
		int32 NumProperties = 0;
		int32 NumEditable = 0;
		int32 NumVisible = 0;
		// number of properties with Config flag
		int32 NumConfig = 0;
		// number of properties with Config flag that are Editable
		int32 NumConfigWithEdit = 0;
		// number of CallInEditor functions
		int32 NumCallable = 0;
	};

	/**
	 * Collect property display info for tooltip
	 */
	static FClassFieldStats GetClassFieldStats(UClass* InClass);

	/**
	 * Find metadata value in class
	 */
	static TOptional<FString> GetMetadataOptional(UClass* InClass, FName InKey);

	/**
	 * Find metadata value in entire hierarchy of classes
	 */
	static TOptional<FString> GetMetadataHierarchical(UClass* InClass, FName InKey);

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
	 * @param Item
	 * @param bModifiedOnly
	 * @return
	 */
	static FString GenerateConfigExport(UObject* Item, bool bModifiedOnly);

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

	/**
	 * Build a text description for a world
	 */
	static FText GetWorldDescription(const UWorld* World);
};
