// Copyright 2022, Aquanox.

#pragma once
#include "Model/SubsystemBrowserCategory.h"

struct FSubsystemCategory_GameInstance : public FSubsystemCategory
{
	FSubsystemCategory_GameInstance();
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
};
