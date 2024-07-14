// Copyright 2022, Aquanox.

#pragma once

#include "SubsystemBrowserFlags.h"
#include "Model/SubsystemBrowserCategory.h"

#if UE_VERSION_NEWER_THAN(5, 1, 0)

/**
 *
 */
struct SUBSYSTEMBROWSER_API FSubsystemCategory_AudioEngine : public FSubsystemCategory
{
	FSubsystemCategory_AudioEngine();
	virtual UClass* GetSubsystemClass() const override;
	virtual bool IsVisibleByDefault() const override { return false; }
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
};

#endif
