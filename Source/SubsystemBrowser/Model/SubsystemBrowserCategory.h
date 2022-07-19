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
struct SUBSYSTEMBROWSER_API FSubsystemCategory
{
	/* Category config identifier */
	FName Name;
	/* Category display title */
	FText Label;
	/* Subsystem base class (optional) */
	TWeakObjectPtr<UClass> SubsystemClass;
	/* Data supplier function */
	FEnumSubsystemsDelegate Selector;
	/* Sort weight for the category (with 0 being topmost, 1000 bottom last) */
	int32 SortOrder = 0;
};

using SubsystemCategoryPtr = TSharedPtr<FSubsystemCategory>;

