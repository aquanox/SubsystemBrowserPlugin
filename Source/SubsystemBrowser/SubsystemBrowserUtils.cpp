// Copyright 2022, Aquanox.

#include "SubsystemBrowserUtils.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserFlags.h"
#include "SourceCodeNavigation.h"
#include "SubsystemBrowserSettings.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Model/SubsystemBrowserDescriptor.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "Misc/EngineVersionComparison.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

static FAutoConsoleCommandWithWorldArgsAndOutputDevice CmdPrintClassData(
	TEXT("SB.PrintClass"), TEXT("Print class details"),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&FSubsystemBrowserUtils::PrintClassDetails)
);
static FAutoConsoleCommandWithWorldArgsAndOutputDevice CmdPrintPropertyData(
	TEXT("SB.PrintProperty"), TEXT("Print property details"),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&FSubsystemBrowserUtils::PrintPropertyDetails)
);

TOptional<FString> FSubsystemBrowserUtils::GetSmartMetaValue(UObject* InObject, const FName& InName, bool bHierarchical, bool bWarn)
{
	TOptional<FString> UserSource;

	if (bHierarchical)
		UserSource = GetMetadataHierarchical(InObject->GetClass(), InName);
	else
		UserSource = GetMetadataOptional(InObject->GetClass(), InName);

	if (UserSource.IsSet() && FName::IsValidXName(UserSource.GetValue(), INVALID_OBJECTNAME_CHARACTERS))
	{
		const FName Name ( *UserSource.GetValue() );
		if (const UFunction* Func = InObject->GetClass()->FindFunctionByName(Name))
		{
			if (Func->HasAllFunctionFlags(FUNC_Native) && !Func->HasAnyFunctionFlags(FUNC_Static) && Func->GetReturnProperty())
			{
				if (Func->GetReturnProperty()->IsA(FStrProperty::StaticClass()))
				{
					FSubsystemBrowserGetStringProperty Delegate;
					Delegate.BindUFunction(InObject, Name);
					return Delegate.Execute();
				}
				else if (Func->GetReturnProperty()->IsA(FTextProperty::StaticClass()))
				{
					FSubsystemBrowserGetTextProperty Delegate;
					Delegate.BindUFunction(InObject, Name);
					return Delegate.Execute().ToString();
				}
			}
		}
		else if (const FProperty* Prop = InObject->GetClass()->FindPropertyByName(Name))
		{
			if (Prop->IsA(FStrProperty::StaticClass()))
			{
				return CastFieldChecked<FStrProperty>(Prop)->GetPropertyValue(InObject);
			}
			else if (Prop->IsA(FTextProperty::StaticClass()))
			{
				return CastFieldChecked<FTextProperty>(Prop)->GetPropertyValue(InObject).ToString();
			}
		}

		if (bWarn)
		{
			UE_LOG(LogSubsystemBrowser, Warning, TEXT("%s specifies source %s but it is neither valid function or property"),
				*GetNameSafe(InObject), *UserSource.GetValue());
		}
	}

	return UserSource;
}

FString FSubsystemBrowserUtils::GetSubsystemOwnerName(UObject* Instance)
{
	// First try searching for user function or
	TOptional<FString> UserSource = GetSmartMetaValue(Instance, FSubsystemBrowserUserMeta::MD_SBOwnerName, true);
	if (UserSource.IsSet())
	{
		return UserSource.GetValue();
	}

	if (ULocalPlayerSubsystem* PlayerSubsystem = Cast<ULocalPlayerSubsystem>(Instance))
	{
		if (ULocalPlayer* LocalPlayer = PlayerSubsystem->GetLocalPlayer<ULocalPlayer>())
		{
			return LocalPlayer->GetName();
		}
	}
	else if (UWorldSubsystem* WorldSubsystem = Cast<UWorldSubsystem>(Instance))
	{
		return GetWorldDescription(WorldSubsystem->GetWorld()).ToString();
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

bool FSubsystemBrowserUtils::GetModuleDetailsForClass(UClass* InClass, FString& OutName, bool& OutGameFlag)
{
	OutName = TEXT("Unknown");
	OutGameFlag = false;

	// Find module name from class
	if( InClass )
	{
		if( UPackage* ClassPackage = InClass->GetOuterUPackage() )
		{
			//@Package name transition
			FName ShortClassPackageName = FPackageName::GetShortFName(ClassPackage->GetFName());

			OutName = ShortClassPackageName.ToString();

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
					OutName = ModuleStatus.Name;
					OutGameFlag = ModuleStatus.bIsGameModule;
					return true;
				}
				else
				{
					UE_LOG(LogSubsystemBrowser, Warning, TEXT("Failed to resolve module details of %s: Query Failed"), *ShortClassPackageName.ToString() );
				}
			}
			else
			{
				UE_LOG(LogSubsystemBrowser, Warning, TEXT("Failed to resolve module details of %s: Not Loaded Yet"), *ShortClassPackageName.ToString() );
			}
		}
	}
	return false;
}

