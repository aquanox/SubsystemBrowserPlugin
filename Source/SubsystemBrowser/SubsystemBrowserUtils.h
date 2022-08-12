// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"

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

	/**
	 * Collect property display info for tooltip
	 */
	static void GetClassPropertyCounts(UClass* InClass, int32& NumTotal, int32& NumVisible);

	/**
	 *
	 */
	static FString GenerateConfigExport(const struct FSubsystemTreeSubsystemItem* Item, bool bModifiedOnly);
};
