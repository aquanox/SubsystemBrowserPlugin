// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserCategory.h"

struct FSubsystemCategory_World : public FSubsystemCategory
{
	FSubsystemCategory_World();
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
};
