// Copyright 2022, Aquanox.

#pragma once

#include "Templates/SharedPointer.h"
#include "Engine/World.h"

/**
 * Represents a type of subsystem that will be shown in browser
 */
struct SUBSYSTEMBROWSER_API FSubsystemCategory : public TSharedFromThis<FSubsystemCategory>
{
	/* Category config identifier */
	FName Name;
	/* Category display in Settings */
	FName SettingsName;
	/* Category display title */
	FText Label;
	/* Sort weight for the category (with 0 being topmost, 1000 bottom last) */
	int32 SortOrder = 0;

	FSubsystemCategory() = default;
	FSubsystemCategory(const FName& Name, const FText& Label, int32 SortOrder);
	virtual ~FSubsystemCategory() = default;

	const FName& GetID() const { return Name; }
	const FName& GetSettingsName() const { return SettingsName; }
	const FText& GetDisplayName() const { return Label; }
	int32 GetSortOrder() const { return SortOrder; }

	// Is this category displayed in Browser Panel by default?
	// Category will still be registered and displayed in category selector
	virtual bool IsVisibleByDefaultInBrowser() const = 0;
	
	// Is this category displayed in Settings Panel?
	virtual bool IsVisibleInSettings() const = 0;

	/* Select subsystems for browser panel display */
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const = 0;
	
	/* Select subsystems for settings display */
	virtual void SelectSettings(TArray<UObject*>& OutData) const = 0;
};

using SubsystemCategoryPtr = TSharedPtr<FSubsystemCategory>;
using SubsystemCategoryConstPtr = TSharedPtr<const FSubsystemCategory>;

/**
 * Subsystem data provider delegate
 */
DECLARE_DELEGATE_TwoParams(FEnumSubsystemsDelegate, UWorld* /* InContext */, TArray<UObject*>& /* OutData */);
DECLARE_DELEGATE_OneParam(FEnumSubsystemConfigsDelegate, TArray<UObject*>& /* OutData */);

/**
 * Basic implementation of category that takes selector delegate.
 *
 * This is mostly an example, inheriting FSubsystemCategory is preferred.
 */
struct SUBSYSTEMBROWSER_API FSimpleSubsystemCategory : public FSubsystemCategory
{
	/* Data supplier function */
	FEnumSubsystemsDelegate Selector;
	/* Data supplier function */
	FEnumSubsystemConfigsDelegate ConfigSelector;

	FSimpleSubsystemCategory() = default;
	
	virtual bool IsVisibleByDefaultInBrowser() const override { return Selector.IsBound(); }
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
	
	virtual bool IsVisibleInSettings() const override { return ConfigSelector.IsBound(); }
	virtual void SelectSettings(TArray<UObject*>& OutData) const override;
};
