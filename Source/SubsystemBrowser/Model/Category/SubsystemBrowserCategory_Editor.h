// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserCategory.h"

struct FSubsystemCategory_Editor : public FSubsystemCategory
{
	FSubsystemCategory_Editor();
	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
};
