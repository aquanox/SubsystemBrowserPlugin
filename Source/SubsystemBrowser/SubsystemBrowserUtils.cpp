// Copyright 2022, Aquanox.

#include "SubsystemBrowserUtils.h"

#include "SubsystemBrowserFlags.h"
#include "SourceCodeNavigation.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/IPluginManager.h"

FSubsystemBrowserUtils::FOnGetSubsystemOwnerName FSubsystemBrowserUtils::OnGetSubsystemOwnerName;

FString FSubsystemBrowserUtils::GetSubsystemOwnerName(UObject* Instance)
{
	if (OnGetSubsystemOwnerName.IsBound())
	{
		return OnGetSubsystemOwnerName.Execute(Instance);
	}

	if (auto PlayerSubsystem = Cast<ULocalPlayerSubsystem>(Instance))
	{
		if (auto LocalPlayer = PlayerSubsystem->GetLocalPlayer<ULocalPlayer>())
		{
			return LocalPlayer->GetName();
		}
	}

	return FString();
}

FString FSubsystemBrowserUtils::GetModulePathForClass(UClass* InClass)
{
	FString ModulePath;
	if (!FSourceCodeNavigation::FindModulePath(InClass->GetOuterUPackage(), ModulePath))
	{
		return TEXT("");
	}
	return ModulePath;
}

FString FSubsystemBrowserUtils::GetModuleNameForClass(UClass* InClass)
{
	FString ModuleName;
	if (!FSourceCodeNavigation::FindClassModuleName(InClass, ModuleName))
	{
		return TEXT("Unknown");
	}
	return ModuleName;
}

TSharedPtr<class IPlugin> FSubsystemBrowserUtils::GetPluginForClass(UClass* InClass)
{
	// Find module name from class
	if( InClass )
	{
		UPackage* ClassPackage = InClass->GetOuterUPackage();

		if( ClassPackage )
		{
			//@Package name transition
			FName ShortClassPackageName = FPackageName::GetShortFName(ClassPackage->GetFName());

			for (TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
			{
				for (const FModuleDescriptor& ModuleDescriptor : Plugin->GetDescriptor().Modules)
				{
					if (ModuleDescriptor.Name == ShortClassPackageName)
					{
						return Plugin;
					}
				}
			}
		}
	}

	return nullptr;
}

bool FSubsystemBrowserUtils::IsGameModuleClass(UClass* InClass)
{
	bool bResult = false;
	// Find module name from class
	if( InClass )
	{
		UPackage* ClassPackage = InClass->GetOuterUPackage();

		if( ClassPackage )
		{
			//@Package name transition
			FName ShortClassPackageName = FPackageName::GetShortFName(ClassPackage->GetFName());

			// Is this module loaded?  In many cases, we may not have a loaded module for this class' package,
			// as it might be statically linked into the executable, etc.
			if( FModuleManager::Get().IsModuleLoaded( ShortClassPackageName ) )
			{
				// Because the module loaded into memory may have a slightly mutated file name (for
				// hot reload, etc), we ask the module manager for the actual file name being used.  This
				// is important as we need to be sure to get the correct symbols.
				FModuleStatus ModuleStatus;
				if( ensure( FModuleManager::Get().QueryModule( ShortClassPackageName, ModuleStatus ) ) )
				{
					bResult = ModuleStatus.bIsGameModule;
				}
			}
		}
	}
	return bResult;
}

void FSubsystemBrowserUtils::CollectSourceFiles(UClass* InClass, TArray<FString>& OutSourceFiles)
{
	OutSourceFiles.Empty();

	if (InClass)
	{
		FString ClassHeaderPath;
		if(FSourceCodeNavigation::FindClassHeaderPath(InClass, ClassHeaderPath))
		{
			const FString AbsoluteHeaderPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ClassHeaderPath);
			OutSourceFiles.Add(AbsoluteHeaderPath);
		}

		FString ClassSourcePath;
		if(FSourceCodeNavigation::FindClassSourcePath(InClass, ClassSourcePath))
		{
			const FString AbsoluteSourcePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ClassSourcePath);
			OutSourceFiles.Add(AbsoluteSourcePath);
		}
	}
}

bool FSubsystemBrowserUtils::HasPropertiesToDisplay(UClass* InClass)
{
#if UE_VERSION_OLDER_THAN(5,0,0)
	for (TFieldIterator<FProperty> It(InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::IncludeDeprecated); It; ++It)
#else
	for (TFieldIterator<FProperty> It(InClass, EFieldIterationFlags::IncludeSuper|EFieldIterationFlags::IncludeDeprecated); It; ++It)
#endif
	{
		FProperty* const Property = *It;
		if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible|CPF_Edit|CPF_AdvancedDisplay))
		{
			return true;
		}
	}
	return false;
}

