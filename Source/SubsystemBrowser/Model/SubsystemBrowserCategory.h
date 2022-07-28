// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"

/**
 * Subsystem data provider delegate
 */
DECLARE_DELEGATE_TwoParams(FEnumSubsystemsDelegate, UWorld* /* InContext */, TArray<UObject*>& /* OutData */);

/**
 * Represents a type of subsystem that will be shown in browser
 */
struct SUBSYSTEMBROWSER_API FSubsystemCategory : public TSharedFromThis<FSubsystemCategory>
{
	/* Category config identifier */
	FName Name;
	/* Category display title */
	FText Label;
	/* Data supplier function */
	FEnumSubsystemsDelegate Selector;
	/* Sort weight for the category (with 0 being topmost, 1000 bottom last) */
	int32 SortOrder = 0;

	FSubsystemCategory() = default;
	FSubsystemCategory(const FName& Name, const FText& Label, const FEnumSubsystemsDelegate& Selector, int32 SortOrder = 0);
	virtual ~FSubsystemCategory() = default;

	/* Select subsystems for the respected category */
	virtual TArray<UObject*> Select(UWorld* InContext) const;

	/* Event that fires when subsystems of this category need to be refreshed
	 * Common case: engine and editor subsystems that dynamically load with modules
	 */
	/* FOnSubsystemsModified& OnSubsystemsModified(); */
};

using SubsystemCategoryPtr = TSharedPtr<FSubsystemCategory>;

