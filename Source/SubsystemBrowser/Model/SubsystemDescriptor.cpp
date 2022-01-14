// Copyright 2022, Aquanox.

#include "Model/SubsystemDescriptor.h"

#include "SourceCodeNavigation.h"
#include "Model/SubsystemBrowserModel.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"

static bool FTickableGameObject_IsTickableObject(UObject* Object) { return false; }
static bool FSourceCodeNavigation_IsGameModuleClass(UClass* InClass);
static void FSourceCodeNavigation_CollectSourceFiles(UClass* InClass, TArray<FString>& OutSourceFiles);

FSubsystemTreeCategoryItem::FSubsystemTreeCategoryItem(const FName& CategoryName, const FText& Label, UClass* SubsystemBaseClass, const FEnumSubsystemsDelegate& Selector)
	: CategoryName(CategoryName)
	, SubsystemBaseClass(SubsystemBaseClass)
	, Label(Label)
	, Selector(Selector)
{
}

FSubsystemTreeSubsystemItem::FSubsystemTreeSubsystemItem(UObject* Instance)
{
	check(Instance);
	Subsystem = Instance;

	UClass* const InClass = Instance->GetClass();
	Class = InClass;

	// setup data needed to display so hotreloads won't crash editor or disaster happen
	DisplayName = InClass->GetDisplayNameText();
	ClassName = InClass->GetFName();
	Package = InClass->GetOuterUPackage()->GetName();
	ShortPackage = FPackageName::GetShortName(Package);
	LongPackage = FString::Printf(TEXT("%s.%s"), *Package, *ClassName.ToString());

	bConfigExportable = InClass->HasAnyClassFlags(CLASS_Config);
	ConfigClass = bConfigExportable ? InClass->ClassConfigName.ToString() : FString();

	bTickable = FTickableGameObject_IsTickableObject(Instance);
	bIsGameModuleClass = FSourceCodeNavigation_IsGameModuleClass(InClass);

	FSourceCodeNavigation::FindModulePath(InClass->GetOuterUPackage(), ModulePath);
	FSourceCodeNavigation::FindClassModuleName(InClass, ModuleName);
	FSourceCodeNavigation_CollectSourceFiles(InClass, SourceFilePaths);

	if (auto PlayerSubsystem = Cast<ULocalPlayerSubsystem>(Instance))
	{
		if (auto LocalPlayer = PlayerSubsystem->GetLocalPlayer<ULocalPlayer>())
		{
			LocalPlayerName = LocalPlayer->GetFName();
		}
	}
}

FString FSubsystemTreeSubsystemItem::GetDisplayNameString() const
{
	return (DisplayName.ToString());
}

FString FSubsystemTreeSubsystemItem::GetClassNameString() const
{
	return (ClassName.ToString());
}

FString FSubsystemTreeSubsystemItem::GetShortPackageString() const
{
	return (ShortPackage);
}

FString FSubsystemTreeSubsystemItem::GetPackageString() const
{
	return (Package);
}

FString FSubsystemTreeSubsystemItem::GetOwnerNameString() const
{
	if (LocalPlayerName.IsNone())
		return FString();
	return (LocalPlayerName.ToString());
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
