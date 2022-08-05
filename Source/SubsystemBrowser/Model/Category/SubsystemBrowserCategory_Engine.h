// Copyright 2022, Aquanox.

#pragma once

#include "Model/SubsystemBrowserCategory.h"

struct FSubsystemCategory_Engine : public FSubsystemCategory
{
	FSubsystemCategory_Engine();

	virtual void Select(UWorld* InContext, TArray<UObject*>& OutData) const override;
};
