// Copyright 2022, Aquanox.

#include "Model/SubsystemBrowserDescriptor.h"

#include "SourceCodeNavigation.h"
#include "SubsystemBrowserSettings.h"
#include "SubsystemBrowserModule.h"
#include "SubsystemBrowserUtils.h"
#include "ToolMenu.h"
#include "Interfaces/IPluginManager.h"
#include "Model/SubsystemBrowserModel.h"
#include "UI/SubsystemTableItemTooltip.h"
#include "SubsystemBrowserStyle.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "SubsystemBrowser"

FSubsystemTreeCategoryItem::FSubsystemTreeCategoryItem(TSharedRef<FSubsystemModel> InModel, TSharedRef<FSubsystemCategory> InCategory)
	: Data(InCategory)
{
	Model = InModel;
}

void FSubsystemTreeCategoryItem::GenerateTooltip(FSubsystemTableItemTooltipBuilder& TooltipBuilder) const
{
	TArray<SubsystemTreeItemPtr> Subsystems;
	Model->GetAllSubsystemsInCategory(SharedThis(this), Subsystems);
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_NumSub", "Num Subsystems"), FText::AsNumber(Subsystems.Num()));
}

FSubsystemTreeSubsystemItem::FSubsystemTreeSubsystemItem()
{
}

FSubsystemTreeSubsystemItem::FSubsystemTreeSubsystemItem(TSharedRef<FSubsystemModel> InModel, TSharedPtr<ISubsystemTreeItem> InParent, UObject* Instance)
{
	Model = InModel;
	Parent = InParent;

	check(Instance);
	Subsystem = Instance;

	UClass* const InClass = Instance->GetClass();
	Class = InClass;

	DisplayName = InClass->GetDisplayNameText();
	ClassName = InClass->GetFName();

	Package = InClass->GetOuterUPackage()->GetName();
	if (!FSubsystemBrowserUtils::GetModuleDetailsForClass(InClass, ModuleName, bIsGameModuleClass))
	{
		ModuleName = FPackageName::GetShortName(Package);
		bIsGameModuleClass = false;
	}
	
	ScriptName = FString::Printf(TEXT("/Script/%s.%s"), *ModuleName, *ClassName.ToString());

	if (InClass->HasAnyClassFlags(CLASS_Config) && !InClass->ClassConfigName.IsNone())
	{
		bConfigExportable = true;
		bIsDefaultConfig = InClass->HasAnyClassFlags(CLASS_DefaultConfig);
		bIsGlobalUserConfig = InClass->HasAnyClassFlags(CLASS_GlobalUserConfig);
		ConfigName = InClass->ClassConfigName;
	}

	FSubsystemBrowserUtils::CollectSourceFiles(InClass, SourceFilePaths);

	OwnerName = FSubsystemBrowserUtils::GetSubsystemOwnerName(Instance);

	if (FSubsystemBrowserUtils::GetPluginDetailsForClass(InClass, PluginName, PluginDisplayName))
	{
		bIsPluginClass = true;
	}

	PropertyStats = FSubsystemBrowserUtils::GetClassFieldStats(InClass);

	TOptional<FString> UserColorValue = FSubsystemBrowserUtils::GetMetadataHierarchical(InClass, FSubsystemBrowserUserMeta::MD_SBColor);
	if (UserColorValue.IsSet())
	{
		FLinearColor Value(ForceInit);
		if (Value.InitFromString(*UserColorValue))
		{
			UserColor = Value;
		}
	}

	TOptional<FString> UserTooltipValue = FSubsystemBrowserUtils::GetMetadataHierarchical(InClass, FSubsystemBrowserUserMeta::MD_SBTooltip);
	if (UserTooltipValue.IsSet())
	{
		UserTooltip = UserTooltipValue;
	}
}

bool FSubsystemTreeSubsystemItem::IsSelected() const
{
	return Model.IsValid() && Model->IsItemSelected(SharedThis(this));
}

FText FSubsystemTreeSubsystemItem::GetDisplayName() const
{
	return DisplayName;
}

bool FSubsystemTreeSubsystemItem::HasViewableElements() const
{
	if (PropertyStats.NumProperties && (PropertyStats.NumEditable || PropertyStats.NumVisible))
		return true;
	if (PropertyStats.NumCallable)
		return true;
	if (PropertyStats.NumConfig)
		return true;
	return false;
}

void FSubsystemTreeSubsystemItem::GenerateTooltip(FSubsystemTableItemTooltipBuilder& TooltipBuilder) const
{
	//TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Path", "Path"), FText::FromString(LongPackage));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Class", "Class"), FText::FromName(ClassName));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Module", "Module"), FText::FromString(ModuleName));
	if (IsPluginModule())
	{
		TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Plugin", "Plugin"), FText::FromString(PluginDisplayName));
	}
	if (IsConfigExportable())
	{
		TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Config", "Config"), FText::FromName(ConfigName));
	}
	if (!OwnerName.IsEmpty())
	{
		TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Owner", "Owned by"), FText::FromString(OwnerName));
	}

	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_Props", "Num Properties"), FText::AsNumber(PropertyStats.NumProperties));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_PropsEditable", "Num Editable Properties"), FText::AsNumber(PropertyStats.NumEditable));
	TooltipBuilder.AddPrimary(LOCTEXT("SubsystemTooltipItem_PropsConfig", "Num Config Properties"), FText::AsNumber(PropertyStats.NumConfig));

	if (UserTooltip.IsSet())
	{
		TooltipBuilder.SetUserTooltip(FText::FromString(UserTooltip.GetValue()));
	}
}

