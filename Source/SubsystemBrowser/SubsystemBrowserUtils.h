// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"

struct FSubsystemBrowserUtils
{
	DECLARE_DELEGATE_RetVal_OneParam(FString, FOnGetSubsystemOwnerName, UObject*);
	static SUBSYSTEMBROWSER_API FOnGetSubsystemOwnerName OnGetSubsystemOwnerName;

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
	 * Test if class has usable properties to be displayed in details panel
	 */
	 static bool HasPropertiesToDisplay(UClass* InClass);
};