bool FSubsystemBrowserUtils::GetPluginDetailsForClass(UClass* InClass, FString& OutName, FString& OutFriendlyName)
{
	if (InClass)
	{
		if (UPackage* ClassPackage = InClass->GetOuterUPackage())
		{
			FName ShortClassPackageName = FPackageName::GetShortFName(ClassPackage->GetFName());

			for (TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
			{
				for (const FModuleDescriptor& ModuleDescriptor : Plugin->GetDescriptor().Modules)
				{
					if (ModuleDescriptor.Name == ShortClassPackageName)
					{
						OutName = Plugin->GetName();
#if UE_VERSION_OLDER_THAN(4, 26, 0)
						OutFriendlyName = Plugin->GetName();
#else
						OutFriendlyName = Plugin->GetFriendlyName();
#endif
						return true;
					}
				}
			}
		}
	}
	return false;
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
			if (Property->HasAnyPropertyFlags(CPF_Edit))
			{
				Stats.NumConfigWithEdit ++;
			}
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

TOptional<FString> FSubsystemBrowserUtils::GetMetadataOptional(UClass* InClass, FName InKey)
{
	if (const FString* Value = InClass->FindMetaData(InKey))
	{
		return *Value;
	}
	return TOptional<FString>();
}

TOptional<FString> FSubsystemBrowserUtils::GetMetadataHierarchical(UClass* InClass, FName InKey)
{
	UClass* CurrentClass = InClass;
	while(CurrentClass && CurrentClass != UObject::StaticClass())
	{
		if (const FString* Value = CurrentClass->FindMetaData(InKey))
		{
			return *Value;
		}

		CurrentClass = CurrentClass->GetSuperClass();
	}

	return TOptional<FString>();
}

void FSubsystemBrowserUtils::SetClipboardText(const FString& ClipboardText)
{
	UE_LOG(LogSubsystemBrowser, Log, TEXT("Clipboard set to:\n%s"), *ClipboardText);

	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
}

FString FSubsystemBrowserUtils::GenerateConfigExport(const FSubsystemTreeSubsystemItem* SelectedSubsystem, bool bModifiedOnly)
{
	if (SelectedSubsystem->IsStale())
		return FString();

	return GenerateConfigExport(SelectedSubsystem->GetObjectForDetails(), bModifiedOnly);
}

FString FSubsystemBrowserUtils::GenerateConfigExport(UObject* Subsystem, bool bModifiedOnly)
{
	UClass* const Class = Subsystem->GetClass();

	FString ConfigBlock;
	ConfigBlock.Reserve(256);
	ConfigBlock += FString::Printf(TEXT("; Should be in %s%s"), *Class->GetConfigName(), LINE_TERMINATOR);
	ConfigBlock += FString::Printf(TEXT("; or defaults %s%s"), *Class->GetDefaultConfigFilename(), LINE_TERMINATOR);
	ConfigBlock += FString::Printf(TEXT("[%s.%s]%s"), *Class->GetOuterUPackage()->GetName(), *Class->GetName(), LINE_TERMINATOR);

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
					const uint8* DataPtr = Property->ContainerPtrToValuePtr<uint8>(Subsystem, Idx);
					const uint8* DefaultValue = Property->ContainerPtrToValuePtrForDefaults<uint8>(Class, SubsystemDefaults, Idx);
					if (bModifiedOnly == false || !Property->Identical(DataPtr, DefaultValue, PPF_DeepCompareInstances))
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
				const uint8* DataPtr = Property->ContainerPtrToValuePtr<uint8>(Subsystem, Idx);
				const uint8* DefaultValue = Property->ContainerPtrToValuePtrForDefaults<uint8>(Class, SubsystemDefaults, Idx);

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

template<typename T>
FString FlagsToString(T InValue, TMap<T, FString> const& InFlagMap)
{
	TArray<FString, TInlineAllocator<8>> Result;
	for(auto& FlagMapEntry : InFlagMap)
	{
		if ( ((uint32)InValue & (uint32)FlagMapEntry.Key) != 0 )
		{
			Result.Add(FlagMapEntry.Value);
		}
	}
	return FString::Join(Result, TEXT("|"));
}

const TMap<EClassFlags, FString>& GetClassFlagsMap()
{
	static TMap<EClassFlags, FString> Data;
	if (!Data.Num())
	{
#define ADD_FLAG(Name) Data.Add(EClassFlags::Name, TEXT(#Name))
		ADD_FLAG(CLASS_Abstract);
		ADD_FLAG(CLASS_DefaultConfig);
		ADD_FLAG(CLASS_Config);
		ADD_FLAG(CLASS_Transient);
		ADD_FLAG(CLASS_MatchedSerializers);
		ADD_FLAG(CLASS_Native);
#if UE_VERSION_OLDER_THAN(5, 1, 0)
		ADD_FLAG(CLASS_NoExport);
#endif
		ADD_FLAG(CLASS_NotPlaceable);
		ADD_FLAG(CLASS_PerObjectConfig);
		ADD_FLAG(CLASS_ReplicationDataIsSetUp);
		ADD_FLAG(CLASS_EditInlineNew);
		ADD_FLAG(CLASS_CollapseCategories);
		ADD_FLAG(CLASS_Interface);
#if UE_VERSION_OLDER_THAN(5, 1, 0)
		ADD_FLAG(CLASS_CustomConstructor);
#endif
		ADD_FLAG(CLASS_Const);
		ADD_FLAG(CLASS_CompiledFromBlueprint);
		ADD_FLAG(CLASS_MinimalAPI);
		ADD_FLAG(CLASS_RequiredAPI);
		ADD_FLAG(CLASS_DefaultToInstanced);
		ADD_FLAG(CLASS_TokenStreamAssembled);
		ADD_FLAG(CLASS_HasInstancedReference);
		ADD_FLAG(CLASS_Hidden);
		ADD_FLAG(CLASS_Deprecated);
		ADD_FLAG(CLASS_HideDropDown);
		ADD_FLAG(CLASS_GlobalUserConfig);
		ADD_FLAG(CLASS_Intrinsic);
		ADD_FLAG(CLASS_Constructed);
		ADD_FLAG(CLASS_ConfigDoNotCheckDefaults);
		ADD_FLAG(CLASS_NewerVersionExists);

#if !UE_VERSION_OLDER_THAN(5, 0, 0)
		ADD_FLAG(CLASS_Optional);
		ADD_FLAG(CLASS_ProjectUserConfig);
		ADD_FLAG(CLASS_NeedsDeferredDependencyLoading);
#endif

#undef ADD_FLAG
	}
	return Data;
}


const TMap<EPropertyFlags, FString>& GetPropertyFlagsMap()
{
	static TMap<EPropertyFlags, FString> Data;
	if (!Data.Num())
	{
#define ADD_FLAG(Name) Data.Add(EPropertyFlags::Name, TEXT(#Name))
		ADD_FLAG(CPF_Edit);
		ADD_FLAG(CPF_ConstParm);
		ADD_FLAG(CPF_BlueprintVisible);
		ADD_FLAG(CPF_ExportObject);
		ADD_FLAG(CPF_BlueprintReadOnly);
		ADD_FLAG(CPF_Net);
		ADD_FLAG(CPF_EditFixedSize);
		ADD_FLAG(CPF_Parm);
		ADD_FLAG(CPF_OutParm);
		ADD_FLAG(CPF_ZeroConstructor);
		ADD_FLAG(CPF_ReturnParm);
		ADD_FLAG(CPF_DisableEditOnTemplate);
		ADD_FLAG(CPF_Transient);
		ADD_FLAG(CPF_Config);
		ADD_FLAG(CPF_DisableEditOnInstance);
		ADD_FLAG(CPF_EditConst);
		ADD_FLAG(CPF_GlobalConfig);
		ADD_FLAG(CPF_InstancedReference);
		ADD_FLAG(CPF_DuplicateTransient);
		ADD_FLAG(CPF_SaveGame);
		ADD_FLAG(CPF_NoClear);
		ADD_FLAG(CPF_ReferenceParm);
		ADD_FLAG(CPF_BlueprintAssignable);
		ADD_FLAG(CPF_Deprecated);
		ADD_FLAG(CPF_IsPlainOldData);
		ADD_FLAG(CPF_RepSkip);
		ADD_FLAG(CPF_RepNotify);
		ADD_FLAG(CPF_Interp);
		ADD_FLAG(CPF_NonTransactional);
		ADD_FLAG(CPF_EditorOnly);
		ADD_FLAG(CPF_NoDestructor);
		ADD_FLAG(CPF_AutoWeak);
		ADD_FLAG(CPF_ContainsInstancedReference);
		ADD_FLAG(CPF_AssetRegistrySearchable);
		ADD_FLAG(CPF_SimpleDisplay);
		ADD_FLAG(CPF_AdvancedDisplay);
		ADD_FLAG(CPF_Protected);
		ADD_FLAG(CPF_BlueprintCallable);
		ADD_FLAG(CPF_BlueprintAuthorityOnly);
		ADD_FLAG(CPF_TextExportTransient);
		ADD_FLAG(CPF_NonPIEDuplicateTransient);
		ADD_FLAG(CPF_ExposeOnSpawn);
		ADD_FLAG(CPF_PersistentInstance);
		ADD_FLAG(CPF_UObjectWrapper);
		ADD_FLAG(CPF_HasGetValueTypeHash);
		ADD_FLAG(CPF_NativeAccessSpecifierPublic);
		ADD_FLAG(CPF_NativeAccessSpecifierProtected);
		ADD_FLAG(CPF_NativeAccessSpecifierPrivate);
		ADD_FLAG(CPF_SkipSerialization);
#undef	ADD_FLAG
	}
	return Data;
}


void FSubsystemBrowserUtils::PrintClassDetails(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& InLog)
{
	if (InArgs.Num() != 1)
	{
		InLog.Log(TEXT("Invalid number of arguments"));
		return;
	}

	UClass* Class = FindObject<UClass>(nullptr, *InArgs[0]);
	if (!Class)
	{
		InLog.Log(TEXT("Class not found"));
		return;
	}

	InLog.Logf(TEXT("Class: %s"), *Class->GetName());
	InLog.Logf(TEXT("Class Flags: %s"), *FlagsToString(Class->GetClassFlags(), GetClassFlagsMap()));

	for (TPropertyValueIterator<FProperty> It(Class, Class->GetDefaultObject()); It; ++It)
	{
		const FProperty* Property = It->Key;
		InLog.Logf(TEXT("Property: %s (%s)"), *Property->GetName(), *Property->GetClass()->GetName());
		InLog.Logf(TEXT("Flags: %s"), *FlagsToString(Property->GetPropertyFlags(), GetPropertyFlagsMap()));
	}
}

void FSubsystemBrowserUtils::PrintPropertyDetails(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& InLog)
{
	if (InArgs.Num() != 2)
	{
		InLog.Log(TEXT("Invalid number of arguments"));
		return;
	}

	UClass* Class = FindObject<UClass>(nullptr, *InArgs[0]);
	if (!Class)
	{
		InLog.Log(TEXT("Class not found"));
		return;
	}

	FProperty* Property = Class->FindPropertyByName(*InArgs[1]);
	if (!Property)
	{
		InLog.Log(TEXT("Property not found"));
		return;
	}

	InLog.Logf(TEXT("Type: %s"), *Property->GetClass()->GetName());
	InLog.Logf(TEXT("Flags: %s"), *FlagsToString(Property->GetPropertyFlags(), GetPropertyFlagsMap()));
}

FText FSubsystemBrowserUtils::GetWorldDescription(const UWorld* World)
{
	FText Description;
	if(World)
	{
		FText PostFix;
		const FWorldContext* WorldContext = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if(Context.World() == World)
			{
				WorldContext = &Context;
				break;
			}
		}

		if (World->WorldType == EWorldType::PIE)
		{
			switch(World->GetNetMode())
			{
				case NM_Client:
					if (WorldContext)
					{
						PostFix = FText::Format(LOCTEXT("ClientPostfixFormat", "(Client {0})"), FText::AsNumber(WorldContext->PIEInstance - 1));
					}
					else
					{
						PostFix = LOCTEXT("ClientPostfix", "(Client)");
					}
					break;
				case NM_DedicatedServer:
				case NM_ListenServer:
					PostFix = LOCTEXT("ServerPostfix", "(Server)");
					break;
				case NM_Standalone:
					PostFix = LOCTEXT("PlayInEditorPostfix", "(Play In Editor)");
					break;
				default:
					break;
			}
		}
		else if(World->WorldType == EWorldType::Editor)
		{
			PostFix = LOCTEXT("EditorPostfix", "(Editor)");
		}

		Description = FText::Format(LOCTEXT("WorldFormat", "{0} {1}"), FText::FromString(World->GetFName().GetPlainNameString()), PostFix);
	}

	return Description;
}

#undef LOCTEXT_NAMESPACE
