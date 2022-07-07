// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserDescriptor.h"

#include "SourceCodeNavigation.h"
#include "Model/SubsystemBrowserModel.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"

static bool FSourceCodeNavigation_IsGameModuleClass(UClass* InClass);
static void FSourceCodeNavigation_CollectSourceFiles(UClass* InClass, TArray<FString>& OutSourceFiles);
static FString GetSubsystemOwnerName(UObject* InObject);

FSubsystemTreeCategoryItem::FSubsystemTreeCategoryItem(TSharedRef<FSubsystemCategory> InCategory)
	: Data(InCategory)
{

}

TArray<UObject*> FSubsystemTreeCategoryItem::Select(UWorld* InContext) const
{
	TArray<UObject*> OutResult;
	Data->Selector.Execute(InContext, OutResult);
	return OutResult;
}

FSubsystemTreeSubsystemItem::FSubsystemTreeSubsystemItem(UObject* Instance)
{
	check(Instance);
	Subsystem = Instance;

	UClass* const InClass = Instance->GetClass();
	Class = InClass;

	// save data needed to display so hotreload or other things won't crash editor or disaster happen
	DisplayName = InClass->GetDisplayNameText();
	ClassName = InClass->GetFName();
	Package = InClass->GetOuterUPackage()->GetName();
	ShortPackage = FPackageName::GetShortName(Package);
	LongPackage = FString::Printf(TEXT("%s.%s"), *Package, *ClassName.ToString());

	if (InClass->HasAnyClassFlags(CLASS_Config))
	{
		bConfigExportable = true;
		ConfigName = InClass->ClassConfigName;
	}

	bIsGameModuleClass = FSourceCodeNavigation_IsGameModuleClass(InClass);

	FSourceCodeNavigation::FindModulePath(InClass->GetOuterUPackage(), ModulePath);
	FSourceCodeNavigation::FindClassModuleName(InClass, ModuleName);
	FSourceCodeNavigation_CollectSourceFiles(InClass, SourceFilePaths);

	OwnerName = GetSubsystemOwnerName(Instance);
}

FText FSubsystemTreeSubsystemItem::GetDisplayName() const
{
	return DisplayName;
}

FString FSubsystemTreeSubsystemItem::GetShortPackageString() const
{
	return ShortPackage;
}

FString FSubsystemTreeSubsystemItem::GetPackageString() const
{
	return Package;
}

FString FSubsystemTreeSubsystemItem::GetConfigNameString() const
{
	return ConfigName.IsNone() ? FString() : ConfigName.ToString();
}

FString FSubsystemTreeSubsystemItem::GetOwnerNameString() const
{
	return OwnerName;
}

static bool FSourceCodeNavigation_IsGameModuleClass(UClass* InClass)
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

static void FSourceCodeNavigation_CollectSourceFiles(UClass* InClass, TArray<FString>& OutSourceFiles)
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

static FString GetSubsystemOwnerName(UObject* Instance)
{
	if (auto PlayerSubsystem = Cast<ULocalPlayerSubsystem>(Instance))
	{
		if (auto LocalPlayer = PlayerSubsystem->GetLocalPlayer<ULocalPlayer>())
		{
			return LocalPlayer->GetName();
		}
	}

	return FString();
}
