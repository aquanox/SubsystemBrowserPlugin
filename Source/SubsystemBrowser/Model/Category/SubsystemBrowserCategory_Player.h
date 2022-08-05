// Copyright 2022, Aquanox.

#pragma once
#include "Model/SubsystemBrowserCategory.h"

struct FSubsystemCategory_Player: public FSubsystemCategory
{
	FSubsystemCategory_Player();
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
};
