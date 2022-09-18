// Copyright 2022, Aquanox.

#include "SubsystemBrowserUtils.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserFlags.h"
#include "SourceCodeNavigation.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interfaces/IPluginManager.h"
#include "Model/SubsystemBrowserDescriptor.h"
#include "HAL/PlatformApplicationMisc.h"

FString FSubsystemBrowserUtils::GetDefaultSubsystemOwnerName(UObject* Instance)
{
	if (ULocalPlayerSubsystem* PlayerSubsystem = Cast<ULocalPlayerSubsystem>(Instance))
	{
		if (ULocalPlayer* LocalPlayer = PlayerSubsystem->GetLocalPlayer<ULocalPlayer>())
		{
			return LocalPlayer->GetName();
		}
	}
	else if (UObject* Outer = Instance->GetOuter())
	{
		return Outer->GetName();
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

FSubsystemBrowserUtils::FClassFieldStats FSubsystemBrowserUtils::GetClassFieldStats(UClass* InClass)
{
	FClassFieldStats Stats;

#if UE_VERSION_OLDER_THAN(5,0,0)
	for (TFieldIterator<FProperty> It(InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::IncludeDeprecated); It; ++It)
#else
	for (TFieldIterator<FProperty> It(InClass, EFieldIterationFlags::IncludeSuper|EFieldIterationFlags::IncludeDeprecated); It; ++It)
#endif
	{
		const FProperty* Property = CastField<FProperty>(*It);
		Stats.NumProperties++;

		if (Property->HasAnyPropertyFlags(CPF_Edit))
		{
			Stats.NumEditable ++;
		}
		if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
		{
			Stats.NumVisible ++;
		}
		if (Property->HasAnyPropertyFlags(CPF_Config|CPF_GlobalConfig))
		{
			Stats.NumConfig ++;
		}
	}

	const FName MD_CallInEditor(TEXT("CallInEditor"));

#if UE_VERSION_OLDER_THAN(5,0,0)
	for (TFieldIterator<UFunction> It(InClass, EFieldIteratorFlags::IncludeSuper, EFieldIteratorFlags::IncludeDeprecated); It; ++It)
#else
	for (TFieldIterator<UFunction> It(InClass, EFieldIterationFlags::IncludeSuper|EFieldIterationFlags::IncludeDeprecated); It; ++It)
#endif
	{
		const UFunction* TestFunction = *It;
		if (TestFunction->GetBoolMetaData(MD_CallInEditor) && (TestFunction->ParmsSize == 0))
		{
			Stats.NumCallable ++;
		}
	}

	return Stats;
}

void FSubsystemBrowserUtils::SetClipboardText(const FString& ClipboardText)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Clipboard set to:\n%s"), *ClipboardText);

	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
}

FString FSubsystemBrowserUtils::GenerateConfigExport(const FSubsystemTreeSubsystemItem* SelectedSubsystem, bool bModifiedOnly)
{
	FString ConfigBlock;
	ConfigBlock.Reserve(256);
	ConfigBlock += FString::Printf(TEXT("; Should be in Default%s.ini"), *SelectedSubsystem->ConfigName.ToString());
	ConfigBlock += LINE_TERMINATOR;
	ConfigBlock += FString::Printf(TEXT("[%s.%s]"), *SelectedSubsystem->Package, *SelectedSubsystem->ClassName.ToString());
	ConfigBlock += LINE_TERMINATOR;

	UObject* const Subsystem  = SelectedSubsystem->Subsystem.Get();
	UClass* const Class = SelectedSubsystem->Class.Get();
	UObject* const SubsystemDefaults  = Class ? Class->GetDefaultObject() : nullptr;

	if (Subsystem && SubsystemDefaults && Class)
	{
		TArray<FProperty*> ModifiedProperties;

		for (TFieldIterator<FProperty> It(Class); It; ++It)
		{
			FProperty* Property = *It;
			if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient | CPF_Deprecated | CPF_SkipSerialization))
				continue;

			if( Property->HasAnyPropertyFlags(CPF_Config) )
			{
				for( int32 Idx=0; Idx<Property->ArrayDim; Idx++ )
				{
					uint8* DataPtr      = Property->ContainerPtrToValuePtr           <uint8>((uint8*)Subsystem, Idx);
					uint8* DefaultValue = Property->ContainerPtrToValuePtrForDefaults<uint8>(Class, (uint8*)SubsystemDefaults, Idx);
					if (bModifiedOnly == false || !Property->Identical( DataPtr, DefaultValue, PPF_DeepCompareInstances))
					{
						ModifiedProperties.Add(Property);
						break;
					}
				}
			}
		}

		for (FProperty* Property : ModifiedProperties)
		{
			const TCHAR* Prefix = TEXT("");

			if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
			{
				ConfigBlock += FString::Printf(TEXT("!%s=ClearArray"), *Property->GetName());
				ConfigBlock += LINE_TERMINATOR;
				Prefix = TEXT("+");

				FScriptArrayHelper ArrayHelper(ArrayProperty, Subsystem);
				if (!ArrayHelper.Num())
				{
					continue;
				}
			}

			for( int32 Idx=0; Idx< Property->ArrayDim; Idx++ )
			{
				uint8* DataPtr      = Property->ContainerPtrToValuePtr           <uint8>(Subsystem, Idx);
				uint8* DefaultValue = Property->ContainerPtrToValuePtrForDefaults<uint8>(Class, SubsystemDefaults, Idx);

				FString ExportValue;
#if UE_VERSION_OLDER_THAN(5,1,0)
				Property->ExportTextItem(ExportValue, DataPtr, DefaultValue, nullptr, 0);
#else
				Property->ExportTextItem_Direct(ExportValue, DataPtr, DefaultValue, nullptr, 0);
#endif

				if (ExportValue.IsEmpty())
				{
					ConfigBlock += FString::Printf(TEXT("%s="), *Property->GetName());
				}
				else
				{
					ConfigBlock += FString::Printf(TEXT("%s%s=%s"), Prefix, *Property->GetName(), *ExportValue);
				}
				ConfigBlock += LINE_TERMINATOR;
			}
		}
	}

	return ConfigBlock;
}

void FSubsystemBrowserUtils::ShowBrowserInfoMessage(FText InText, SNotificationItem::ECompletionState InType)
{
	FNotificationInfo Info(InText);
	Info.ExpireDuration = 5.0f;

	if (TSharedPtr<SNotificationItem> InfoItem = FSlateNotificationManager::Get().AddNotification(Info))
	{
		InfoItem->SetCompletionState(InType);
	}
}

bool FSubsystemBrowserUtils::TryUpdateDefaultConfigFile(UObject* Instance)
{
	if (!Instance)
	{
		return false;
	}

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FString ConfigFile = Instance->GetDefaultConfigFilename();

	if (!FPaths::FileExists(ConfigFile) || !IFileManager::Get().IsReadOnly(*ConfigFile))
	{
		Instance->UpdateDefaultConfigFile();
		return true;
	}

	return false;
#else
	return Instance->TryUpdateDefaultConfigFile();
#endif
}
