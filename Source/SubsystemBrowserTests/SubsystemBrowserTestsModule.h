// Copyright 2022, Aquanox.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSubsystemBrowserTestsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual bool SupportsDynamicReloading() override
    {
        return false;
    }
};