void FSubsystemTreeSubsystemItem::GenerateContextMenu(UToolMenu* MenuBuilder) const
{
	TWeakPtr<const FSubsystemTreeSubsystemItem> Self = SharedThis(this);

	{
		FToolMenuSection& Section = MenuBuilder->AddSection("SubsystemContextActions", LOCTEXT("SubsystemContextActions", "Common"));
		Section.AddMenuEntry("OpenSourceFile",
			LOCTEXT("OpenSourceFile", "Open Source File"),
			FText::GetEmpty(),
			FStyleHelper::GetSlateIcon("SystemWideCommands.FindInContentBrowser"),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						UObject* ViewedObject = Self.Pin()->GetObjectForDetails();
						if (!ViewedObject || !FSourceCodeNavigation::CanNavigateToClass(ViewedObject->GetClass()))
						{
							FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("OpenSourceFile_Failed", "Failed to open source file."), SNotificationItem::CS_Fail);
						}
						else
						{
							FSourceCodeNavigation::NavigateToClass(ViewedObject->GetClass());
						}
					}
				})
			)
		);
	}

	{
		FToolMenuSection& Section = MenuBuilder->AddSection("SubsystemReferenceActions", LOCTEXT("SubsystemReferenceActions", "References"));
		Section.AddMenuEntry("CopyClassName",
			LOCTEXT("CopyClassName", "Copy Class Name"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						FSubsystemBrowserUtils::SetClipboardText(FString::Printf(TEXT("U%s"), *Self.Pin()->ClassName.ToString()));
					}
				})
			)
		);
		Section.AddMenuEntry("CopyPackageName",
			LOCTEXT("CopyPackageName", "Copy Module Name"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						FSubsystemBrowserUtils::SetClipboardText(Self.Pin()->ModuleName);
					}
				})
			)
		);
		Section.AddMenuEntry("CopyScriptName",
			LOCTEXT("CopyScriptName", "Copy Script Name"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						FSubsystemBrowserUtils::SetClipboardText(Self.Pin()->ScriptName);
					}
				})
			)
		);
		Section.AddMenuEntry("CopyFilePath",
			LOCTEXT("CopyFilePath", "Copy File Path"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						const TArray<FString>& FilePaths = Self.Pin()->SourceFilePaths;

						FString ClipboardText;
						if (FilePaths.Num() > 0)
						{
							const FString* FoundHeader = FilePaths.FindByPredicate([](const FString& S)
							{
								FString Extension = FPaths::GetExtension(S);
								return Extension == TEXT("h") || Extension == TEXT("hpp");
							});

							if (!FoundHeader) FoundHeader = &FilePaths[0];

							ClipboardText = FPaths::ConvertRelativePathToFull(*FoundHeader);
						}

						FSubsystemBrowserUtils::SetClipboardText(ClipboardText);
					}
				})
			)
		);
	}

	if (IsConfigExportable())
	{
		FToolMenuSection& Section = MenuBuilder->AddSection("SubsystemConfigActions", LOCTEXT("SubsystemConfigActions", "Config"));

		Section.AddMenuEntry("ExportToDefaults",
			LOCTEXT("ExportToDefaults", "Export Default Config"),
			LOCTEXT("ExportToDefaultTooltip", "Export current values to DefaultConfig (only if class has DefaultConfig specifier)"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						if (FSubsystemBrowserUtils::TryUpdateDefaultConfigFile(Self.Pin()->GetObjectForDetails()))
						{
							FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("SubsystemBrowser", "Successfully updated defaults"), SNotificationItem::CS_Success);
						}
						else
						{
							FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("SubsystemBrowser", "Failed to update defaults"), SNotificationItem::CS_Fail);
						}
					}
				}),
				FCanExecuteAction::CreateSP(this, &FSubsystemTreeSubsystemItem::IsDefaultConfig)
			)
		);
		Section.AddMenuEntry("ExportModified",
			LOCTEXT("ExportModified", "Export Modified Properties"),
			LOCTEXT("ExportModifiedTooltip", "Export modified properties as an INI section and store it in clipboard"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						FString ClipboardText = FSubsystemBrowserUtils::GenerateConfigExport(Self.Pin().Get(), true);
						FSubsystemBrowserUtils::SetClipboardText(ClipboardText);
						FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("SubsystemBrowser", "Copied to clipboard"), SNotificationItem::CS_Success);
					}
				}),
				FCanExecuteAction::CreateSP(this, &FSubsystemTreeSubsystemItem::IsConfigExportable)
			)
		);
		Section.AddMenuEntry("ExportAll",
			LOCTEXT("ExportAll", "Export All Properties"),
			LOCTEXT("ExportAllTooltip", "Export all config properties as an INI section and store it in clipboard"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Self]()
				{
					if (Self.IsValid())
					{
						FString ClipboardText = FSubsystemBrowserUtils::GenerateConfigExport(Self.Pin().Get(), false);
						FSubsystemBrowserUtils::SetClipboardText(ClipboardText);
						FSubsystemBrowserUtils::ShowBrowserInfoMessage(LOCTEXT("SubsystemBrowser", "Copied to clipboard"), SNotificationItem::CS_Success);
					}
				}),
				FCanExecuteAction::CreateSP(this, &FSubsystemTreeSubsystemItem::IsConfigExportable)
			)
		);
	}
}

#undef LOCTEXT_NAMESPACE
